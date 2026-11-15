#include "Policies.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>

namespace RetroMediaLab::MediaEngine::Reconstruction::Sector {
namespace {
class MatchingPolicy final : public Policy {
public:
    MatchingPolicy(bool prefix, std::string encoder, std::vector<std::string> formats)
        : prefix_(prefix), encoder_(std::move(encoder)), formats_(std::move(formats)) {
        if (Primitives::isBlankIdentifier(encoder_)) throw MediaError(DiagnosticCode::PolicyEncoderId);
        if (formats_.empty()) throw MediaError(prefix_ ? DiagnosticCode::PolicyPrefixes : DiagnosticCode::PolicyFormats);
        for (const auto& format : formats_)
            if (Primitives::isBlankIdentifier(format)) throw MediaError(prefix_ ? DiagnosticCode::PolicyPrefixes : DiagnosticCode::PolicyFormats);
    }
    bool canHandle(const Image& image) const override {
        for (const auto& format : formats_)
            if (prefix_ ? Primitives::ordinalIdentifierStartsWithIgnoreCase(image.geometry().formatId, format)
                        : Primitives::ordinalIdentifierEqualsIgnoreCase(image.geometry().formatId, format)) return true;
        return false;
    }
    std::string_view encoderId(const Image&) const override { return encoder_; }
private:
    bool prefix_;
    std::string encoder_;
    std::vector<std::string> formats_;
};
}
std::shared_ptr<const Policy> prefixPolicy(std::string encoder, std::vector<std::string> prefixes) {
    return std::make_shared<MatchingPolicy>(true, std::move(encoder), std::move(prefixes));
}
std::shared_ptr<const Policy> exactPolicy(std::string encoder, std::vector<std::string> formats) {
    return std::make_shared<MatchingPolicy>(false, std::move(encoder), std::move(formats));
}
}

