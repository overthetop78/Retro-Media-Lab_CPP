#pragma once
#include "Constants.h"
namespace RetroMediaLab::MediaEngine::HardDiskRaw {
inline constexpr bool compatible(std::int64_t length,std::int32_t blockSize){return length>0&&(blockSize==Constants::LegacyBlockSize||blockSize==Constants::LargeBlockSize)&&length%blockSize==0;}
}
