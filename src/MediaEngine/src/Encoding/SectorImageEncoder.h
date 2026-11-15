#pragma once
#include "Registry.h"
#include "../Reconstruction/Sector/Registry.h"

namespace RetroMediaLab::MediaEngine::Encoding {
class SectorImageEncoder final {
public:
    RETROMEDIALAB_MEDIAENGINE_API SectorImageEncoder();
    RETROMEDIALAB_MEDIAENGINE_API SectorImageEncoder(Registry, Reconstruction::Sector::Registry);
    RETROMEDIALAB_MEDIAENGINE_API bool canEncode(const MediaFileSystems::SectorImage&) const;
    RETROMEDIALAB_MEDIAENGINE_API EncodedTrack encodeTrack(const MediaFileSystems::SectorImage&, std::int32_t cylinder,
        std::int32_t head, std::string_view encoderId, const Attributes&, std::uint32_t cellTicks,
        std::uint32_t indexTicks, std::stop_token = {}) const;
    // This single-track API uses nanoseconds for both cells and index time.
    RETROMEDIALAB_MEDIAENGINE_API EncodedTrack encodePhysicalTrack(const MediaFileSystems::SectorImage&, std::int32_t cylinder,
        std::int32_t head, std::uint32_t indexNanoseconds, std::stop_token = {}) const;
    // Whole-image results retain the common 25 ns tick unit in both modes.
    RETROMEDIALAB_MEDIAENGINE_API std::vector<DiskTrack> encode(const MediaFileSystems::SectorImage&, std::stop_token = {}) const;
    RETROMEDIALAB_MEDIAENGINE_API std::vector<DiskTrack> encodePhysical(const MediaFileSystems::SectorImage&, std::stop_token = {}) const;
private:
    std::vector<DiskTrack> encodeImage(const MediaFileSystems::SectorImage&, bool physical, std::stop_token) const;
    Registry encoders_;
    Reconstruction::Sector::Registry policies_;
};
}

