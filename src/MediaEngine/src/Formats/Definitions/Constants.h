#pragma once
#include <array>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Definitions::Constants {
inline constexpr std::string_view Wildcard="*",Bps="bps",ImageBps="img_bps",Sectors="secs",FirstSector="id",Mode="mode",FmMode="fm";
inline constexpr std::string_view AppleGcr="apple2.gcr",DataGeneral="datageneral",Hp="hp.mmfm",Micropolis="micropolis",Northstar="northstar",IbmFm="ibm.fm",IbmMfm="ibm.mfm";
inline constexpr std::array RawCodecs{IbmFm,IbmMfm,AppleGcr,DataGeneral,Hp,Micropolis,Northstar};
inline constexpr std::int32_t SmallSector=256,DefaultSector=512,DataGeneralSectors=8,FirstIbmSector=1,DecimalBase=10;
inline constexpr char ListSeparator=',',HeadSeparator='.',RangeSeparator='-',RepeatSeparator='*',Comment='#',Assignment='=';
inline constexpr std::string_view DiskPattern=R"(^\s*disk\s+([\w,.-]+))",ImportPattern=R"r(^\s*import\s+([\w,.-]*)\s*"([^"]+)")r",WordsPattern=R"(\s+)";
inline constexpr std::string_view TracksPrefix="tracks ",End="end",Cylinders="cyls",Heads="heads",RootFile="diskdefs.cfg",ResourcePrefix=":/MediaEngine/DiskDefinitions/";
inline constexpr std::string_view Atari1440="atarist.1440",Mac1440="mac.1440",Ibm1440="ibm.1440",AppleSos="apple3.sos",AppleProDos140="apple2.prodos.140",AppleProDos800="apple2.prodos.800",Mac800="mac.800";
inline constexpr std::size_t TrackWords=3,HeadPart=1,RangeLastPart=1,RepeatCountPart=1,ImportPrefixGroup=1,ImportFileGroup=2,DiskIdGroup=1;
inline constexpr std::size_t SelectionWord=1,CodecWord=2;
inline constexpr std::string_view RawWriterId="defined-img",RawExtension=".img",CapacityField="capacity";
inline constexpr std::string_view BitcellCodec="bitcell";
}
