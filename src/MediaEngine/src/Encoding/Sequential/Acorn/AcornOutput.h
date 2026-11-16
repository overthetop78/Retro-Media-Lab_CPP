#pragma once
#include "../../../Models/Sequential/Image.h"
namespace RetroMediaLab::MediaEngine::AcornTape {
std::shared_ptr<const SequentialImage> encodeUef(const std::vector<std::vector<std::uint8_t>>&,std::stop_token);
std::shared_ptr<const SequentialImage> encodeWave(const std::vector<std::vector<std::uint8_t>>&,int rate,std::stop_token);
}
