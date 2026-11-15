#pragma once
#include <string_view>
#include <cstdint>
#include <cstddef>
namespace RetroMediaLab::MediaEngine::ImageDisk::Constants {
inline constexpr std::string_view WriterId="imagedisk";
inline constexpr std::string_view Extension=".imd";
inline constexpr std::string_view Signature="IMD",DefaultComment="IMD Retro Media Lab",CommentPrefix="IMD ",MetadataKey="imagedisk.structure";
inline constexpr std::string_view CommentKey="Comment",TracksKey="Tracks",ModeKey="Mode",CylinderKey="Cylinder",HeadKey="Head",SectorsKey="Sectors",NumberKey="Number",SizeKey="Size",RecordTypeKey="RecordType";
inline constexpr std::uint8_t Terminator=0x1a,HeadMask=0x01,HeadMap=0x40,CylinderMap=0x80,ExplicitSize=0xff,MaxSizeCode=6,MaxByte=255,MaxMode=5,MaxRecord=8;
inline constexpr std::size_t SignatureSize=3,TrackHeader=5,ModeOffset=0,CylinderOffset=1,HeadFlagsOffset=2,CountOffset=3,SizeOffset=4,SizeMapEntry=2,BitsPerByte=8,BaseSize=128;
inline constexpr std::string_view Sections[]{"TrackHeader","SectorNumberMap","CylinderMap","HeadMap","SectorSizeMap","SectorRecord","CompressedValue","SectorData"};
inline constexpr std::uint32_t Rate250=250000,Rate300=300000,Rate500=500000,MaxSize=65535;
inline constexpr std::size_t MaxLength=2147483647,JsonMaxDepth=64,JsonQuotePairLength=2;
inline constexpr std::uint8_t AsciiMaximum=127,AsciiReplacement='?';
inline constexpr std::string_view JsonWhitespace=" \t\r\n";
inline constexpr char ObjectOpen='{',ObjectClose='}',ArrayOpen='[',ArrayClose=']',Quote='"',Escape='\\',Comma=',',Colon=':',Dot='.',ExponentLower='e',ExponentUpper='E';
}
