#pragma once
#include <string_view>
#include <cstdint>
#include <array>
namespace RetroMediaLab::MediaEngine::CloneCd::Constants {
inline constexpr std::string_view FormatId="optical-clonecd",Extension=".ccd",ImageExtension=".img",SubExtension=".sub",HeaderSection="CloneCD",DiscSection="Disc",SessionPrefix="Session ",EntryPrefix="Entry ",TrackPrefix="TRACK ",VersionKey="Version",SessionsKey="Sessions",EntriesKey="TocEntries",SessionKey="Session",PointKey="Point",ControlKey="Control",AdrKey="ADR",TrackNumberKey="TrackNo",AbsoluteSectorKey="PLBA",AbsoluteMinuteKey="AMin",AbsoluteSecondKey="ASec",AbsoluteFrameKey="AFrame",AbsoluteLbaKey="ALBA",ZeroKey="Zero",PointMinuteKey="PMin",PointSecondKey="PSec",PointFrameKey="PFrame",ModeKey="MODE",IsrcKey="ISRC",FlagsKey="FLAGS",IndexPrefix="INDEX ",CatalogKey="CATALOG",HexPrefix="0x",VersionMetadata="version",SessionsMetadata="sessionCount",TracksMetadata="trackCount",SubMetadata="subchannels",TrueValue="True",FalseValue="False";
inline constexpr int SectionBrackets=2;
inline constexpr int MinimumVersion=2,MaximumVersion=3,RawBytes=2352,SubBytes=96,RawUserOffset=16,Mode1UserBytes=2048,Mode2UserBytes=2336,DecimalBase=10,HexBase=16,AudioMode=0,Mode1Mode=1,Mode2Mode=2;
inline constexpr std::uint64_t MaximumSigned=0x7fffffffffffffff;
inline constexpr char Semicolon=';',Hash='#',Open='[',Close=']',Equals='=',Space=' ',Minus='-';
}
