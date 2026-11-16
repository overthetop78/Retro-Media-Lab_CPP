#pragma once
#include "Export.h"
#include "MediaKind.h"
#include "MechanicalSoundConstants.h"
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>
namespace RetroMediaLab::MediaEngine {
// Owns no host output, clock or rendering thread. Default instances share the drive sample cache.
class MechanicalSoundService final {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit MechanicalSoundService(std::filesystem::path assetsRoot={},std::filesystem::path ffmpegExecutable={});
    RETROMEDIALAB_MEDIAENGINE_API ~MechanicalSoundService();
    RETROMEDIALAB_MEDIAENGINE_API static std::vector<std::string> profiles(MediaKind);
    RETROMEDIALAB_MEDIAENGINE_API static std::string profileDisplayName(std::string_view);
    RETROMEDIALAB_MEDIAENGINE_API std::vector<std::int16_t> render(MediaKind,std::string_view operation,std::int32_t sampleRate,std::int32_t sampleCount,std::int64_t offset,std::optional<std::string> profile={},double volumePercent=DefaultMechanicalVolumePercent,std::int32_t distance=1,std::int32_t direction=1,std::int32_t cylinder=-1)const;
    RETROMEDIALAB_MEDIAENGINE_API double duration(MediaKind,std::string_view operation,std::optional<std::string> profile={},std::int32_t distance=1,std::int32_t direction=1,std::int32_t cylinder=-1)const;
    RETROMEDIALAB_MEDIAENGINE_API void prepareProfile(MediaKind,std::int32_t sampleRate,std::optional<std::string> profile={})const;
private:
    struct State;std::shared_ptr<State> state_;
};
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::int16_t> protectMechanicalAudioMix(std::span<const double>);
}
