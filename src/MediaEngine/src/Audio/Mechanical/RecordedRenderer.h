#pragma once
#include "RecordedSounds.h"
#include "Constants.h"
#include <functional>
#include <memory>
namespace RetroMediaLab::MediaEngine::MechanicalAudio {
// Loaders return mono float PCM at the requested rate; neither callback owns an audio device.
using SampleLoader=std::function<std::vector<float>(const RecordedSample&,int)>;
using DurationLoader=std::function<double(const RecordedSample&)>;
class RecordedRenderer final {
public:
    RETROMEDIALAB_MEDIAENGINE_API RecordedRenderer(std::vector<RecordedProfile>,SampleLoader,DurationLoader);
    RETROMEDIALAB_MEDIAENGINE_API ~RecordedRenderer();
    RecordedRenderer(const RecordedRenderer&)=delete;
    RecordedRenderer& operator=(const RecordedRenderer&)=delete;
    RETROMEDIALAB_MEDIAENGINE_API double duration(std::string_view profile,std::string_view operation,int distance=1,int direction=1,int cylinder=-1);
    RETROMEDIALAB_MEDIAENGINE_API std::vector<std::int16_t> render(std::string_view device,std::string_view operation,int rate,int count,std::int64_t offset,std::string_view profile,
        double volume=Constants::DefaultVolumePercent,int distance=1,int direction=1,int cylinder=-1);
    // Call before starting emulation to keep first-use decoding out of audio rendering.
    RETROMEDIALAB_MEDIAENGINE_API void prepareProfile(std::string_view profile,int rate);
private:
    struct State;
    std::unique_ptr<State> state_;
};
}
