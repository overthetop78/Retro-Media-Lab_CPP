#pragma once

#include "AppApi.h"
#include <RetroMediaLab/Platform/Gamepads.h>

namespace RetroMediaLab::ControllerAnalogProfiles {
inline constexpr int MaximumInnerPercent = 50;
inline constexpr int MaximumOuterPercent = 30;

struct Profile {
    int stickPercent = 0;
    int triggerPercent = 0;
    int outerPercent = 0;
    bool operator==(const Profile&) const = default;
};

RETROMEDIALAB_APP_API Profile profile(const QString& deviceId);
RETROMEDIALAB_APP_API void preview(const QString& deviceId, Profile value);
RETROMEDIALAB_APP_API bool save(const QString& deviceId, Profile value);
RETROMEDIALAB_APP_API QList<Platform::GamepadState> read(Platform::GamepadSource& source);
RETROMEDIALAB_APP_API void apply(Platform::GamepadState& state, Profile value);
}
