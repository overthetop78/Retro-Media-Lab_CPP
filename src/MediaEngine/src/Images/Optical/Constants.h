#pragma once
#include <array>
#include <string_view>
#include <cstdint>
namespace RetroMediaLab::MediaEngine::OpticalSessionConstants {
inline constexpr std::array<std::string_view,5> Prefixes{""," ","-","_"," - "};
inline constexpr std::string_view IsoExtension=".iso",BinExtension=".bin",OggExtension=".ogg",WaveExtension=".wav",ChdFormat="optical-chd",LoopKey="REM:LOOP",NoLoopKey="REM:NOLOOP",TrackCountKey="trackCount",AudioCountKey="companionAudioCount";
inline constexpr int FirstAudioTrack=2,FormatCount=20,FormatsPerType=10,PrefixCount=5,NumberWidth=2,MaximumTracks=99,SectorBytes=2352,SubBytes=96,PregapFrames=150,ProbeFrame=100,ProbeBytes=4,Channels=2,SampleRate=44100,LoopDisabled=-1,LoopEnabled=1;
inline constexpr std::uint64_t MaximumSourceBytes=0x7fffffffffffffff;
inline constexpr std::int64_t MaximumCacheBytes=0x7fffffff;
inline constexpr char Zero='0';
}
