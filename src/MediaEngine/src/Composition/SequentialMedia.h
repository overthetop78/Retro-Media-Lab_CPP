#pragma once
#include "../Decoding/Sequential/Registry.h"
#include "../Encoding/Sequential/Registry.h"

namespace RetroMediaLab::MediaEngine {
// Shared protocol composition. Format readers and writers remain independent.
class SequentialMediaComposition final {
public:
    RETROMEDIALAB_MEDIAENGINE_API static SequentialMediaComposition createDefault();

    const SequentialDecoderRegistry& decoders() const noexcept { return decoders_; }
    const SequentialEncoderRegistry& encoders() const noexcept { return encoders_; }

private:
    SequentialMediaComposition(SequentialDecoderRegistry decoders, SequentialEncoderRegistry encoders)
        : decoders_(std::move(decoders)), encoders_(std::move(encoders)) {}

    SequentialDecoderRegistry decoders_;
    SequentialEncoderRegistry encoders_;
};
}
