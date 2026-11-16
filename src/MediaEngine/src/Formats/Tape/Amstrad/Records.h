#pragma once
#include <vector>
#include <optional>
#include <span>
#include <cstdint>
#include <stop_token>
namespace RetroMediaLab::MediaEngine::AmstradTape {
struct Record {std::uint8_t sync;std::vector<std::uint8_t> data;bool valid;std::vector<std::int64_t> sources;};
std::optional<Record> decodeRecord(std::span<const std::uint8_t>,std::int64_t,std::stop_token);
std::vector<std::uint8_t> encodeRecord(std::uint8_t,std::span<const std::uint8_t>,std::stop_token);
}
