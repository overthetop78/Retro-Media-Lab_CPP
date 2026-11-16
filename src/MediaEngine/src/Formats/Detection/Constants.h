#pragma once
#include "../FormatIds.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <utility>
namespace RetroMediaLab::MediaEngine::Detection::Constants {
inline constexpr std::string_view Domain="MediaEngine",RawScp="raw.scp",Scp=".scp",AdfExtension=".adf",Img=".img",Ima=".ima",St=".st",Msa=".msa",Atr=".atr",Atx=".atx",D13=".d13",Do=".do",Po=".po",TwoMg=".2mg",Image=".image",Dc42=".dc42",AtariPrefix="atarist.",AtariAtx="atari.atx";
inline constexpr std::string_view RawExplanation="Detection.RawScp",Multiple="Detection.Multiple",ExtensionInferred="Detection.ExtensionInferred",AcornSize="Detection.AcornSize",AmigaSize="Detection.AmigaSize",AtariUnknownSize="Detection.AtariUnknownSize",AtariSize="Detection.AtariSize",AppleDosOrder="Detection.AppleDosOrder",AppleProDosOrder="Detection.AppleProDosOrder",AppleContainer="Detection.AppleContainer",IbmAmbiguous="Detection.IbmAmbiguous",IbmSize="Detection.IbmSize";
inline constexpr std::pair<std::int64_t,std::string_view> StSizes[]{{368640,Formats::AtariSt360},{409600,Formats::AtariSt400},{450560,Formats::AtariSt440},{737280,Formats::AtariSt720},{819200,Formats::AtariSt800},{829440,Formats::AtariSt810},{901120,Formats::AtariSt880},{1474560,Formats::AtariSt1440}};
inline constexpr std::pair<std::int64_t,std::string_view> AtrSizes[]{{92176,Formats::Atari90},{133136,Formats::Atari130},{143376,Formats::Atari140},{183952,Formats::Atari180}};
inline constexpr std::pair<std::int64_t,std::string_view> IbmSizes[]{{163840,Formats::Ibm160},{184320,Formats::Ibm180},{327680,Formats::Ibm320},{368640,Formats::Ibm360},{737280,Formats::Ibm720},{819200,Formats::Ibm800},{1228800,Formats::Ibm1200},{1474560,Formats::Ibm1440},{1720320,Formats::Ibm1680},{2949120,Formats::Ibm2880}};
inline constexpr std::pair<std::int64_t,std::string_view> MacContainerSizes[]{{409684,Formats::Mac400},{419284,Formats::Mac400},{819284,Formats::Mac800},{838484,Formats::Mac800},{1474644,Formats::Mac1440},{1491844,Formats::Mac1440}};
inline constexpr std::pair<std::int64_t,std::string_view> MacRawSizes[]{{409600,Formats::Mac400},{819200,Formats::Mac800},{1474560,Formats::Mac1440}};
inline constexpr std::size_t AtxHeaderLength=4,MsaHeaderLength=10,MsaSectorsOffset=2,MsaHeadsOffset=4,MsaFirstCylinderOffset=6,MsaLastCylinderOffset=8,MacHeaderOffset=1024,MacSignatureLength=2;
inline constexpr std::uint16_t MsaSignature=0x0e0f,MinimumSectors=1,MaximumSectors=36,MaximumHeads=2;
inline constexpr std::uint16_t MacMfsSignature=0xd2d7,MacHfsSignature=0x4244;
inline constexpr std::uint32_t BytesPerKiB=1024,BytesPerSector=512;
inline constexpr std::array<std::uint8_t,AtxHeaderLength> AtxSignature{'A','T','8','X'};
}
