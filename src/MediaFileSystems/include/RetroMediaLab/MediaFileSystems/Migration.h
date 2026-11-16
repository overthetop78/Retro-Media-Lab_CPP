#pragma once
#include "FileSystemModels.h"
#include "SectorImage.h"
#include <functional>
#include <limits>
namespace RetroMediaLab::MediaFileSystems {
struct MigrationEntry {
    std::string sourcePath, targetName;
    EntryKind kind = EntryKind::Unknown;
    std::optional<std::vector<std::uint8_t>> content;
    std::optional<Timestamp> modified;
    std::string comment;
    std::uint32_t rawAttributes = 0;
    bool metadataValid = true;
    std::vector<MigrationEntry> children;
};
struct MigrationPlan {
    std::string sourceFileSystemId, targetFileSystemId, volumeName;
    std::vector<MigrationEntry> entries;
};
enum class MigrationLossKind {MissingContent,UnsupportedEntryKind,InvalidName,NameTooLong,NameCollision,FileTooLarge,InvalidMetadata,ModifiedDate,Comment,Attributes};
struct MigrationLoss {
    MigrationLossKind kind;
    std::string path;
    bool isBlocking = false;
    std::string detail;
};
using MigrationNamePolicy = std::function<bool(std::string_view)>;
struct MigrationTargetCapabilities {
    std::string fileSystemId;
    std::int64_t maximumNameLength = 0, maximumFileSize = 0;
    bool supportsDirectories = false, supportsLinks = false, supportsModifiedDate = false;
    bool supportsComments = false, supportsRawAttributes = false, isCaseSensitive = false;
    std::string forbiddenNameCharacters;
    bool allowsControlCharacters = false;
    MigrationNamePolicy namePolicy;
    std::int64_t maximumVolumeNameLength = std::numeric_limits<std::int32_t>::max();
    MigrationNamePolicy volumeNamePolicy;
};
struct MigrationValidationReport {
    std::vector<MigrationLoss> losses;
    bool metadataLossAccepted = false;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API bool canExecute() const noexcept;
};
class MigrationPlanner final {
public:
    RETROMEDIALAB_MEDIAFILESYSTEMS_API static MigrationPlan create(const FileSystemVolume&, std::string targetFileSystemId);
};
class MigrationValidator final {
public:
    RETROMEDIALAB_MEDIAFILESYSTEMS_API static MigrationValidationReport validate(const MigrationPlan&, const MigrationTargetCapabilities&, bool acceptMetadataLoss = false);
    RETROMEDIALAB_MEDIAFILESYSTEMS_API static void ensureExecutable(const MigrationValidationReport&);
};
class MigrationMetadataReducer final {
public:
    // Apply only after validation and explicit acceptance of metadata losses.
    RETROMEDIALAB_MEDIAFILESYSTEMS_API static MigrationPlan reduce(const MigrationPlan&, const MigrationTargetCapabilities&);
};
struct MigrationInjectionResult {
    SectorWritePlan image;
    MigrationValidationReport report;
};
class FileSystemMigrationService final {
public:
    RETROMEDIALAB_MEDIAFILESYSTEMS_API static MigrationPlan createPlan(const FileSystemVolume&, std::string targetFileSystemId);
    RETROMEDIALAB_MEDIAFILESYSTEMS_API static MigrationValidationReport validate(const MigrationPlan&, const SectorImage&, bool acceptMetadataLoss = false);
    RETROMEDIALAB_MEDIAFILESYSTEMS_API static MigrationInjectionResult inject(const MigrationPlan&, const SectorImage&, bool acceptMetadataLoss = false, std::stop_token = {});
};
}
