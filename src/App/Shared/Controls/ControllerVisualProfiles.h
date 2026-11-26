#pragma once

#include "Shared/Controls/ControllerArtwork.h"
#include <optional>

namespace RetroMediaLab::ControllerVisualProfiles {
struct Profile {
    ControllerArtwork::HostModel model;
    QString displayName;
};

RETROMEDIALAB_APP_API std::optional<Profile> profile(const QString& deviceId);
RETROMEDIALAB_APP_API bool save(const QString& deviceId, const std::optional<Profile>& profile);
}
