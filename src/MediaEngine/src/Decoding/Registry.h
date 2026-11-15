#pragma once
#include "Decoder.h"
#include <span>
namespace RetroMediaLab::MediaEngine::Decoding {
struct Selection {std::size_t revolutionIndex=0;SharedResult result;};
class Registry final {
public:
    RETROMEDIALAB_MEDIAENGINE_API Registry();
    RETROMEDIALAB_MEDIAENGINE_API explicit Registry(std::vector<std::shared_ptr<const Decoder>>);
    RETROMEDIALAB_MEDIAENGINE_API const std::vector<std::shared_ptr<const Decoder>>& decoders() const noexcept;
    RETROMEDIALAB_MEDIAENGINE_API SharedResult decode(std::string_view,const Flux::SharedRevolution&) const;
    RETROMEDIALAB_MEDIAENGINE_API SharedResult decodeAutomatic(const Flux::SharedRevolution&) const;
    RETROMEDIALAB_MEDIAENGINE_API std::optional<Selection> decodeBest(std::span<const Flux::SharedRevolution>,std::optional<std::string_view> decoderId = {}) const;
    // The operation keeps the registry service alive; each invocation returns an owned value.
    RETROMEDIALAB_MEDIAENGINE_API DecodeOperation operation() const;
private:
    class State;
    std::shared_ptr<State> state_;
};
}
