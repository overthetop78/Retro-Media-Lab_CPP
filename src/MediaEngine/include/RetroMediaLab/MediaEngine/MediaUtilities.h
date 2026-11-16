#pragma once
#include "Export.h"
#include <cstdint>
#include <map>
#include <span>
#include <string>
namespace RetroMediaLab::MediaEngine {
RETROMEDIALAB_MEDIAENGINE_API std::uint32_t computeMediaCrc32(std::span<const std::uint8_t>)noexcept;
RETROMEDIALAB_MEDIAENGINE_API std::int32_t readMediaMetadataIntOrDefault(const std::map<std::string,std::string>&,const std::string& key);
}
