#pragma once
#include <string_view>
#include <cstdint>
namespace RetroMediaLab::MediaEngine::Gdi::Constants {
inline constexpr std::string_view FormatId="optical-gdi",WriterId="optical-gdi",Extension=".gdi",BinExtension=".bin",RawExtension=".raw",MediaProfile="Dreamcast GD-ROM",ProfileKey="mediaProfile",TrackCountKey="trackCount",TrackPrefix="track.",FileSuffix=".file",LbaSuffix=".lba",ControlSuffix=".control",SectorSizeSuffix=".sectorSize",OutputTrack=".track";
inline constexpr int FieldCount=6,MinimumLines=2,AudioControl=0,DataControl=4,DataBytes=2048,RawBytes=2352,RawUserOffset=16,TrackNumberWidth=2,NumberField=0,LbaField=1,ControlField=2,SizeField=3,FileField=4,OffsetField=5,TransferBytes=64*1024;
inline constexpr std::uint64_t MaximumSigned=0x7fffffffffffffff,MaximumInt=0x7fffffff;
inline constexpr char Comment='#',Quote='"',Separator=' ',PathSeparator='/',Zero='0',ExtensionSeparator='.';
#ifdef _WIN32
inline constexpr std::string_view NewLine="\r\n";
#else
inline constexpr std::string_view NewLine="\n";
#endif
}
