#pragma once
#include "CpcDsk.h"
#include "Constants.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::CpcDsk::Internal {
inline bool starts(MediaFileSystems::Bytes bytes,std::string_view text){return bytes.size()>=text.size()&&std::equal(text.begin(),text.end(),bytes.begin(),[](char a,std::uint8_t b){return std::uint8_t(a)==b;});}
inline std::uint16_t word(MediaFileSystems::Bytes data,std::size_t at){return std::uint16_t(data[at]|std::uint16_t(data[at+1])<<std::numeric_limits<std::uint8_t>::digits);}
inline void word(MediaFileSystems::MutableBytes data,std::size_t at,std::uint16_t value){data[at]=std::uint8_t(value);data[at+1]=std::uint8_t(value>>std::numeric_limits<std::uint8_t>::digits);}
inline std::uint32_t nominalSize(std::uint8_t code){return Constants::MinimumSectorSize<<(code&Constants::SectorSizeCodeMask);}
}
