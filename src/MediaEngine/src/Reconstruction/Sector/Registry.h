#pragma once
#include "Policy.h"
namespace RetroMediaLab::MediaEngine::Reconstruction::Sector {
class Registry final {
public:
    using Entry = std::shared_ptr<const Policy>;
    RETROMEDIALAB_MEDIAENGINE_API Registry();
    RETROMEDIALAB_MEDIAENGINE_API explicit Registry(std::vector<Entry>);
    RETROMEDIALAB_MEDIAENGINE_API Entry resolve(const Image&) const;
private:
    std::vector<Entry> ordered_;
};
}

