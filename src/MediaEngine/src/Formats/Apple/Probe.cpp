#include "Probe.h"
#include "Geometry.h"
#include "Constants.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaFileSystems/AppleRecognitionConstants.h>
namespace RetroMediaLab::MediaEngine::Apple {
namespace F=MediaFileSystems::Apple;
namespace C=F::RecognitionConstants;
bool looksLikeDos33(Bytes data) noexcept{
    if(data.size()!=Geometry::AppleII::Capacity)return false;
    constexpr auto offset=C::DosVtocTrack*Geometry::AppleII::SectorsPerTrack*Geometry::AppleII::SectorSize;
    return F::validDosVtoc(data.subspan(offset,Geometry::AppleII::SectorSize),Geometry::AppleII::TrackCount,Geometry::AppleII::SectorsPerTrack);
}
bool looksLikeProDos(Bytes data) noexcept{
    constexpr auto offset=C::ProDosRootBlock*C::BlockSize;
    return data.size()>=offset+C::BlockSize&&F::validProDosHeader(data.subspan(offset,C::BlockSize));
}
std::optional<F::MacintoshKind> probeMacintosh(Bytes data) noexcept{return F::identifyMacintosh(data);}
bool looksLikeLisaOffice(Bytes data) noexcept{
    if(data.size()!=C::LisaCapacity)return false;
    for(std::size_t offset=0;offset+C::LisaMinimumLength<=data.size();offset+=C::BlockSize)
        if(F::validLisaHeader(data.subspan(offset)))return true;
    return false;
}
bool looksLikeSos(Bytes data) noexcept{return data.size()==Geometry::AppleII::Capacity&&F::containsSosMarker(data);}
bool looksLikeImage(std::string_view extension,Bytes data,std::optional<std::string_view>) noexcept{
    using Primitives::asciiIdentifierEqualsIgnoreCase;
    if(asciiIdentifierEqualsIgnoreCase(extension,Constants::Img))return looksLikeLisaOffice(data)||((data.size()==Geometry::MacintoshGcr::Capacity400K||data.size()==Geometry::MacintoshGcr::Capacity800K||data.size()==Geometry::MacintoshGcr::Capacity1440K)&&probeMacintosh(data).has_value());
    if(!asciiIdentifierEqualsIgnoreCase(extension,Constants::Dsk))return false;
    return data.size()==Geometry::AppleII::Capacity||((data.size()==Geometry::MacintoshGcr::Capacity400K||data.size()==Geometry::MacintoshGcr::Capacity800K||data.size()==Geometry::MacintoshGcr::Capacity1440K)&&probeMacintosh(data).has_value());
}
}
