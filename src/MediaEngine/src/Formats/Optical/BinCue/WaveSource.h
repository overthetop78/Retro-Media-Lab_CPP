#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/Sources.h>
namespace RetroMediaLab::MediaEngine::BinCue {
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::RandomAccessData> wavePcmSource(std::shared_ptr<const MediaFileSystems::RandomAccessData>,bool requireCompleteSectors=true,std::stop_token={});
}
