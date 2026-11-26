#pragma once
#include "InfrastructureApi.h"
#include "GwProgressTracker.h"
#include <RetroMediaLab/MediaEngine/DiskExplorationTypes.h>
#include <RetroMediaLab/MediaEngine/MediaExplorationProgress.h>
#include <mutex>
namespace RetroMediaLab::Infrastructure {
struct LoadingProgress {
    MediaEngine::MessageInfo stage;
    std::optional<MediaEngine::MessageInfo> detailMessage;
    std::string detail;
    double value=0;
    bool indeterminate=true;
    std::uint64_t revision=0;
    GwTrackProgress tracks;
    std::shared_ptr<const MediaEngine::DiskCaptureSnapshot> capture;
    std::optional<MediaEngine::MessageInfo> formatCheck;
};
class OpeningProgress final {
public:
    RETROMEDIALAB_INFRASTRUCTURE_API void reset(std::string fileName);
    RETROMEDIALAB_INFRASTRUCTURE_API void report(const MediaEngine::DiskExplorationProgress&);
    RETROMEDIALAB_INFRASTRUCTURE_API void report(const MediaEngine::MediaExplorationProgress&);
    RETROMEDIALAB_INFRASTRUCTURE_API LoadingProgress snapshot()const;
    RETROMEDIALAB_INFRASTRUCTURE_API void loadCapture(std::shared_ptr<const MediaEngine::DiskCaptureSnapshot>);
private:
    mutable std::mutex mutex_;
    LoadingProgress value_;
};
}
