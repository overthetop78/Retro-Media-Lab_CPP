#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/Sources.h>
namespace RetroMediaLab::MediaEngine::AudioPcm {
struct DecodedPcm {
    std::shared_ptr<const MediaFileSystems::RandomAccessData> data;
    int channels=0,sampleRate=0;
};
RETROMEDIALAB_MEDIAENGINE_API DecodedPcm decode(std::shared_ptr<const MediaFileSystems::RandomAccessData>,std::stop_token={});
}
