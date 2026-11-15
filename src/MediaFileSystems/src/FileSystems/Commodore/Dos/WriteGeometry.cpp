#include "WriterInternal.h"
#include <QString>
namespace RetroMediaLab::MediaFileSystems::CommodoreDos::Writing {
namespace C=Commodore::Constants;
bool Geometry::is1581() const noexcept{return geometry.formatId==C::Format1581;}
std::int32_t Geometry::center() const noexcept{return is1581()?Constants::HeaderTrack1581:Constants::HeaderTrack1541;}
Geometry resolve(std::string_view format){
    const auto value=QString::fromUtf8(format.data(),qsizetype(format.size()));
    if(value.compare(QString::fromUtf8(C::Format1541),Qt::CaseInsensitive)==0)return {{std::string(C::Format1541),C::SectorSize,C::StandardTracks,1,C::MaximumSectors},Commodore::blocksPerSide1541(C::StandardTracks)};
    if(value.compare(QString::fromUtf8(C::Format1571),Qt::CaseInsensitive)==0)return {{std::string(C::Format1571),C::SectorSize,C::StandardTracks,C::TwoSides,C::MaximumSectors},Commodore::blocksPerSide1541(C::StandardTracks)*C::TwoSides};
    if(value.compare(QString::fromUtf8(C::Format1581),Qt::CaseInsensitive)==0)return {{std::string(C::Format1581),C::SectorSize,C::Cylinders1581,C::Heads1581,C::BlocksPerTrack1581},C::Cylinders1581*C::BlocksPerTrack1581};
    throw MediaError(DiagnosticCode::CbmWriteFormat,{std::string(format)});
}
std::int32_t Geometry::logical(std::int32_t track,std::int32_t sector) const{
    if(is1581())return Commodore::block1581(track,sector);
    const auto side=track>std::int32_t(geometry.cylinders)?1:0;
    const auto local=track-side*std::int32_t(geometry.cylinders);
    return geometry.heads==C::TwoSides?Commodore::block1571(local,sector,geometry.cylinders,side):Commodore::sideBlock1541(local,sector,geometry.cylinders);
}
std::pair<std::int32_t,std::int32_t> Geometry::address(std::int32_t block) const{
    if(is1581())return Commodore::address1581(block);
    const auto value=Commodore::address1541(block,geometry.cylinders,geometry.heads);
    return {value.track+value.side*std::int32_t(geometry.cylinders),value.sector};
}
SectorAddress Geometry::physical(std::int32_t block) const{
    if(is1581()){const auto [track,sector]=Commodore::address1581(block);return {track-1,0,sector};}
    const auto value=Commodore::address1541(block,geometry.cylinders,geometry.heads);
    return {value.track-1,value.side,value.sector};
}
}
