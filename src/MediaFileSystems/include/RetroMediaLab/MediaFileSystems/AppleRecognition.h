#pragma once
#include "Sources.h"
#include "Export.h"
#include <optional>
#include <string>
namespace RetroMediaLab::MediaFileSystems::Apple {
enum class MacintoshKind {Mfs,Hfs};
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<std::uint16_t> macintoshSignature(Bytes) noexcept;
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool supportedMacintoshSignature(std::uint16_t) noexcept;
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<MacintoshKind> identifyMacintosh(Bytes) noexcept;
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool validDosVtoc(Bytes,std::int32_t tracks,std::int32_t sectorsPerTrack) noexcept;
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool validProDosHeader(Bytes) noexcept;
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool knownLisaVersion(std::uint16_t) noexcept;
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool printableLisaName(Bytes) noexcept;
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool validLisaHeader(Bytes) noexcept;
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string decodeLisaName(Bytes);
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool containsSosMarker(Bytes) noexcept;
}
