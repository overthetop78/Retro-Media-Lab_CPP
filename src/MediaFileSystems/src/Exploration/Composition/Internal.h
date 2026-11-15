#pragma once
#include <RetroMediaLab/MediaFileSystems/SectorRegistry.h>
#include <string>
namespace RetroMediaLab::MediaFileSystems::Composition {
std::u16string ordinalKey(std::string_view);
bool blank(std::string_view);
std::u16string identity(std::string_view,const FileSystemVolume&);
}
