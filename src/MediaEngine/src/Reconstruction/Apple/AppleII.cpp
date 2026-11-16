#include "Reconstructors.h"
#include "Constants.h"
#include "../../Formats/Apple/Geometry.h"
#include "../../Formats/FormatIds.h"
#include "../../Codecs/AppleII/Constants.h"
#include "../../Codecs/AppleRwts18/Constants.h"
#include "../../Models/Sectors/SectorImages.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::AppleScp {
namespace M=MediaFileSystems;namespace G=Apple::Geometry::AppleII;
namespace {
[[noreturn]] void unusable(std::string_view family,const IsoReconstruction::Candidates& candidates){throw MediaError(DiagnosticCode::ScpNoUsableSectors,{std::string(family),std::to_string(candidates.entries().size()),std::to_string(0)});}
std::shared_ptr<const M::SectorImage> prodos(const IsoReconstruction::Candidates& candidates,std::stop_token token){
    std::int32_t tracks=0;for(const auto& group:candidates.entries())if(group.address.cylinder<G::MaximumReconstructedTrackCount)tracks=std::max(tracks,static_cast<std::int32_t>(group.address.cylinder+1));
    if(tracks==0)unusable(Constants::ProDosName,candidates);tracks=std::max(tracks,G::TrackCount);std::vector<M::SectorBlock> blocks;
    for(std::int32_t track=0;track<tracks;++track)for(std::int32_t logical=0;logical<G::ProDosBlocksPerTrack;++logical){
        checkCancellation(token);M::SectorBlock block;block.logicalBlock=track*G::ProDosBlocksPerTrack+logical;block.address={track,0,logical};bool complete=true;block.integrityValid=true;
        for(std::int32_t half=0;half<G::SectorsPerProDosBlock;++half){
            const M::SectorAddress address{track,0,G::ProDosToPhysical[logical*G::SectorsPerProDosBlock+half]};const auto* values=candidates.find(address);if(!values){complete=false;break;}
            const auto selected=select(0,address,*values,token);block.data.insert(block.data.end(),selected.data.begin(),selected.data.end());block.integrityValid=block.integrityValid==true&&selected.integrityValid==true;block.revolution=std::max(block.revolution,selected.revolution);
        }
        if(complete)blocks.push_back(std::move(block));
    }
    if(blocks.empty())unusable(Constants::ProDosName,candidates);
    return sparseSectorImage({std::string(Formats::AppleIIProDos),G::ProDosBlockSize,static_cast<std::uint32_t>(tracks),Apple::Geometry::SingleHead,G::ProDosBlocksPerTrack},std::move(blocks));
}
}
std::shared_ptr<const M::SectorImage> appleII(const Scp::Image& image,const Decoder& decoder,bool prodosOrder,const IsoReconstruction::CollectionProgressCallback& progress,std::stop_token token){
    const auto candidates=decoder.collect(image,AppleII::Constants::Id,G::SectorSize,progress,token);if(candidates.empty())throw MediaError(DiagnosticCode::ScpNoDecodedSectors,{std::string(Constants::AppleIIName)});if(prodosOrder)return prodos(candidates,token);
    const bool sixAndTwo=std::any_of(candidates.entries().begin(),candidates.entries().end(),[](const auto& group){return group.address.number>=G::Dos32SectorsPerTrack;});const auto sectors=sixAndTwo?G::SectorsPerTrack:G::Dos32SectorsPerTrack;
    std::vector<M::SectorBlock> blocks;std::int32_t tracks=G::TrackCount;
    for(const auto& group:candidates.entries()){
        checkCancellation(token);const auto& a=group.address;if(a.cylinder>=G::MaximumReconstructedTrackCount||a.number<0||a.number>=sectors)continue;
        const auto logical=a.cylinder*sectors+(sixAndTwo?G::PhysicalToDos[static_cast<std::size_t>(a.number)]:a.number);blocks.push_back(select(static_cast<std::uint64_t>(logical),a,group.candidates,token));tracks=std::max(tracks,static_cast<std::int32_t>(a.cylinder+1));
    }
    if(blocks.empty())unusable(Constants::AppleIIName,candidates);
    return sparseSectorImage({std::string(sixAndTwo?Formats::AppleIIDos33:Formats::AppleIIDos32),G::SectorSize,static_cast<std::uint32_t>(tracks),Apple::Geometry::SingleHead,static_cast<std::uint32_t>(sectors)},std::move(blocks));
}
std::shared_ptr<const M::SectorImage> rwts18(const Scp::Image& image,const Decoder& decoder,const IsoReconstruction::CollectionProgressCallback& progress,std::stop_token token){
    namespace C=AppleRwts18::Constants;const auto candidates=decoder.collect(image,C::Id,C::SectorByteCount,progress,token);if(candidates.empty())throw MediaError(DiagnosticCode::ScpNoDecodedSectors,{std::string(Constants::RwtsSectorName)});
    std::vector<M::SectorBlock> blocks;std::int32_t tracks=G::TrackCount;
    for(const auto& group:candidates.entries()){
        checkCancellation(token);const auto& a=group.address;if(a.cylinder<0||a.cylinder>=G::MaximumReconstructedTrackCount||a.number<0||a.number>C::LastSectorNumber)continue;
        blocks.push_back(select(static_cast<std::uint64_t>(a.cylinder*C::SectorCount+a.number),a,group.candidates,token));tracks=std::max(tracks,static_cast<std::int32_t>(a.cylinder+1));
    }
    if(blocks.empty())unusable(Constants::RwtsSectorName,candidates);
    return sparseSectorImage({std::string(Formats::AppleIIRwts18),C::SectorByteCount,static_cast<std::uint32_t>(tracks),Apple::Geometry::SingleHead,C::SectorCount},std::move(blocks));
}
}
