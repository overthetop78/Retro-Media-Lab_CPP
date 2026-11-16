#pragma once
#include "ExplorationInfo.h"
#include "Export.h"
#include <filesystem>
#include <future>
#include <span>
#include <stop_token>
namespace RetroMediaLab::MediaEngine {
struct FileSystemMigrationTarget {std::string_view formatId, fileSystemId, extension;};
class FileSystemMigrationTargetCatalog final {
public:
    RETROMEDIALAB_MEDIAENGINE_API static std::span<const FileSystemMigrationTarget> all() noexcept;
    RETROMEDIALAB_MEDIAENGINE_API static const FileSystemMigrationTarget& get(std::string_view formatId);
};
enum class MigrationLossKind {MissingContent,UnsupportedEntryKind,InvalidName,NameTooLong,NameCollision,FileTooLarge,InvalidMetadata,ModifiedDate,Comment,Attributes};
struct MigrationLoss {MigrationLossKind kind; std::string path; bool isBlocking; std::string detail;};
struct MigrationValidationReport {
    std::vector<MigrationLoss> losses;
    bool metadataLossAccepted = false;
    RETROMEDIALAB_MEDIAENGINE_API bool canExecute() const noexcept;
};
struct MigrationResult {std::shared_ptr<const MediaDocument> image; MigrationValidationReport report;};
class FileSystemMigrationService final {
public:
    RETROMEDIALAB_MEDIAENGINE_API static MigrationValidationReport validate(const FileSystemVolumeInfo&, std::string_view targetFormatId, bool acceptMetadataLoss = false);
    RETROMEDIALAB_MEDIAENGINE_API static MigrationResult createImage(const FileSystemVolumeInfo&, std::string_view targetFormatId, bool acceptMetadataLoss = false, std::stop_token = {});
    RETROMEDIALAB_MEDIAENGINE_API static std::future<MigrationResult> writeAsync(FileSystemVolumeInfo, std::filesystem::path output, std::string targetFormatId, bool acceptMetadataLoss = false, std::stop_token = {});
};
}
