#pragma once
#include "FileSystemModels.h"
#include "SectorImage.h"
namespace RetroMediaLab::MediaFileSystems::BbcDfs {
struct CatalogueFields {std::uint32_t entryCount,totalSectors;bool countValid;};
RETROMEDIALAB_MEDIAFILESYSTEMS_API CatalogueFields decodeCatalogue(Bytes);
struct EntryFields {std::uint32_t length,startSector,load,execute;bool operator==(const EntryFields&) const = default;};
RETROMEDIALAB_MEDIAFILESYSTEMS_API EntryFields decodeEntry(Bytes,std::size_t offset);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string decodeName(Bytes);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string describeAddresses(std::uint32_t load,std::uint32_t execute);
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool structuredCatalogue(Bytes names,Bytes metadata,std::uint32_t totalSectors);
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool canRead(const SectorImage&,std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemVolume read(const SectorImage&,std::stop_token = {});
}
