#pragma once
#include <RetroMediaLab/MediaFileSystems/SectorReader.h>
namespace RetroMediaLab::MediaFileSystems::Composition {
std::shared_ptr<const SectorReader> createAdfsDocumentReader();
}
