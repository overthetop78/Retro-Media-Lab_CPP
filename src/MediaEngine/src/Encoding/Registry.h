#pragma once
#include "Track.h"
#include <map>
#include <memory>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Encoding {
class Registry final {
public:
    using Encoder = std::shared_ptr<const TrackEncoder>;
    RETROMEDIALAB_MEDIAENGINE_API Registry();
    RETROMEDIALAB_MEDIAENGINE_API explicit Registry(std::vector<Encoder>);
    const std::vector<Encoder>& encoders() const noexcept { return ordered_; }
    RETROMEDIALAB_MEDIAENGINE_API Encoder get(std::string_view id) const;
    RETROMEDIALAB_MEDIAENGINE_API EncodedTrack encode(std::string_view id, const Request&,
        std::stop_token = {}) const;
private:
    std::vector<Encoder> ordered_;
    std::map<std::string, Encoder, std::less<>> byId_;
};
}

