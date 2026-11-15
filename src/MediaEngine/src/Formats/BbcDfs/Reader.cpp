#include "BbcDfsImages.h"
#include "Constants.h"
#include "../../Models/Sectors/SectorImages.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaEngine::BbcDfs {
using namespace Constants;
namespace {
bool same(std::string_view a,std::string_view b){if(a.size()!=b.size())return false;const auto lower=[](char c){return c>=UpperFirst && c<=UpperLast?char(c+CaseDelta):c;};return std::equal(a.begin(),a.end(),b.begin(),[&](char x,char y){return lower(x)==lower(y);});}
std::uint64_t capacity(const Geometry& g){return std::uint64_t(g.cylinders)*g.heads*g.sectorsPerTrack*g.blockSize;}
Geometry geometry(const Profile& p){return {std::string(p.id),SectorSize,p.cylinders,p.heads,SectorsPerTrack};}
bool rawAlias(std::string_view extension){return same(extension,Ima) || same(extension,Img);}
std::optional<Geometry> rawGeometry(MediaFileSystems::Bytes header,std::uint64_t length){for(const auto& p:Profiles){auto g=geometry(p);if(capacity(g)==length && hasCatalogues(header,g))return g;}return {};}
}
std::vector<Geometry> geometries(){std::vector<Geometry> result;for(const auto& p:Profiles)result.push_back(geometry(p));return result;}
std::optional<Geometry> findGeometry(std::string_view id){for(const auto& p:Profiles)if(same(id,p.id))return geometry(p);return {};}
std::optional<Geometry> findGeometry(std::uint32_t heads,std::uint64_t length){for(const auto& p:Profiles){const auto g=geometry(p);if(g.heads==heads && capacity(g)==length)return g;}return {};}
Geometry observedGeometry(std::int64_t cylinders,std::int64_t heads){
    const auto count=cylinders>FortyCylinders?EightyCylinders:FortyCylinders;
    const std::uint32_t sides=heads>1?TwoHeads:1;
    for(const auto& p:Profiles)if(p.cylinders==count && p.heads==sides)return geometry(p);
    return geometry(Profiles.front());
}
bool hasCatalogues(MediaFileSystems::Bytes header,const Geometry& g){
    if(g.blockSize!=SectorSize || g.sectorsPerTrack!=SectorsPerTrack || !g.heads || g.heads>TwoHeads)return false;
    const auto expected=std::uint64_t(g.cylinders)*SectorsPerTrack;if(expected>std::numeric_limits<std::uint32_t>::max())return false;
    const auto length=std::uint64_t(g.heads-1)*TrackSize+HeaderSectors*SectorSize;if(header.size()<length)return false;
    for(std::uint32_t head=0;head<g.heads;++head){const auto offset=std::size_t(head)*TrackSize;
        if(!MediaFileSystems::BbcDfs::structuredCatalogue(header.subspan(offset,SectorSize),header.subspan(offset+SectorSize,SectorSize),std::uint32_t(expected)))return false;}
    return true;
}
bool canRead(MediaFileSystems::Bytes header,std::uint64_t length,std::string_view extension,std::optional<std::string_view> requested){
    if(requested && !findGeometry(*requested))return false;
    if(rawAlias(extension)){const auto found=rawGeometry(header,length);return found && (!requested || same(found->formatId,*requested));}
    if(same(extension,Adl)){const auto target=geometry(Profiles.back());return length==capacity(target) && (!requested || same(*requested,target.formatId)) && hasCatalogues(header,target);}
    if(!same(extension,Ssd) && !same(extension,Dsd))return false;
    const auto heads=same(extension,Ssd)?1:TwoHeads;return length>=SectorSize && length<=std::uint64_t(EightyCylinders)*heads*TrackSize;
}
Geometry detect(MediaFileSystems::Bytes header,std::uint64_t length,std::string_view extension){
    if(rawAlias(extension)){const auto result=rawGeometry(header,length);if(result)return *result;throw MediaError(DiagnosticCode::DfsUnknownExtension,{std::string(extension)});}
    const bool adl=same(extension,Adl) && length==capacity(geometry(Profiles.back())) && hasCatalogues(header,geometry(Profiles.back()));
    if(!adl && !same(extension,Ssd) && !same(extension,Dsd))throw MediaError(DiagnosticCode::DfsUnknownExtension,{std::string(extension)});
    const std::uint32_t heads=same(extension,Ssd)?1:TwoHeads;
    if(length<SectorSize)throw MediaError(DiagnosticCode::DfsIncompleteImage,{std::to_string(length),std::to_string(SectorSize)});
    auto required=length;
    for(std::uint32_t head=0;head<heads;++head){const auto offset=std::size_t(head)*TrackSize+SectorSize;if(header.size()<offset+SectorSize)continue;
        const auto catalogue=MediaFileSystems::BbcDfs::decodeCatalogue(header.subspan(offset,SectorSize));if(!catalogue.countValid)continue;
        const auto sectors=catalogue.totalSectors;if(sectors==SmallSectors || sectors==LargeSectors)required=std::max(required,std::uint64_t(sectors)*heads*SectorSize);}
    for(const auto& profile:Profiles){const auto g=geometry(profile);if(g.heads==heads && capacity(g)>=required)return g;}
    throw MediaError(DiagnosticCode::DfsUnsupportedGeometry,{std::to_string(length),std::to_string(heads)});
}
std::shared_ptr<const MediaFileSystems::SectorImage> read(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::string_view extension,std::stop_token token){
    checkCancellation(token);if(!source)throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::InvalidSource);
    const auto length=source->length();std::vector<std::uint8_t> header(std::size_t(std::min(std::uint64_t(TrackSize+HeaderSectors*SectorSize),length)));source->readExactly(0,header,token);checkCancellation(token);
    auto g=detect(header,length,extension);return sourceSectorImage(std::move(source),std::move(g),0,{},SourceSectorMetadata{0,0,true});
}
MediaFileSystems::FileSystemVolume explore(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::string_view extension,std::stop_token token){const auto image=read(std::move(source),extension,token);auto volume=MediaFileSystems::BbcDfs::read(*image,token);MediaFileSystems::analyzeEntries(image->geometry().formatId,volume,token);return volume;}
}
