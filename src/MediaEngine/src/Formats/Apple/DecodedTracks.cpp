#include "DecodedTracks.h"
#include "TrackSelectionConstants.h"
#include "Geometry.h"
#include "Probe.h"
#include "SectorOrder.h"
#include "../FormatIds.h"
#include "../../Codecs/AppleII/Gcr.h"
#include "../../Codecs/AppleRwts18/Gcr.h"
#include "../../Models/Sectors/Builders.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <map>
#include <set>
namespace RetroMediaLab::MediaEngine::Apple {
namespace Constants=TrackSelectionConstants;
namespace {
using Sector=Decoding::Sector;
using Block=MediaFileSystems::SectorBlock;
using Selected=SelectedSector;
void select(std::vector<Selected>& result,std::map<std::pair<std::int32_t,std::int32_t>,std::size_t>& indices,std::int32_t track,const std::vector<Decoding::Sector>& decoded,const std::function<bool(const Sector&)>& accept){
    for(const auto& sector:decoded){
        if(!accept(sector))continue;
        const auto key=std::pair{track,sector.number};const auto found=indices.find(key);
        if(found==indices.end()){indices.emplace(key,result.size());result.push_back({track,sector});}
        else if(sector.integrityValid==true&&result[found->second].sector.integrityValid!=true)result[found->second].sector=sector;
    }
}
std::shared_ptr<const MediaFileSystems::SectorImage> build(const std::vector<Selected>& selected,bool rwts){
    namespace G=Apple::Geometry;
    std::int32_t tracks=G::AppleII::TrackCount,maxSector=-1;
    for(const auto& item:selected){tracks=std::max(tracks,item.track+1);maxSector=std::max(maxSector,item.sector.number);}
    const auto sectors=rwts?Constants::RwtsSectorCount:!selected.empty()&&maxSector<G::AppleII::Dos32SectorsPerTrack?G::AppleII::Dos32SectorsPerTrack:G::AppleII::SectorsPerTrack;
    const auto size=rwts?Constants::RwtsSectorSize:G::AppleII::SectorSize;
    std::vector<Block> blocks;blocks.reserve(selected.size());
    for(const auto& item:selected){
        Block block;block.logicalBlock=std::uint64_t(item.track)*sectors+(!rwts&&sectors==G::AppleII::SectorsPerTrack?Apple::physicalToDosFileSector(item.sector.number):item.sector.number);
        block.address={item.track,0,item.sector.number};block.data=*item.sector.data;block.integrityValid=item.sector.integrityValid;blocks.push_back(std::move(block));
    }
    auto id=rwts?Formats::AppleIIRwts18:selected.empty()?Constants::AppleIIGcr:sectors==G::AppleII::Dos32SectorsPerTrack?Formats::AppleIIDos32:Constants::AppleIIGcr;
    if(!rwts&&sectors==G::AppleII::SectorsPerTrack&&!selected.empty()){
        std::map<std::pair<std::int32_t,std::int32_t>,const Sector*> byAddress;
        for(const auto& item:selected)byAddress.emplace(std::pair{item.track,item.sector.number},&item.sector);
        std::vector<Block> proDos;
        for(std::int32_t track=0;track<tracks;++track)for(std::int32_t logical=0;logical<G::AppleII::ProDosBlocksPerTrack;++logical){
            const auto first=byAddress.find({track,Apple::proDosToPhysicalSector(logical*G::AppleII::SectorsPerProDosBlock)});
            const auto second=byAddress.find({track,Apple::proDosToPhysicalSector(logical*G::AppleII::SectorsPerProDosBlock+1)});
            if(first==byAddress.end()||second==byAddress.end())continue;
            Block block;block.logicalBlock=std::uint64_t(track)*G::AppleII::ProDosBlocksPerTrack+logical;block.address={track,0,logical};block.data=*first->second->data;
            block.data.insert(block.data.end(),second->second->data->begin(),second->second->data->end());block.integrityValid=first->second->integrityValid==true&&second->second->integrityValid==true;proDos.push_back(std::move(block));
        }
        if(proDos.size()==std::size_t(tracks)*G::AppleII::ProDosBlocksPerTrack){
            const auto dense=denseSectorImage(proDos,tracks*G::AppleII::ProDosBlocksPerTrack,G::AppleII::ProDosBlockSize);
            if(Apple::looksLikeProDos(dense.data))return sparseSectorImage({std::string(Formats::AppleIIProDos),G::AppleII::ProDosBlockSize,std::uint32_t(tracks),G::SingleHead,G::AppleII::ProDosBlocksPerTrack},std::move(proDos));
        }
        if(blocks.size()==std::size_t(tracks)*G::AppleII::SectorsPerTrack){
            const auto dense=denseSectorImage(blocks,tracks*G::AppleII::SectorsPerTrack,G::AppleII::SectorSize);
            if(Apple::looksLikeDos33(dense.data))id=Formats::AppleIIDos33;
        }
    }
    return sparseSectorImage({std::string(id),std::uint32_t(size),std::uint32_t(tracks),G::SingleHead,std::uint32_t(sectors)},std::move(blocks));
}
std::int32_t score(const std::vector<Decoding::Sector>& sectors){
    std::set<std::int32_t> distinct;std::int32_t valid=0;
    for(const auto& sector:sectors){distinct.insert(sector.number);if(sector.integrityValid==true)++valid;}
    return std::int32_t(distinct.size())*Constants::DistinctSectorScoreWeight+valid*Constants::IntegrityScoreWeight+std::int32_t(sectors.size());
}
std::vector<Decoding::Sector> filter(Decoding::Result result,std::int32_t track,std::int32_t count,std::size_t size){
    std::vector<Decoding::Sector> sectors;
    for(auto& sector:result.sectors)if(sector.cylinder==track&&sector.number>=0&&sector.number<count&&sector.data&&sector.data->size()==size)sectors.push_back(std::move(sector));
    return sectors;
}
}
TrackCandidates decodeTrack(Flux::BitView bits,std::int32_t track,std::stop_token token){
    auto standard=filter(AppleII::decodeBits(bits,token),track,Geometry::AppleII::SectorsPerTrack,Geometry::AppleII::SectorSize);
    auto rwts=filter(AppleRwts18::decodeBits(bits,token),track,Constants::RwtsSectorCount,Constants::RwtsSectorSize);
    const auto standardScore=score(standard),rwtsScore=score(rwts);return {std::move(standard),std::move(rwts),standardScore,rwtsScore};
}
std::vector<SelectedSector> selectBestDecoded(std::span<const DecodedTrack> tracks,const std::function<bool(const Decoding::Sector&)>& accept){
    std::vector<Selected> selected;std::map<std::pair<std::int32_t,std::int32_t>,std::size_t> indices;
    for(const auto& track:tracks)select(selected,indices,track.track,track.sectors,accept);return selected;
}
std::shared_ptr<const MediaFileSystems::SectorImage> buildStandard(std::span<const DecodedTrack> tracks){
    const auto selected=selectBestDecoded(tracks,[](const auto& sector){return sector.number>=0&&sector.number<Geometry::AppleII::SectorsPerTrack&&sector.data&&sector.data->size()==Geometry::AppleII::SectorSize;});return build(selected,false);
}
std::shared_ptr<const MediaFileSystems::SectorImage> buildRwts18(std::span<const DecodedTrack> tracks){
    const auto selected=selectBestDecoded(tracks,[](const auto& sector){return sector.number>=0&&sector.number<Constants::RwtsSectorCount&&sector.data&&sector.data->size()==Constants::RwtsSectorSize;});std::size_t count=0;for(const auto& track:tracks)count+=track.sectors.size();
    if(selected.empty())throw MediaError(DiagnosticCode::AppleNoRwtsSectors,{std::to_string(count),std::to_string(tracks.size())});return build(selected,true);
}
}
