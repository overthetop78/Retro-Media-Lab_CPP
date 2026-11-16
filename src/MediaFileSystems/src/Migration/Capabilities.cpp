#include "Internal.h"
#include "Constants.h"
#include <RetroMediaLab/MediaFileSystems/AmigaDos.h>
#include <RetroMediaLab/MediaFileSystems/AppleDos.h>
#include <RetroMediaLab/MediaFileSystems/ProDos.h>
#include <RetroMediaLab/MediaFileSystems/Fat12.h>
#include <RetroMediaLab/MediaFileSystems/CommodoreDos.h>
#include <RetroMediaLab/MediaFileSystems/FileSystemIds.h>
#include "../FileSystems/Amiga/Dos/Constants.h"
#include "../FileSystems/Apple/Dos/Constants.h"
#include "../FileSystems/Apple/ProDos/Constants.h"
#include "../FileSystems/Commodore/Dos/Constants.h"
#include "../FileSystems/Fat/Constants.h"
#include "../Exploration/Composition/Internal.h"
#include <algorithm>
#include <limits>

namespace RetroMediaLab::MediaFileSystems::MigrationInternal {
MigrationTargetCapabilities capabilities(const MigrationPlan& plan, const SectorImage& image) {
    using namespace MigrationConstants;
    constexpr auto maximum = std::numeric_limits<std::int64_t>::max();
    if (image.capacity() > std::uint64_t(maximum)) throw MediaError(DiagnosticCode::MigrationCapacityLimit);
    MigrationTargetCapabilities result;
    result.fileSystemId = plan.targetFileSystemId;
    result.maximumFileSize = std::int64_t(image.capacity());
    const auto& id = plan.targetFileSystemId;
    if (id == FileSystemIds::AmigaDosFfs || id == FileSystemIds::AmigaDosOfs) {
        result.maximumNameLength = result.maximumVolumeNameLength = AmigaDos::Constants::OrdinaryNameMaximumLength;
        result.supportsDirectories = result.supportsModifiedDate = result.supportsComments = true;
        result.forbiddenNameCharacters = AmigaForbidden;
        result.namePolicy = result.volumeNamePolicy = AmigaDos::validName;
    } else if (id == FileSystemIds::Fat12) {
        result.maximumNameLength = FatConstants::FatDirectoryLayout::NameLength + FatNameSeparatorLength + FatConstants::FatDirectoryLayout::ExtensionLength;
        result.maximumVolumeNameLength = FatBootSectorLayout::VolumeLabelLength;
        result.supportsDirectories = result.supportsModifiedDate = true;
        result.forbiddenNameCharacters = FatForbidden;
        result.namePolicy = RetroMediaLab::MediaFileSystems::Fat12::validShortName;
        result.volumeNamePolicy = RetroMediaLab::MediaFileSystems::Fat12::validVolumeName;
    } else if (id == FileSystemIds::AppleDos) {
        result.maximumNameLength = RetroMediaLab::MediaFileSystems::AppleDos::Constants::NameLength;
        result.maximumFileSize = std::min(result.maximumFileSize, std::int64_t(RetroMediaLab::MediaFileSystems::AppleDos::Constants::MaximumBinarySize));
        result.maximumVolumeNameLength = RetroMediaLab::MediaFileSystems::AppleDos::Constants::VolumeTextLength;
        result.forbiddenNameCharacters = AppleDosForbidden;
        result.namePolicy = RetroMediaLab::MediaFileSystems::AppleDos::validName;
        result.volumeNamePolicy = [](std::string_view name) { return RetroMediaLab::MediaFileSystems::AppleDos::volumeNumber(name).has_value(); };
    } else if (id == FileSystemIds::ProDos || id == FileSystemIds::Sos) {
        result.maximumNameLength = result.maximumVolumeNameLength = RetroMediaLab::MediaFileSystems::ProDos::Constants::MaximumNameLength;
        result.maximumFileSize = RetroMediaLab::MediaFileSystems::ProDos::Constants::MaximumFileLength;
        result.supportsDirectories = result.supportsModifiedDate = true;
        result.namePolicy = result.volumeNamePolicy = RetroMediaLab::MediaFileSystems::ProDos::validName;
    } else if (id == FileSystemIds::CommodoreDos) {
        if (image.blockCount() > std::uint64_t(maximum / RetroMediaLab::MediaFileSystems::CommodoreDos::Constants::DataBytes)) throw MediaError(DiagnosticCode::MigrationCapacityLimit);
        result.maximumNameLength = result.maximumVolumeNameLength = RetroMediaLab::MediaFileSystems::CommodoreDos::Constants::NameLength;
        result.maximumFileSize = std::int64_t(image.blockCount()) * RetroMediaLab::MediaFileSystems::CommodoreDos::Constants::DataBytes;
        result.namePolicy = result.volumeNamePolicy = RetroMediaLab::MediaFileSystems::CommodoreDos::validName;
    } else throw MediaError(DiagnosticCode::MigrationUnsupported, {id});
    result.supportsRawAttributes = (id == FileSystemIds::AppleDos || id == FileSystemIds::CommodoreDos)
        && Composition::ordinalKey(plan.sourceFileSystemId) == Composition::ordinalKey(id);
    return result;
}
}
