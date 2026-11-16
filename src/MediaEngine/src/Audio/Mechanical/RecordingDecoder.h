#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <filesystem>
#include <stop_token>
#include <vector>
namespace RetroMediaLab::MediaEngine::MechanicalAudio {
struct DecodedRecording {std::vector<float> samples;int sampleRate=0;};
// FLAC bank input only. A zero output rate preserves the native sample rate.
RETROMEDIALAB_MEDIAENGINE_API DecodedRecording decodeRecording(const std::filesystem::path&,int outputRate=0,const std::filesystem::path& executable={},std::stop_token={});
}
