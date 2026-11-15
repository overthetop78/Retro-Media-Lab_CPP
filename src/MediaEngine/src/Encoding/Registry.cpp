#include "Registry.h"
#include "../Composition/FluxCodecs.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include "../Primitives/Identifiers.h"
namespace RetroMediaLab::MediaEngine::Encoding {
Registry::Registry() : Registry(Composition::defaultEncoders()) {}
Registry::Registry(std::vector<Encoder> encoders) : ordered_(std::move(encoders)) {
    for (std::size_t index = 0; index < ordered_.size(); ++index) {
        const auto& encoder = ordered_[index];
        if (!encoder)
            throw MediaError(DiagnosticCode::EncoderRegistryNull, {std::to_string(index)});
        const auto id = encoder->id();
        if (Primitives::isBlankIdentifier(id))
            throw MediaError(DiagnosticCode::EncoderRegistryInvalidId, {std::to_string(index)});
        if (!byId_.emplace(std::string(id), encoder).second)
            throw MediaError(DiagnosticCode::EncoderRegistryDuplicate, {std::string(id)});
    }
}
Registry::Encoder Registry::get(std::string_view id) const {
    const auto found = byId_.find(id);
    if (found == byId_.end())
        throw MediaError(DiagnosticCode::EncoderRegistryUnknown, {std::string(id)});
    return found->second;
}
EncodedTrack Registry::encode(std::string_view id, const Request& request, std::stop_token token) const {
    return get(id)->encode(request, token);
}
}

