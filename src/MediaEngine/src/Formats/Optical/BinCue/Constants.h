#pragma once
#include "../../../Models/Optical/Track.h"
#include <array>
#include <string_view>
namespace RetroMediaLab::MediaEngine::BinCue::Constants {
inline constexpr std::string_view FormatId="optical-bin-cue",Extension=".cue",BinExtension=".bin",WaveExtension=".wav",OggExtension=".ogg",File="FILE",Track="TRACK",Index="INDEX",Pregap="PREGAP",Postgap="POSTGAP",Flags="FLAGS",Catalog="CATALOG",Isrc="ISRC",Title="TITLE",Performer="PERFORMER",Songwriter="SONGWRITER",Rem="REM",BinaryFile="BINARY",WaveFile="WAVE",VorbisFile="OGG",Loop="LOOP",NoLoop="NOLOOP",RemPrefix="REM:",RemLoop="REM:LOOP",RemNoLoop="REM:NOLOOP",Indent="  ";
inline constexpr std::string_view WriterId=FormatId,OutputTrack=".track",TrackCountKey="trackCount",FileCountKey="fileCount";
inline constexpr int RawBytes=2352,DataBytes=2048,Mode2Bytes=2336,RawUserOffset=16,TrackNumberWidth=2,TransferBytes=65536,CdChannels=2,CdRate=44100;
inline constexpr int NoSize=0,NoOffset=0;
inline int storedSize(OpticalTrackMode mode){switch(mode){case OpticalTrackMode::Audio:case OpticalTrackMode::Mode1Raw2352:case OpticalTrackMode::Mode2Raw2352:return RawBytes;case OpticalTrackMode::Mode1Data2048:return DataBytes;case OpticalTrackMode::Mode2Data2336:return Mode2Bytes;default:return NoSize;}}
inline int userOffset(OpticalTrackMode mode){return mode==OpticalTrackMode::Mode1Raw2352||mode==OpticalTrackMode::Mode2Raw2352?RawUserOffset:NoOffset;}
inline int userLength(OpticalTrackMode mode){return mode==OpticalTrackMode::Mode1Raw2352?DataBytes:mode==OpticalTrackMode::Mode2Raw2352?Mode2Bytes:storedSize(mode);}
inline constexpr char Quote='"',Colon=':',Space=' ',Tab='\t',Zero='0';
inline constexpr int TokenPair=2,TimeFields=3,SecondsPerMinute=60,FramesPerSecond=75,TimeWidth=2;
inline constexpr std::uint64_t MaximumSigned=0x7fffffffffffffff,MaximumInt=0x7fffffff;
struct ModeToken {std::string_view name;OpticalTrackMode mode;};
inline constexpr std::array<ModeToken,5> Modes{{{"AUDIO",OpticalTrackMode::Audio},{"MODE1/2048",OpticalTrackMode::Mode1Data2048},{"MODE1/2352",OpticalTrackMode::Mode1Raw2352},{"MODE2/2336",OpticalTrackMode::Mode2Data2336},{"MODE2/2352",OpticalTrackMode::Mode2Raw2352}}};
#ifdef _WIN32
inline constexpr std::string_view NewLine="\r\n";
#else
inline constexpr std::string_view NewLine="\n";
#endif
}
