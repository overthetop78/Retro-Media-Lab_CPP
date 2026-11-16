#pragma once
#include <string_view>
#include <cstdint>
namespace RetroMediaLab::MediaEngine::AlcoholMds::Constants {
inline constexpr std::string_view FormatId="optical-alcohol-mds",Extension=".mds",DataExtension=".mdf",Signature="MEDIA DESCRIPTOR",WildcardPrefix="*.",HexPrefix="0x",VersionKey="version",MediumKey="mediumType",SessionsKey="sessionCount",TracksKey="trackCount";
inline constexpr int HeaderSize=92,SignatureOffset=0,SignatureLength=16,VersionOffset=16,MediumTypeOffset=18,SessionCountOffset=20,DiskStructureOffset=64,SessionsOffsetOffset=80,SessionBlockSize=24,SessionStartOffset=0,SessionEndOffset=4,SessionNumberOffset=8,SessionTotalBlocksOffset=10,SessionLeadInBlocksOffset=11,SessionFirstTrackOffset=12,SessionLastTrackOffset=14,SessionTrackBlocksOffset=20,TrackBlockSize=80,TrackModeOffset=0,TrackSubchannelModeOffset=1,TrackAdrControlOffset=2,TrackNumberOffset=4,TrackMinuteOffset=9,TrackSecondOffset=10,TrackFrameOffset=11,TrackExtraOffset=12,TrackSectorSizeOffset=16,TrackStartSectorOffset=36,TrackStartOffsetOffset=40,TrackSessionOffset=48,TrackFooterOffset=52,TrackFooterSize=16,FooterFileNameOffset=0,FooterWideCharacterOffset=4,MaximumMajorVersion=1,FramesPerSecond=75,SecondsPerMinute=60,MaximumNameBytes=4096,WideBytes=2,Data2048=2048,Data2336=2336,RawBytes=2352,SubBytes=96,RawUserOffset=16,Mode2=2,DataControl=4,HexWidth=4;
inline constexpr std::uint16_t DvdMediumFlag=0x10;
inline constexpr std::uint8_t LeadOut=0xa2;
inline constexpr std::uint64_t MaximumSigned=0x7fffffffffffffff;
inline constexpr char VersionSeparator='.',Zero='0';
}
