#pragma once
#include <RetroMediaLab/MediaFileSystems/FileSystemModels.h>
#include <RetroMediaLab/MediaFileSystems/Sources.h>
namespace RetroMediaLab::MediaFileSystems::Macintosh {
enum class ForkKind {Data,Resource};
std::string fileType(Bytes);
bool lessName(const std::string&,const std::string&);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string decodeName(Bytes);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string pascalString(Bytes,std::size_t offset,std::size_t maximumLength);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<Timestamp> fromSeconds(std::int64_t);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::int32_t storageReference(std::uint32_t) noexcept;
}
