#pragma once
#include <string_view>
namespace RetroMediaLab::MediaEngine::MechanicalAudio::Constants {
inline constexpr std::string_view Tape="Tape",Floppy="Floppy",HardDisk="HardDisk",Optical="Optical",Motor="Motor";
inline constexpr std::string_view DefaultTape="Standard",DefaultFloppy="Floppy35",DefaultHardDisk="HardDisk35",DefaultOptical="OpticalTray";
inline constexpr std::string_view Flac=".flac",AssetsDirectory="Assets",BankDirectory="MechanicalAudio";
inline constexpr std::string_view Start="Start";
inline constexpr double MaximumVolumePercent=100,DefaultVolumePercent=50,PcmScale=32767;
inline constexpr double PcmMinimum=-32768;
}
