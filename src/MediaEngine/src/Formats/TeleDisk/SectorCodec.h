#pragma once
#include "Constants.h"
#include "Records.h"
#include <RetroMediaLab/MediaEngine/Export.h>
namespace RetroMediaLab::MediaEngine::TeleDisk {
RETROMEDIALAB_MEDIAENGINE_API std::uint16_t crc(MediaFileSystems::Bytes,std::uint16_t initial=Constants::CrcInitial,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> decodeSector(MediaFileSystems::Bytes,SectorEncoding,std::size_t expectedLength,std::uint8_t cylinder,std::uint8_t head,std::uint8_t sector);
RETROMEDIALAB_MEDIAENGINE_API EncodedSector encodeSector(MediaFileSystems::Bytes);
}
