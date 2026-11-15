#pragma once
#include "../Reconstruction/Sector/Policy.h"
namespace RetroMediaLab::MediaEngine::Composition {
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::shared_ptr<const Reconstruction::Sector::Policy>> defaultSectorPolicies();
}

