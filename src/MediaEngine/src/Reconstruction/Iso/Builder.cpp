#include "Builder.h"
#include "Constants.h"
#include "../../Models/Sectors/SectorImages.h"
#include "../../Models/Sectors/SectorConstants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include "../../Primitives/Identifiers.h"
#include <algorithm>
#include <bit>
#include <set>
namespace RetroMediaLab::MediaEngine::IsoReconstruction {
namespace {
using Address=MediaFileSystems::SectorAddress;
void validate(const UniformRequest& r,const std::vector<MediaFileSystems::SectorBlock>& blocks){
    namespace M=MediaFileSystems;namespace S=SectorConstants;
    if(Primitives::isBlankIdentifier(r.formatId))throw M::MediaError(M::DiagnosticCode::SectorInvalidFormatId,{r.formatId});
    const auto dimension=[](std::int32_t value,std::string_view name){if(value<=0)throw M::MediaError(M::DiagnosticCode::SectorInvalidGeometry,{std::string(name),std::to_string(value)});};
    dimension(r.sectorSize,S::BlockSizeField);dimension(r.cylinders,S::CylindersField);dimension(r.heads,S::HeadsField);dimension(r.sectorsPerTrack,S::TrackSectorsField);
    if(r.capacity&&*r.capacity<=0)throw M::MediaError(M::DiagnosticCode::SectorInvalidGeometry,{std::string(S::CapacityField),std::to_string(*r.capacity)});
    std::set<std::uint64_t> seen;for(const auto& block:blocks)if(!seen.insert(block.logicalBlock).second)throw M::MediaError(M::DiagnosticCode::SectorDuplicateBlock,{std::to_string(block.logicalBlock)});
    const auto first=std::uint64_t(r.cylinders)*std::uint64_t(r.heads);
    if(first>Constants::MaximumBlockCount||std::uint64_t(r.sectorsPerTrack)>Constants::MaximumBlockCount/first)throw MediaError(DiagnosticCode::IsoGeometryOverflow,{std::to_string(r.cylinders),std::to_string(r.heads),std::to_string(r.sectorsPerTrack)});
}
std::int32_t sourceOffset(const Candidate& candidate){return std::bit_cast<std::int32_t>(std::uint32_t(candidate.sector.bitOffset));}
}
std::shared_ptr<const MediaFileSystems::SectorImage> createUniform(const Candidates& groups,const UniformRequest& r,std::stop_token token){
    checkCancellation(token);const auto& entries=groups.entries();std::vector<const Candidate*> selected;selected.reserve(entries.size());
    std::map<std::pair<std::int64_t,std::int64_t>,std::vector<std::size_t>> tracks;
    for(std::size_t index=0;index<entries.size();++index){checkCancellation(token);selected.push_back(&best(entries[index].candidates,token));const auto& a=entries[index].address;tracks[{a.cylinder,a.head}].push_back(index);}
    std::vector<std::int32_t> orders(entries.size());
    for(auto& [track,indices]:tracks){checkCancellation(token);std::stable_sort(indices.begin(),indices.end(),[&](auto a,auto b){return sourceOffset(*selected[a])<sourceOffset(*selected[b]);});for(std::size_t order=0;order<indices.size();++order)orders[indices[order]]=std::int32_t(order);}
    std::vector<MediaFileSystems::SectorBlock> blocks;
    for(std::size_t item=0;item<entries.size();++item){
        checkCancellation(token);const auto& address=entries[item].address;
        if(!r.allowSectorNumbersBeyondGeometry&&address.number>r.sectorsPerTrack)continue;
        if(address.cylinder>=r.cylinders||address.head>=r.heads)continue;
        if(!r.sectorIndex)throw MediaError(DiagnosticCode::IsoSectorIndexMissing);
        const auto index=r.sectorIndex(address);checkCancellation(token);if(index<0||index>=r.sectorsPerTrack)continue;
        const auto& candidate=*selected[item];const auto& sector=candidate.sector;if(!sector.data)throw MediaError(DiagnosticCode::IsoCandidateData);
        auto data=r.normalizeData?r.normalizeData(*sector.data):*sector.data;checkCancellation(token);if(data.empty())continue;
        const auto logical=std::bit_cast<std::int32_t>((std::uint32_t(address.cylinder)*std::uint32_t(r.heads)+std::uint32_t(address.head))*std::uint32_t(r.sectorsPerTrack)+std::uint32_t(index));
        MediaFileSystems::SectorBlock block;block.logicalBlock=std::uint64_t(logical);block.address=address;block.data=std::move(data);block.integrityValid=sector.integrityValid;block.revolution=candidate.revolution;
        block.headerIntegrityValid=sector.headerIntegrityValid;block.dataIntegrityValid=sector.dataIntegrityValid;block.deletedDataMark=sector.deletedDataMark;block.recordedAddress=Address{sector.cylinder,sector.head,sector.number};block.recordedTrackOrder=orders[item];block.recordedSizeCode=sector.sizeCode;blocks.push_back(std::move(block));
    }
    validate(r,blocks);MediaFileSystems::SectorImageOptions options;options.allowVariableBlockSize=r.allowVariableBlockSize;if(r.capacity)options.capacity=std::uint64_t(*r.capacity);
    return sparseSectorImage({r.formatId,std::uint32_t(r.sectorSize),std::uint32_t(r.cylinders),std::uint32_t(r.heads),std::uint32_t(r.sectorsPerTrack)},std::move(blocks),options);
}
}
