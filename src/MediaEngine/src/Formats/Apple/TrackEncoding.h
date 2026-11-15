#pragma once
#include "../../Encoding/Registry.h"
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
namespace RetroMediaLab::MediaEngine::Apple {
class TrackEncodingService final {
public:
    RETROMEDIALAB_MEDIAENGINE_API TrackEncodingService();
    RETROMEDIALAB_MEDIAENGINE_API explicit TrackEncodingService(Encoding::Registry);
    RETROMEDIALAB_MEDIAENGINE_API static bool supportsStandard(std::string_view) noexcept;
    RETROMEDIALAB_MEDIAENGINE_API std::vector<Flux::Bits> encodeStandard(const MediaFileSystems::SectorImage&,std::int32_t maximumBits,std::stop_token={}) const;
    RETROMEDIALAB_MEDIAENGINE_API std::vector<Flux::Bits> encodeRwts18(const MediaFileSystems::SectorImage&,std::int32_t maximumBits,std::stop_token={}) const;
private:
    Encoding::Registry encoders_;
};
}
