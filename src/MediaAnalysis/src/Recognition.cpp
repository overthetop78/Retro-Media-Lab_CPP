#include <RetroMediaLab/MediaAnalysis/Recognition.h>
#include "Constants/RecognitionConstants.h"
#include <QString>
#include <algorithm>
#include <array>
#include "Catalogs/Common.h"
#include "Catalogs/Amiga.h"
#include "Catalogs/IbmPc.h"
#include "Catalogs/AtariCommon.h"
#include "Catalogs/AtariTos.h"
#include "Catalogs/Atari8Bit.h"
#include "Catalogs/AppleDos.h"
#include "Catalogs/ProDos.h"
#include "Catalogs/Macintosh.h"
#include "Catalogs/Lisa.h"
#include "Catalogs/Commodore.h"
#include "Catalogs/Cpm.h"
#include "Catalogs/BbcMicro.h"
#include "Catalogs/Dec.h"
#include "Catalogs/Msx.h"
#include "Catalogs/Ucsd.h"

namespace RetroMediaLab::MediaAnalysis {
namespace {
constexpr auto makeCatalog() {
    std::array<MediaContentRecognitionRule, RecognitionConstants::RuleCount> rows{};
    std::size_t index = 0;
    for (const auto& rule : Catalogs::Common::Rows) rows[index++] = rule;
    for (const auto& rule : Catalogs::Amiga::Rows) rows[index++] = rule;
    for (const auto& rule : Catalogs::IbmPc::Rows) rows[index++] = rule;
    for (const auto& rule : Catalogs::AtariCommon::Rows) rows[index++] = rule;
    for (const auto& rule : Catalogs::AtariTos::Rows) rows[index++] = rule;
    for (const auto& rule : Catalogs::Atari8Bit::Rows) rows[index++] = rule;
    for (const auto& rule : Catalogs::AppleDos::Rows) rows[index++] = rule;
    for (const auto& rule : Catalogs::ProDos::Rows) rows[index++] = rule;
    for (const auto& rule : Catalogs::Macintosh::Rows) rows[index++] = rule;
    for (const auto& rule : Catalogs::Lisa::Rows) rows[index++] = rule;
    for (const auto& rule : Catalogs::Commodore::Rows) rows[index++] = rule;
    for (const auto& rule : Catalogs::Cpm::Rows) rows[index++] = rule;
    for (const auto& rule : Catalogs::BbcMicro::Rows) rows[index++] = rule;
    for (const auto& rule : Catalogs::Dec::Rows) rows[index++] = rule;
    for (const auto& rule : Catalogs::Msx::Rows) rows[index++] = rule;
    for (const auto& rule : Catalogs::Ucsd::Rows) rows[index++] = rule;
    return rows;
}
constexpr auto Catalog = makeCatalog();
bool at(Bytes content, Bytes group, std::size_t start) {
    return start <= content.size() && group.size() <= content.size() - start
        && std::equal(group.begin(), group.end(), content.begin() + start);
}
bool signatureMatches(Bytes content, const MediaContentSignature& signature) {
    if (signature.byteGroups.empty() || std::any_of(signature.byteGroups.begin(), signature.byteGroups.end(), [](Bytes group) { return group.empty(); })) return false;
    const auto direction = signature.direction;
    if (direction == MediaContentSearchDirection::Start || direction == MediaContentSearchDirection::End) {
        if (!signature.position || signature.byteGroups.size() != 1) return false;
        auto start = *signature.position;
        if (direction == MediaContentSearchDirection::End) {
            if (start >= content.size()) return false;
            start = content.size() - 1 - start;
        }
        return at(content, signature.byteGroups.front(), start);
    }
    if (signature.position || (direction != MediaContentSearchDirection::SearchStart && direction != MediaContentSearchDirection::SearchEnd)) return false;
    const bool reverse = direction == MediaContentSearchDirection::SearchEnd;
    std::size_t boundary = reverse ? content.size() : 0;
    for (const auto group : signature.byteGroups) {
        bool found = false;
        if (reverse) {
            if (group.size() > boundary) return false;
            auto start = boundary - group.size();
            for (;;) {
                if (at(content, group, start)) { boundary = start; found = true; break; }
                if (start == 0) break;
                --start;
            }
        } else {
            if (group.size() > content.size()) return false;
            const auto end = content.size() - group.size();
            for (auto start = boundary; start <= end; ++start) {
                if (at(content, group, start)) { boundary = start + group.size(); found = true; break; }
            }
        }
        if (!found) return false;
    }
    return true;
}
}
std::string normalizeExtension(std::string_view extension) {
    auto normalized = QString::fromUtf8(extension.data(), qsizetype(extension.size())).trimmed();
    if (normalized.isEmpty()) return {};
    if (!normalized.startsWith(QLatin1Char(RecognitionConstants::ExtensionPrefix))) normalized.prepend(QLatin1Char(RecognitionConstants::ExtensionPrefix));
    auto scalars = normalized.toStdU32String();
    for (auto& scalar : scalars)
        if (scalar != RecognitionConstants::DottedCapitalI) scalar = QChar::toLower(scalar);
    return QString::fromUcs4(scalars.data(), qsizetype(scalars.size())).toUtf8().toStdString();
}
bool hasContentConditions(const MediaContentRecognitionRule& rule) { return !rule.signatures.empty() || !rule.contentLengths.empty(); }
bool matches(const MediaContentRecognitionRule& rule, Content content) {
    if (!content || !hasContentConditions(rule)) return false;
    if (!rule.contentLengths.empty() && std::find(rule.contentLengths.begin(), rule.contentLengths.end(), content->size()) == rule.contentLengths.end()) return false;
    return std::all_of(rule.signatures.begin(), rule.signatures.end(), [&](const auto& signature) { return signatureMatches(*content, signature); });
}
std::span<const MediaContentRecognitionRule> recognitionRules() { return Catalog; }
const MediaContentRecognitionRule* findRule(MediaFileSystemFamily family, std::string_view extension, Content content, MediaContentRecognitionPriority priority) {
    const auto normalized = normalizeExtension(extension);
    const auto find = [&](std::optional<MediaFileSystemFamily> candidate) -> const MediaContentRecognitionRule* {
        for (const auto& row : Catalog) {
            if (row.family == candidate && row.priority == priority && (row.extension.empty() || row.extension == normalized)
                && (hasContentConditions(row) ? matches(row, content) : !row.extension.empty())) return &row;
        }
        return nullptr;
    };
    if (const auto* rule = find(family)) return rule;
    if (family == MediaFileSystemFamily::Atari8Bit || family == MediaFileSystemFamily::AtariTos)
        if (const auto* rule = find(MediaFileSystemFamily::Atari)) return rule;
    return find(std::nullopt);
}
}
