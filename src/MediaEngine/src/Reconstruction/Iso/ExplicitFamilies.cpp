#include "Policy.h"
#include "../../Formats/Adf/AdfImages.h"
#include "../../Formats/Adf/Constants.h"
#include "../../Formats/Raw/Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <map>
#include <set>
namespace RetroMediaLab::MediaEngine::IsoReconstruction {
namespace M=MediaFileSystems;
std::shared_ptr<const M::SectorImage> buildAdfs(std::string_view format,const CandidateSet& set,std::stop_token token){
    const auto profile=Adf::findGeometry(format);
    if(!profile)throw MediaError(DiagnosticCode::IsoRequestedGeometry,{std::string(format)});
    const auto& g=profile->geometry;const auto& candidates=g.blockSize==Adf::Constants::BbcBlockSize?set.physical:set.addressed;
    if(candidates.empty())throw MediaError(DiagnosticCode::IsoRequestedGeometry,{std::string(format)});
    for(const auto& group:candidates.entries()){
        checkCancellation(token);const auto& a=group.address;
        if(a.cylinder<0||a.cylinder>=g.cylinders||a.head<0||a.head>=g.heads||a.number<profile->firstSectorNumber||a.number>=profile->firstSectorNumber+g.sectorsPerTrack)throw MediaError(DiagnosticCode::IsoRequestedGeometry,{std::string(format)});
        for(const auto& candidate:group.candidates)if(!candidate.sector.data||candidate.sector.data->size()!=g.blockSize)throw MediaError(DiagnosticCode::IsoRequestedGeometry,{std::string(format)});
    }
    UniformRequest request;request.formatId=g.formatId;request.sectorSize=static_cast<std::int32_t>(g.blockSize);request.cylinders=static_cast<std::int32_t>(g.cylinders);request.heads=static_cast<std::int32_t>(g.heads);request.sectorsPerTrack=static_cast<std::int32_t>(g.sectorsPerTrack);request.sectorIndex=[first=profile->firstSectorNumber](auto a){return static_cast<std::int32_t>(a.number-first);};return createUniform(candidates,request,token);
}
std::shared_ptr<const M::SectorImage> buildGeneric(std::string format,const CandidateSet& set,bool amstrad,std::stop_token token){
    const auto m=measure(set.addressed,token);UniformRequest request;request.formatId=std::move(format);request.sectorSize=m.sectorSize;request.cylinders=m.cylinders;request.heads=m.heads;request.sectorsPerTrack=m.sectorsPerTrack;request.allowSectorNumbersBeyondGeometry=amstrad;
    request.sectorIndex=[m,amstrad](auto a){if(!amstrad&&!m.zeroBased)return static_cast<std::int32_t>(a.number-1);const auto found=std::find(m.sectorOrder.begin(),m.sectorOrder.end(),a.number);return found==m.sectorOrder.end()?-1:static_cast<std::int32_t>(found-m.sectorOrder.begin());};return createUniform(set.addressed,request,token);
}
std::shared_ptr<const M::SectorImage> buildUcsd(std::string format,const CandidateSet& set,std::stop_token token){
    const auto m=measure(set.physical,token);std::map<std::pair<std::int64_t,std::int64_t>,std::set<std::int64_t>> numbers;
    for(const auto& group:set.physical.entries()){checkCancellation(token);numbers[{group.address.cylinder,group.address.head}].insert(group.address.number);}
    UniformRequest request;request.formatId=std::move(format);request.sectorSize=m.sectorSize;request.cylinders=m.cylinders;request.heads=Raw::Constants::UcsdHeads;request.sectorsPerTrack=Raw::Constants::UcsdSectors;
    request.sectorIndex=[numbers=std::move(numbers)](auto a){const auto track=numbers.find({a.cylinder,a.head});if(track==numbers.end())return -1;const auto sector=track->second.find(a.number);return sector==track->second.end()?-1:static_cast<std::int32_t>(std::distance(track->second.begin(),sector));};return createUniform(set.physical,request,token);
}
}
