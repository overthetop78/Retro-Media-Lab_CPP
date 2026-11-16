#pragma once
#include "Stream.h"
#include "Constants.h"
namespace RetroMediaLab::MediaEngine::HxcStream {
void appendLittleEndian(std::vector<std::uint8_t>&,std::uint32_t,std::size_t width=Constants::FieldBytes);
void appendBlock(std::vector<std::uint8_t>&,std::uint32_t,std::span<const std::uint8_t>,std::optional<std::uint32_t> count={});
void appendPulse(std::vector<std::uint8_t>&,std::uint32_t);
}
