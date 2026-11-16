#pragma once
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::AtomicConstants {
inline constexpr std::uint64_t ChunkSize=64*1024;
inline constexpr char Separator='.';
inline constexpr std::string_view TemporaryExtension=".tmp",BackupExtension=".bak";
}
