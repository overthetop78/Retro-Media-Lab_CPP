#include "CoherentImages.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Commodore900 {
using namespace Constants;
MediaFileSystems::SectorGeometry geometry(){return {std::string(FormatId),SectorSize,CylinderCount,HeadCount,MaximumSectors};}
bool Internal::sameFormat(std::string_view id){if(id.size()!=FormatId.size())return false;for(std::size_t i=0;i<id.size();++i){const auto c=id[i];if((c>=UpperFirst&&c<=UpperLast?char(c+CaseDelta):c)!=FormatId[i])return false;}return true;}
std::uint32_t sectorsPerTrack(std::int64_t cylinder){
 if(cylinder<0||cylinder>=CylinderCount)throw MediaError(DiagnosticCode::C900Cylinder,{std::to_string(cylinder),std::to_string(CylinderCount-1)});
 for(std::size_t zone=0;zone<ZoneEnds.size();++zone)if(cylinder<ZoneEnds[zone])return ZoneSectors[zone];return MinimumSectors;
}
MediaFileSystems::SectorAddress addressOf(std::int64_t logical){
 if(logical<0||logical>=BlockCount)throw MediaError(DiagnosticCode::C900Block,{std::to_string(logical),std::to_string(BlockCount-1)});
 auto remaining=std::uint32_t(logical);
 for(std::uint32_t cylinder=0;cylinder<CylinderCount;++cylinder){const auto sectors=sectorsPerTrack(cylinder),blocks=sectors*HeadCount;if(remaining<blocks)return {cylinder,remaining/sectors,remaining%sectors};remaining-=blocks;}
 throw MediaError(DiagnosticCode::C900Block,{std::to_string(logical),std::to_string(BlockCount-1)});
}
std::uint32_t logicalBlockOf(MediaFileSystems::SectorAddress address){
 const auto sectors=sectorsPerTrack(address.cylinder);
 if(address.head<0||address.head>=HeadCount||address.number<0||address.number>=sectors)throw MediaError(DiagnosticCode::C900Address,{std::to_string(address.cylinder),std::to_string(address.head),std::to_string(address.number),std::to_string(HeadCount-1),std::to_string(sectors-1)});
 std::uint32_t preceding=0;for(std::int64_t cylinder=0;cylinder<address.cylinder;++cylinder)preceding+=sectorsPerTrack(cylinder)*HeadCount;
 return preceding+std::uint32_t(address.head)*sectors+std::uint32_t(address.number);
}
}
