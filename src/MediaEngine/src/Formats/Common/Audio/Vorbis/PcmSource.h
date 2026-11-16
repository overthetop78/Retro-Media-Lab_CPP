#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/Sources.h>
namespace RetroMediaLab::MediaEngine::Vorbis {
struct PcmSource {
    std::shared_ptr<const MediaFileSystems::RandomAccessData> data;
    int channels=0;
    long sampleRate=0;
};
RETROMEDIALAB_MEDIAENGINE_API PcmSource openPcmSource(std::shared_ptr<const MediaFileSystems::RandomAccessData>,std::stop_token={});
}
