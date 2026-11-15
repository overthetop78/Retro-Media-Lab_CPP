#include "RawImages.h"
#include "Constants.h"
#include <RetroMediaLab/MediaFileSystems/Fat.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Raw {
using namespace Constants;
namespace {
std::uint64_t capacity(const Profile& p){return std::uint64_t(p.cylinders)*p.heads*p.sectors*SectorSize;}
Geometry geometry(const Profile& p){return {std::string(p.id),SectorSize,p.cylinders,p.heads,p.sectors};}
std::string geometryId(std::uint32_t c,std::uint32_t h,std::uint32_t s){const auto length=std::uint64_t(c)*h*s*SectorSize;const auto known=ibmFromCapacity(length);return known?known->formatId:std::string(IbmPrefix)+std::to_string(length/BytesPerKibibyte);}
}
std::vector<Geometry> ibmGeometries(){std::vector<Geometry> result;for(const auto& p:Ibm)result.push_back(geometry(p));return result;}
std::vector<Geometry> msxGeometries(){std::vector<Geometry> result;for(const auto& p:Msx)result.push_back(geometry(p));return result;}
std::optional<Geometry> ibmFromCapacity(std::uint64_t length){for(const auto& p:Ibm)if(p.id!=IbmDmf && capacity(p)==length)return geometry(p);return {};}
std::optional<Geometry> ibmFromDescriptor(std::uint8_t descriptor){switch(descriptor){case Ibm160Descriptor:return *ibmFromCapacity(Ibm160Capacity);case Ibm180Descriptor:return *ibmFromCapacity(Ibm180Capacity);case Ibm320Descriptor:return *ibmFromCapacity(Ibm320Capacity);case Ibm360Descriptor:return *ibmFromCapacity(Ibm360Capacity);default:return {};}}
std::optional<Geometry> ibmFromBoot(MediaFileSystems::Bytes boot,std::uint8_t descriptor){const auto bpb=MediaFileSystems::Fat::detectBpbGeometry(boot);if(bpb)return Geometry{geometryId(bpb->cylinders,bpb->heads,bpb->sectorsPerTrack),SectorSize,bpb->cylinders,bpb->heads,bpb->sectorsPerTrack};return ibmFromDescriptor(descriptor);}
Geometry detectIbm(MediaFileSystems::Bytes boot,std::uint64_t length){
    if(!length || length%SectorSize!=0)throw MediaError(DiagnosticCode::RawInvalidLength,{std::to_string(length),std::to_string(SectorSize)});
    const auto bpb=MediaFileSystems::Fat::detectBpbGeometry(boot,length);
    if(bpb)return {geometryId(bpb->cylinders,bpb->heads,bpb->sectorsPerTrack),SectorSize,bpb->cylinders,bpb->heads,bpb->sectorsPerTrack};
    const auto known=ibmFromCapacity(length);if(known)return *known;
    throw MediaError(DiagnosticCode::RawUnknownGeometry,{std::to_string(length)});
}
bool looksLikeMsx(MediaFileSystems::Bytes boot,std::uint64_t length){
    if(length<SectorSize || length%SectorSize!=0 || boot.size()<SectorSize)return false;
    for(std::size_t i=0;i<MsxOem.size();++i){auto value=char(boot[OemOffset+i]);if(value>=UpperFirst && value<=UpperLast)value=char(value+CaseDelta);auto expected=char(MsxOem[i]+CaseDelta);if(value!=expected)return false;}
    return MediaFileSystems::Fat::detectBpbGeometry(boot).has_value();
}
std::optional<Geometry> msxFromCapacity(std::uint64_t length,std::uint8_t descriptor){for(const auto& p:Msx)if(capacity(p)==length && (length!=AmbiguousMsxCapacity || (descriptor==MsxOneDoubleDensity)==(p.id==Msx1Dd)))return geometry(p);return {};}
}
