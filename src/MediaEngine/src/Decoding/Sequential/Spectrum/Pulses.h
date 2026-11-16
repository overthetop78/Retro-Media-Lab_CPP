#pragma once
#include <vector>
#include <span>
#include <cstdint>
#include <stop_token>
namespace RetroMediaLab::MediaEngine::SpectrumTape {
std::vector<std::vector<std::uint8_t>> detect(std::span<const std::int32_t>,int,std::stop_token);
}
