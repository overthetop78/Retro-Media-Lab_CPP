#pragma once
#include "../../../Models/Sequential/Image.h"
namespace RetroMediaLab::MediaEngine::Tzx {
std::string blockIdText(std::uint8_t);
bool tryBlockId(const SequentialSegmentFields&,std::uint8_t&);
}
