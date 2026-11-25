#pragma once
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaFileSystems::VolumeConstants {
inline constexpr std::uint32_t DefaultBlockSize = 512;
inline constexpr std::uint32_t MaximumBlockSize = 1024 * 1024;
inline constexpr std::string_view WholeMedia = "whole-media";
inline constexpr std::string_view DirectVolume = "direct-volume";
inline constexpr std::string_view Partition = "partition";
inline constexpr std::string_view OpticalTrack = "optical-track";
inline constexpr std::string_view SequentialContent = "sequential-content";
inline constexpr std::string_view Mbr = "mbr";
inline constexpr std::string_view Gpt = "gpt";
inline constexpr std::string_view Rdb = "rdb";
}
