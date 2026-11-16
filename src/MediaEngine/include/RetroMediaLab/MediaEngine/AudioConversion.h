#pragma once
#include "Export.h"
#include <filesystem>
#include <optional>
#include <stop_token>
namespace RetroMediaLab::MediaEngine {
enum class AudioOutputFormat {Wave, Mp3, OggVorbis, Flac, Aac, M4a, Wma};
struct AudioConversionRequest {
    std::filesystem::path input,output;
    AudioOutputFormat format=AudioOutputFormat::Wave;
    std::optional<int> sampleRate,channels,bitRate;
    bool overwrite=false;
};
// Blocking service for a worker thread. No GUI or host-installed FFmpeg required.
class AudioConversionService final {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit AudioConversionService(std::filesystem::path executable={});
    RETROMEDIALAB_MEDIAENGINE_API std::filesystem::path executable()const;
    RETROMEDIALAB_MEDIAENGINE_API std::filesystem::path convert(const AudioConversionRequest&,std::stop_token={})const;
private:
    std::filesystem::path executable_;
};
}
