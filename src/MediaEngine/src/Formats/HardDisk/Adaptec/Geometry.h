#pragma once
#include "Codec.h"
namespace RetroMediaLab::MediaEngine::Adaptec {
struct Geometry {std::int32_t sectorSize=0,cylinders=0,heads=0,sectorsPerTrack=0;};
RETROMEDIALAB_MEDIAENGINE_API std::optional<Geometry> readGeometry(std::span<const Field>);
}
