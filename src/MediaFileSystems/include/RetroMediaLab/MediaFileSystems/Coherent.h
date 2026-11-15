#pragma once
#include "FileSystemModels.h"
#include "SectorImage.h"
namespace RetroMediaLab::MediaFileSystems::Coherent {
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::uint32_t canonicalUInt32(Bytes);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::uint32_t blockPointer(Bytes);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string decodeName(Bytes);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string inodeDescription(std::uint16_t);
RETROMEDIALAB_MEDIAFILESYSTEMS_API EntryKind entryKind(std::uint16_t) noexcept;
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<Timestamp> decodeTime(std::uint32_t);
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool looksLike(Bytes);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::uint32_t declaredBlocks(Bytes);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::uint32_t validatedBlocks(Bytes header,std::uint64_t imageLength);
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool canRead(const SectorImage&);
RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemVolume read(const SectorImage&,std::stop_token = {});
}
