#pragma once
#include "ReaderRegistry.h"
namespace RetroMediaLab::MediaEngine {
// DriveFactory supplies concrete mechanics; the service owns no audio device or persistent thread.
class MediaImageReadingService final {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit MediaImageReadingService(ReaderRegistry, DriveFactory = {});
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::shared_ptr<const MediaDocument>> readAsync(std::shared_ptr<const MediaSource>, std::stop_token = {}) const;
private:
    ReaderRegistry registry_, automaticRegistry_;
    DriveFactory drives_;
};
}
