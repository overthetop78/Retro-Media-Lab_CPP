#pragma once
#include <RetroMediaLab/MediaFileSystems/Sources.h>
#include <string>
#include <string_view>
#include <vector>
namespace RetroMediaLab::MediaFileSystems::Petscii {
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string decode(Bytes);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::vector<std::uint8_t> encode(std::u16string_view,std::int32_t length);
}
