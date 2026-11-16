#pragma once
#include <string_view>
namespace RetroMediaLab::MediaEngine::MechanicalAudio::Constants {
inline constexpr std::string_view Tape="Tape",Floppy="Floppy",HardDisk="HardDisk",Optical="Optical",Motor="Motor";
inline constexpr std::string_view DefaultTape="Standard",DefaultFloppy="Floppy35",DefaultHardDisk="HardDisk35",DefaultOptical="OpticalTray";
inline constexpr std::string_view Flac=".flac",AssetsDirectory="Assets",BankDirectory="MechanicalAudio";
inline constexpr std::string_view Start="Start";
inline constexpr double MaximumVolumePercent=100,DefaultVolumePercent=50,PcmScale=32767;
inline constexpr double PcmMinimum=-32768;
inline constexpr std::string_view FlacSignature="fLaC",FloatCodec="pcm_f32le",FloatFormat="f32le",PipeOutput="pipe:1",Filter="-af",StereoToMono="pan=mono|c0=0.5*c0+0.5*c1",FailOnError="-xerror";
inline constexpr int FlacHeaderBytes=42,MetadataHeader=4,StreamInfoBytes=34,StreamRateOffset=18,StreamRateShift=44,StreamChannelsShift=41,FloatBytes=4,ByteBits=8,ChannelMask=7,MonoChannels=1,StereoChannels=2;
inline constexpr unsigned MetadataTypeMask=127,MetadataLengthMask=0x00ffffff;
}
