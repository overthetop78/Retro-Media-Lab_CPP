#pragma once
#include "../../../Models/Sequential/Image.h"
namespace RetroMediaLab::MediaEngine::Tzx {
std::string blockIdText(std::uint8_t);
bool tryDataRange(const SequentialSegmentFields&,std::span<const std::uint8_t>,int& offset,int& length);
bool tryBlockId(const SequentialSegmentFields&,std::uint8_t&);
}
