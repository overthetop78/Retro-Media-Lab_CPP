#include "Registry.h"
#include "../../Composition/SectorPolicies.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Reconstruction::Sector {
Registry::Registry() : Registry(Composition::defaultSectorPolicies()) {}
Registry::Registry(std::vector<Entry> policies) : ordered_(std::move(policies)) {
    for (const auto& policy : ordered_) if (!policy) throw MediaError(DiagnosticCode::PolicyNull);
}
Registry::Entry Registry::resolve(const Image& image) const {
    for (const auto& policy : ordered_) if (policy->canHandle(image)) return policy;
    return {};
}
}

