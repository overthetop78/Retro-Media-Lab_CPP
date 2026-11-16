#include "Internal.h"
#include "Constants.h"
#include "../Exploration/Composition/Internal.h"
#include <RetroMediaLab/MediaFileSystems/AmigaDos.h>
#include <RetroMediaLab/MediaFileSystems/AppleDos.h>
#include <RetroMediaLab/MediaFileSystems/AppleSos.h>
#include <RetroMediaLab/MediaFileSystems/CommodoreDos.h>
#include <RetroMediaLab/MediaFileSystems/Fat12.h>
#include <RetroMediaLab/MediaFileSystems/FileSystemIds.h>
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <algorithm>
#include <limits>
#include <unordered_set>

namespace RetroMediaLab::MediaFileSystems {
MigrationPlan FileSystemMigrationService::createPlan(const FileSystemVolume& source, std::string target) { return MigrationPlanner::create(source, std::move(target)); }
MigrationValidationReport FileSystemMigrationService::validate(const MigrationPlan& plan, const SectorImage& image, bool accept) {
    return MigrationValidator::validate(plan, MigrationInternal::capabilities(plan, image), accept);
}
MigrationInjectionResult FileSystemMigrationService::inject(const MigrationPlan& plan, const SectorImage& image, bool accept, std::stop_token token) {
    checkCancellation(token);
    const auto capabilities = MigrationInternal::capabilities(plan, image);
    auto report = MigrationValidator::validate(plan, capabilities, accept);
    MigrationValidator::ensureExecutable(report);
    const auto count = image.blockCount();
    if (count > std::uint64_t(std::numeric_limits<std::int32_t>::max())) throw MediaError(DiagnosticCode::MigrationCapacityLimit);
    std::uint64_t available = 0;
    image.visitAvailableBlocks([&](std::uint64_t) { ++available; return available <= count; }, token);
    if (available != count) throw MediaError(DiagnosticCode::MigrationBlank);
    std::vector<SectorAddress> addresses;
    addresses.reserve(std::size_t(count));
    const auto& geometry = image.geometry();
    for (std::uint64_t index = 0; index < count; ++index) {
        checkCancellation(token);
        const auto block = image.block(index, token);
        if (!block || block->data.size() != geometry.blockSize || std::any_of(block->data.begin(), block->data.end(), [](auto byte) { return byte != 0; })) throw MediaError(DiagnosticCode::MigrationBlank);
        addresses.push_back(block->address);
    }
    const auto writable = MigrationMetadataReducer::reduce(plan, capabilities);
    const auto& id = plan.targetFileSystemId;
    SectorWritePlan filled;
    if (id == FileSystemIds::AmigaDosFfs || id == FileSystemIds::AmigaDosOfs)
        filled = AmigaDos::write(writable, id == FileSystemIds::AmigaDosFfs ? AmigaDos::Variant::Ffs : AmigaDos::Variant::Ofs, geometry, {}, token);
    else if (id == FileSystemIds::Fat12)
        filled = Fat12::write(writable, {geometry, Composition::ordinalKey(geometry.formatId).starts_with(Composition::ordinalKey(MigrationConstants::MsxPrefix))}, token);
    else if (id == FileSystemIds::AppleDos) filled = AppleDos::write(writable, geometry.formatId, token);
    else if (id == FileSystemIds::ProDos) filled = ProDos::write(writable, geometry, std::uint32_t(count), addresses, {}, token);
    else if (id == FileSystemIds::Sos) {
        auto proDosGeometry = geometry;
        proDosGeometry.formatId = Formats::AppleIIProDos140;
        filled = AppleSos::write(writable, proDosGeometry, std::uint32_t(count), addresses, geometry.formatId, {}, token);
    } else if (id == FileSystemIds::CommodoreDos) filled = CommodoreDos::write(writable, geometry.formatId, {}, token);
    else throw MediaError(DiagnosticCode::MigrationUnsupported, {id});
    const auto& output = filled.geometry;
    if (Composition::ordinalKey(output.formatId) != Composition::ordinalKey(geometry.formatId) || output.blockSize != geometry.blockSize
        || output.cylinders != geometry.cylinders || output.heads != geometry.heads || output.sectorsPerTrack != geometry.sectorsPerTrack || filled.blocks.size() != count)
        throw MediaError(DiagnosticCode::MigrationGeometry);
    std::unordered_set<std::uint64_t> seen;
    for (const auto& block : filled.blocks) {
        checkCancellation(token);
        if (block.logicalBlock >= count || !seen.insert(block.logicalBlock).second || block.address != addresses[std::size_t(block.logicalBlock)]) throw MediaError(DiagnosticCode::MigrationAddress);
    }
    filled.geometry = geometry;
    filled.capacity = image.capacity();
    filled.logicalBlockCount = count;
    return {std::move(filled), std::move(report)};
}
}
