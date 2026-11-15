#pragma once
#include "MediaFileSystemReader.h"
#include "VolumeDetectors.h"
namespace RetroMediaLab::MediaFileSystems {
struct ExploredFileSystemVolume {
    VolumeDescriptor descriptor;
    std::optional<std::string> readerId;
    std::shared_ptr<const FileSystemVolume> fileSystem;
    std::vector<Diagnostic> diagnostics;
    std::vector<AnalyzedFileSystemEntry> analyzedEntries;
};
struct MediaFileSystemExplorationResult {
    std::vector<VolumeDescriptor> volumes;
    std::vector<ExploredFileSystemVolume> exploredVolumes;
    std::vector<Diagnostic> diagnostics;
    std::optional<std::string> firstFileName;
};
class MediaExplorer {
public:
    RETROMEDIALAB_MEDIAFILESYSTEMS_API MediaExplorer(std::vector<std::shared_ptr<const MediaFileSystemReader>>, VolumeDetectorRegistry);
    RETROMEDIALAB_MEDIAFILESYSTEMS_API static MediaExplorer createDefault(std::vector<std::shared_ptr<const MediaFileSystemReader>> adapterReaders = {});
    RETROMEDIALAB_MEDIAFILESYSTEMS_API std::span<const std::shared_ptr<const MediaFileSystemReader>> readers() const noexcept;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API std::future<MediaFileSystemExplorationResult> exploreAsync(std::shared_ptr<const ImageDocument>, std::stop_token = {}) const;
private:
    struct Impl;
    std::shared_ptr<const Impl> impl_;
};
}
