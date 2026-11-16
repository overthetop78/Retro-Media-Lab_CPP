#pragma once
#include "../Disk/Explorer.h"
#include "../../Images/MediaExplorer.h"
#include <RetroMediaLab/MediaEngine/MediaExploration.h>
namespace RetroMediaLab::MediaEngine {
class MediaExplorationService final {
public:
 RETROMEDIALAB_MEDIAENGINE_API MediaExplorationService(MediaImageReadingService,DiskImageExplorer,MediaExplorer);
 RETROMEDIALAB_MEDIAENGINE_API ExploredMediaImage explore(const std::filesystem::path&,std::optional<std::string> = {},const MediaExplorationProgressCallback& = {},std::stop_token={})const;
 RETROMEDIALAB_MEDIAENGINE_API MediaOpeningAnalysisResult analyze(const std::filesystem::path&,std::optional<std::string> = {},const DiskExplorationProgressCallback& = {},const MediaExplorationProgressCallback& = {},std::stop_token={},bool includeFileSystems=true,const DiskCaptureLoadedCallback& = {})const;
private:
 std::shared_ptr<const MediaDocument> read(const std::filesystem::path&,std::optional<std::string>,std::stop_token)const;
 MediaImageReadingService reading_;DiskImageExplorer disks_;MediaExplorer media_;
};
}
