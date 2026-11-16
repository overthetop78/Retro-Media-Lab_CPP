#pragma once
#include "WriterRegistry.h"
#include <RetroMediaLab/MediaEngine/DriveService.h>
#include <RetroMediaLab/MediaEngine/MediaExplorationProgress.h>
namespace RetroMediaLab::MediaEngine {
class MediaImageWritingService final {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit MediaImageWritingService(WriterRegistry,MediaDriveService={});
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::vector<std::filesystem::path>> writeAsync(std::shared_ptr<const MediaDocument>,std::filesystem::path output,std::string targetFormat,std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::vector<std::filesystem::path>> writeAsync(std::shared_ptr<const MediaDocument>,std::filesystem::path output,std::string targetFormat,MediaExplorationProgressCallback,std::stop_token)const;
    const MediaDriveService& driveService()const noexcept{return drives_;}
private:
    WriterRegistry writers_;MediaDriveService drives_;
};
}
