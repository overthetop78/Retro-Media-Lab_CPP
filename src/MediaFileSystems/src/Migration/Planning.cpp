#include <RetroMediaLab/MediaFileSystems/Migration.h>
#include "Constants.h"
#include "../Exploration/Composition/Internal.h"
#include <QString>
#include <QChar>
#include <algorithm>
#include <cstdlib>
#include <map>

namespace RetroMediaLab::MediaFileSystems {
namespace {
using namespace MigrationConstants;
QString text(std::string_view value) { return QString::fromUtf8(value.data(), qsizetype(value.size())); }
MigrationEntry prepare(const FileSystemEntry& source, const std::string& parent) {
    const auto path = parent.empty() ? source.name : parent + PathSeparator + source.name;
    MigrationEntry result{path, source.name, source.kind, source.content, source.modified, source.comment, source.rawAttributes, source.metadataValid, {}};
    for (const auto& child : source.children) result.children.push_back(prepare(child, path));
    return result;
}
std::string timestamp(const Timestamp& value) {
    const auto pad = [](int field, int width) { return QString::number(field).rightJustified(width, QLatin1Char(Padding)); };
    const auto offset = std::abs(value.offsetMinutes);
    return text(TimestampPattern).arg(pad(value.year, YearWidth), pad(value.month, FieldWidth), pad(value.day, FieldWidth),
        pad(value.hour, FieldWidth), pad(value.minute, FieldWidth), pad(value.second, FieldWidth),
        pad(value.nanosecond / NanosecondsPerTick, FractionWidth), QString(QChar(value.offsetMinutes < 0 ? NegativeOffset : PositiveOffset)),
        pad(offset / MinutesPerHour, FieldWidth), pad(offset % MinutesPerHour, FieldWidth)).toUtf8().toStdString();
}
void validateEntries(const std::vector<MigrationEntry>& entries, const MigrationTargetCapabilities& target, std::vector<MigrationLoss>& losses) {
    // Preserve first-occurrence group order, as Enumerable.GroupBy does in C#.
    std::map<std::u16string, std::size_t> groups;
    std::vector<std::vector<const MigrationEntry*>> ordered;
    for (const auto& entry : entries) {
        const auto key = target.isCaseSensitive ? text(entry.targetName).toStdU16String() : Composition::ordinalKey(entry.targetName);
        const auto [position, inserted] = groups.emplace(key, ordered.size());
        if (inserted) ordered.emplace_back();
        ordered[position->second].push_back(&entry);
    }
    for (const auto& group : ordered) if (group.size() > 1)
        for (const auto* entry : group) losses.push_back({MigrationLossKind::NameCollision, entry->sourcePath, true, entry->targetName});
    const auto forbidden = text(target.forbiddenNameCharacters);
    for (const auto& entry : entries) {
        const auto name = text(entry.targetName);
        const auto add = [&](MigrationLossKind kind, bool blocking, std::string detail = {}) { losses.push_back({kind, entry.sourcePath, blocking, std::move(detail)}); };
        if (name.isEmpty() || std::any_of(name.begin(), name.end(), [&](QChar c) { return forbidden.contains(c) || (!target.allowsControlCharacters && c.category() == QChar::Other_Control); })
            || (target.namePolicy && !target.namePolicy(entry.targetName))) add(MigrationLossKind::InvalidName, true, entry.targetName);
        if (name.size() > target.maximumNameLength) add(MigrationLossKind::NameTooLong, true, std::to_string(name.size()));
        if (!entry.metadataValid) add(MigrationLossKind::InvalidMetadata, true);
        if (entry.kind == EntryKind::Directory && !target.supportsDirectories) add(MigrationLossKind::UnsupportedEntryKind, true, std::string(DirectoryKind));
        if (entry.kind == EntryKind::Link && !target.supportsLinks) add(MigrationLossKind::UnsupportedEntryKind, true, std::string(LinkKind));
        if (entry.kind == EntryKind::Unknown) add(MigrationLossKind::UnsupportedEntryKind, true, std::string(UnknownKind));
        if (entry.kind == EntryKind::File && !entry.content) add(MigrationLossKind::MissingContent, true);
        if (entry.kind == EntryKind::File && entry.content && entry.content->size() > std::uint64_t(target.maximumFileSize)) add(MigrationLossKind::FileTooLarge, true, std::to_string(entry.content->size()));
        if (entry.modified && !target.supportsModifiedDate) add(MigrationLossKind::ModifiedDate, false, timestamp(*entry.modified));
        if (!entry.comment.empty() && !target.supportsComments) add(MigrationLossKind::Comment, false, entry.comment);
        if (entry.rawAttributes != 0 && !target.supportsRawAttributes) add(MigrationLossKind::Attributes, false, std::to_string(entry.rawAttributes));
        validateEntries(entry.children, target, losses);
    }
}
void reduceEntries(std::vector<MigrationEntry>& entries, const MigrationTargetCapabilities& target) {
    for (auto& entry : entries) {
        if (!target.supportsModifiedDate) entry.modified.reset();
        if (!target.supportsComments) entry.comment.clear();
        if (!target.supportsRawAttributes) entry.rawAttributes = 0;
        reduceEntries(entry.children, target);
    }
}
}
bool MigrationValidationReport::canExecute() const noexcept {
    return (metadataLossAccepted || losses.empty()) && std::none_of(losses.begin(), losses.end(), [](const auto& loss) { return loss.isBlocking; });
}
MigrationPlan MigrationPlanner::create(const FileSystemVolume& source, std::string targetFileSystemId) {
    if (Composition::blank(targetFileSystemId)) throw MediaError(DiagnosticCode::MigrationTargetId);
    MigrationPlan result{source.fileSystemId, std::move(targetFileSystemId), source.name, {}};
    for (const auto& entry : source.entries) result.entries.push_back(prepare(entry, {}));
    return result;
}
MigrationValidationReport MigrationValidator::validate(const MigrationPlan& plan, const MigrationTargetCapabilities& target, bool acceptMetadataLoss) {
    if (Composition::ordinalKey(plan.targetFileSystemId) != Composition::ordinalKey(target.fileSystemId)) throw MediaError(DiagnosticCode::MigrationTargetMismatch);
    if (target.maximumNameLength <= 0 || target.maximumFileSize < 0) throw MediaError(DiagnosticCode::MigrationLimits);
    MigrationValidationReport result{{}, acceptMetadataLoss};
    const auto length = text(plan.volumeName).size();
    if (length > target.maximumVolumeNameLength) result.losses.push_back({MigrationLossKind::NameTooLong, std::string(RootPath), true, std::to_string(length)});
    if (target.volumeNamePolicy && !target.volumeNamePolicy(plan.volumeName)) result.losses.push_back({MigrationLossKind::InvalidName, std::string(RootPath), true, plan.volumeName});
    validateEntries(plan.entries, target, result.losses);
    return result;
}
void MigrationValidator::ensureExecutable(const MigrationValidationReport& report) {
    if (!report.canExecute()) throw MediaError(DiagnosticCode::MigrationBlocked);
}
MigrationPlan MigrationMetadataReducer::reduce(const MigrationPlan& plan, const MigrationTargetCapabilities& target) {
    auto result = plan;
    reduceEntries(result.entries, target);
    return result;
}
}
