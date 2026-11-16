#pragma once
#include "DiskExploration.h"
namespace RetroMediaLab::MediaEngine {
enum class MediaExplorationStage {ReadingMedia,ReadingFileSystem,RecognizingFiles};
struct MediaExplorationProgress {MediaExplorationStage stage;std::string detail;double value;std::optional<MediaKind> mediaKind;};
using MediaExplorationProgressCallback=std::function<void(const MediaExplorationProgress&)>;
struct MediaOpeningAnalysisResult {std::shared_ptr<const MediaDocument> document;std::shared_ptr<const ExploredDisk> diskExploration;std::optional<ExploredMediaImage> mediaExploration;};
RETROMEDIALAB_MEDIAENGINE_API std::future<ExploredMediaImage> exploreMediaAsync(std::filesystem::path,std::optional<std::string> format={},MediaExplorationProgressCallback={},std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::future<MediaOpeningAnalysisResult> analyzeMediaOpeningAsync(std::filesystem::path,std::optional<std::string> format={},DiskExplorationProgressCallback={},MediaExplorationProgressCallback={},std::stop_token={},bool includeFileSystems=true,DiskCaptureLoadedCallback={});
}
