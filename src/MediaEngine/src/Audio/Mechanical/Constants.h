#pragma once
#include <string_view>
namespace RetroMediaLab::MediaEngine::MechanicalAudio::Constants {
inline constexpr std::string_view Tape="Tape",Floppy="Floppy",HardDisk="HardDisk",Optical="Optical",Motor="Motor";
inline constexpr std::string_view DefaultTape="Standard",DefaultFloppy="Floppy35",DefaultHardDisk="HardDisk35",DefaultOptical="OpticalTray";
inline constexpr std::string_view Flac=".flac",AssetsDirectory="Assets",BankDirectory="MechanicalAudio";
}
