#pragma once
#include <vector>
#include <span>
#include <cstdint>
#include <stop_token>
namespace RetroMediaLab::MediaEngine::CommodoreTape {
struct Pulse {int cycles;std::int64_t source;};
struct FrameBlock {std::vector<std::uint8_t> data;std::vector<std::int64_t> sources;bool parityValid;};
std::vector<FrameBlock> decode(std::span<const Pulse>,std::stop_token);
std::vector<int> encode(std::span<const std::uint8_t>,std::stop_token);
}
