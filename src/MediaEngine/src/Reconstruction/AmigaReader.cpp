#include "AmigaReader.h"
#include "Iso/Builder.h"
#include "../Codecs/Amiga/Constants.h"
#include "../Formats/Adf/AdfImages.h"
#include "../Models/Sectors/SectorImages.h"
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <map>
#include <set>
#include <limits>
namespace RetroMediaLab::MediaEngine::AmigaReconstruction {
namespace M=MediaFileSystems;namespace C=AmigaMfm::Constants;
std::int32_t inferSectorsPerTrack(std::span<const M::SectorAddress> addresses){
    const auto geometry=Adf::findGeometry(M::Formats::AmigaDos)->geometry;
    std::map<std::pair<std::int64_t,std::int64_t>,std::set<std::int64_t>> tracks;
    for(const auto& address:addresses)if(address.cylinder<geometry.cylinders&&address.head<geometry.heads&&address.number>=0&&address.number<C::HighDensitySectorsPerTrack)tracks[{address.cylinder,address.head}].insert(address.number);
    std::int32_t convincing=0;
    for(const auto& [track,numbers]:tracks){static_cast<void>(track);if(numbers.size()>=C::HighDensityCredibleSectorCount&&*numbers.rbegin()>=C::DoubleDensitySectorsPerTrack)++convincing;}
    return convincing>=C::HighDensityCredibleTrackCount?C::HighDensitySectorsPerTrack:C::DoubleDensitySectorsPerTrack;
}
std::shared_ptr<const M::SectorImage> read(const Scp::Image& image,const Decoding::DecodeOperation& decode,std::stop_token token,const IsoReconstruction::CollectionProgressCallback& progress){
    if(!decode)throw MediaError(DiagnosticCode::IsoDecoderMissing);
    checkCancellation(token);
    std::int32_t completedTracks=0,completedRevolutions=0,totalRevolutions=0;
    if(image.tracks.size()>static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))throw MediaError(DiagnosticCode::ScpOverflow);
    for(const auto& track:image.tracks){if(track.revolutions.size()>static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()-totalRevolutions))throw MediaError(DiagnosticCode::ScpOverflow);totalRevolutions+=static_cast<std::int32_t>(track.revolutions.size());}
    IsoReconstruction::Candidates candidates;
    for(const auto& track:image.tracks){checkCancellation(token);Scp::visitDecodeWindows(track,[&](const auto& window){
        checkCancellation(token);const auto result=decode(C::Id,std::make_shared<const Flux::Revolution>(window.flux));
        for(const auto& sector:result.sectors)if(sector.data&&sector.data->size()==C::SectorByteCount&&sector.cylinder==track.cylinder&&sector.head==track.head)candidates.add({sector.cylinder,sector.head,sector.number},{sector,window.revolution,{}});
        ++completedRevolutions;
        if(progress)progress({IsoReconstruction::CollectionProgressKind::RevolutionDecoded,completedRevolutions,totalRevolutions,track.cylinder,track.head,window.revolution,static_cast<std::int32_t>(track.revolutions.size())});
    },token);
        ++completedTracks;
        if(progress)progress({IsoReconstruction::CollectionProgressKind::TrackDecoded,completedTracks,static_cast<std::int32_t>(image.tracks.size()),track.cylinder,track.head,0,static_cast<std::int32_t>(track.revolutions.size())});
    }
    if(candidates.empty())throw MediaError(DiagnosticCode::ScpNoDecodedSectors,{std::string(C::StructureName)});
    std::vector<M::SectorAddress> addresses;for(const auto& group:candidates.entries())addresses.push_back(group.address);
    const auto sectors=inferSectorsPerTrack(addresses);
    const auto g=Adf::findGeometry(sectors==C::HighDensitySectorsPerTrack?M::Formats::AmigaDosHighDensity:M::Formats::AmigaDos)->geometry;
    std::vector<M::SectorBlock> blocks;
    for(const auto& group:candidates.entries()){
        checkCancellation(token);const auto& a=group.address;if(a.cylinder>=g.cylinders||a.head>=g.heads||a.number<0||a.number>=sectors)continue;
        const auto& selected=IsoReconstruction::best(group.candidates,token);M::SectorBlock block;block.logicalBlock=static_cast<std::uint64_t>((a.cylinder*g.heads+a.head)*sectors+a.number);block.address=a;block.data=*selected.sector.data;block.integrityValid=selected.sector.integrityValid;block.revolution=selected.revolution;blocks.push_back(std::move(block));
    }
    if(blocks.empty())throw MediaError(DiagnosticCode::ScpNoUsableSectors,{std::string(C::StructureName),std::to_string(candidates.entries().size()),std::to_string(blocks.size())});
    return sparseSectorImage(g,std::move(blocks));
}
}
