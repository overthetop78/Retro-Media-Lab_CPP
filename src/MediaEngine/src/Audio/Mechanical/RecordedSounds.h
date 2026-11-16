#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>
namespace RetroMediaLab::MediaEngine::MechanicalAudio {
struct RecordedOperation {
    std::string operation;
    std::int32_t distance = 0, direction = 0, cylinder = -1;
    bool loop = false;
    std::optional<std::string> startOperation;
};
struct RecordedSample {
    std::string filePath;
    std::vector<RecordedOperation> operations;
    RETROMEDIALAB_MEDIAENGINE_API std::string_view id() const noexcept;
};
struct RecordedProfile {
    std::string id, device, displayName;
    std::vector<RecordedSample> samples;
};
RETROMEDIALAB_MEDIAENGINE_API std::span<const RecordedProfile> recordedProfiles();
RETROMEDIALAB_MEDIAENGINE_API const RecordedProfile* findRecordedProfile(std::string_view id);
RETROMEDIALAB_MEDIAENGINE_API std::optional<std::string_view> baseRecordedProfile(std::string_view device) noexcept;
RETROMEDIALAB_MEDIAENGINE_API std::vector<const RecordedProfile*> recordedProfilesForDevice(std::string_view device);
// Select a sample using the same stable priorities as the C# catalogue.
RETROMEDIALAB_MEDIAENGINE_API const RecordedSample* selectRecordedSample(const RecordedProfile&, std::string_view operation, std::int32_t distance = 1, std::int32_t direction = 1, std::int32_t cylinder = -1);
}
