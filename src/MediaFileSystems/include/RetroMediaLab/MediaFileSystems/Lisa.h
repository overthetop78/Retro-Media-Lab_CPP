#pragma once
#include "SectorImage.h"
#include "FileSystemModels.h"
#include "AppleRecognition.h"
#include <map>
namespace RetroMediaLab::MediaFileSystems::Lisa {
enum class CatalogVersion : std::uint16_t {Table=0x000e,Hash=0x000f,BTree=0x0011};
struct CatalogResult {std::map<std::uint16_t,std::string> names;std::vector<Diagnostic> warnings;};
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string catalogVersionName(std::uint16_t);
RETROMEDIALAB_MEDIAFILESYSTEMS_API CatalogResult readCatalog(const SectorImage&,std::uint16_t,std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool canRead(const SectorImage&,std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemVolume read(const SectorImage&,std::stop_token = {});
}
