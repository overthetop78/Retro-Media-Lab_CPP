#pragma once
#include "MediaDocument.h"
#include <RetroMediaLab/MediaEngine/ExplorationInfo.h>
#include <RetroMediaLab/MediaFileSystems/MediaExplorer.h>
namespace RetroMediaLab::MediaEngine {
class MediaExplorer final {
public:
    explicit MediaExplorer(MediaFileSystems::MediaExplorer explorer):explorer_(std::move(explorer)){}
    RETROMEDIALAB_MEDIAENGINE_API static MediaExplorer createDefault();
    RETROMEDIALAB_MEDIAENGINE_API std::future<ExploredMediaImage> exploreAsync(std::shared_ptr<const MediaDocument>,std::stop_token={})const;
private:MediaFileSystems::MediaExplorer explorer_;
};
}
