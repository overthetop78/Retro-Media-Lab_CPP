#pragma once
#include <array>
#include <string_view>
#include <RetroMediaLab/MediaFileSystems/CommodoreGeometryConstants.h>
namespace RetroMediaLab::MediaEngine::CommodoreDisks::Constants {
inline constexpr std::string_view D64="D64",D71="D71",D81="D81",SizeSeparator=", ";
inline constexpr std::string_view D64Extension=".d64",D71Extension=".d71",D81Extension=".d81",DosWriterId="commodore-dos",D81WriterId="commodore-d81";
inline constexpr std::array<std::string_view,4> NameKeys{"MediaEngine.CommodoreTracks","MediaEngine.CommodoreTracksErrors","MediaEngine.CommodoreSideTracks","MediaEngine.CommodoreSideTracksErrors"};
namespace DiskErrorCode {
inline constexpr std::uint8_t None=1,HeaderNotFound=2,SyncNotFound=3,DataNotFound=4,DataChecksumError=5,WriteVerifyError=6,WriteProtected=7,HeaderChecksumError=8,WriteError=9,DiskIdMismatch=10,DriveError=11,DriveNotReady=15;
}
inline constexpr std::uint8_t NoError=std::uint8_t(DiskErrorCode::None);
inline constexpr char UpperFirst='A',UpperLast='Z';
inline constexpr int CaseDelta='a'-'A';
}
