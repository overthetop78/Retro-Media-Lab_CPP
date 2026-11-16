#include <RetroMediaLab/MediaEngine/MediaUtilities.h>
#include "../Formats/Woz/Woz.h"
#include <RetroMediaLab/MediaFileSystems/MetadataNumbers.h>
namespace RetroMediaLab::MediaEngine {
std::uint32_t computeMediaCrc32(std::span<const std::uint8_t> bytes)noexcept{return Woz::crc32(bytes);}
std::int32_t readMediaMetadataIntOrDefault(const std::map<std::string,std::string>& metadata,const std::string& key){return MediaFileSystems::MetadataNumbers::readIntOrDefault(metadata,key);}
}
