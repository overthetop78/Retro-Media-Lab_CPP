#pragma once
namespace RetroMediaLab::MediaFileSystems::FileCore::Layout {
inline constexpr int DiscRecordOffset = 4;
inline constexpr int DiscRecordLength = 60;
inline constexpr int BootBlockAddress = 0xc00;
inline constexpr int BootDiscRecordOffset = 0x1c0;
}
