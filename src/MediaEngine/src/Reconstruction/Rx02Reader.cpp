#include "Rx02Reader.h"
#include "Iso/Builder.h"
#include "../Formats/Rx02/Rx02.h"
#include "../Formats/Rx02/SectorOrder.h"
#include "../Codecs/DecRx02/Constants.h"
#include "../Models/Sectors/SectorImages.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <array>
#include <limits>
#include <map>
namespace RetroMediaLab::MediaEngine::Rx02Reconstruction {
namespace C=Rx02::Constants;namespace M=MediaFileSystems;namespace I=IsoReconstruction;
namespace {
const auto& inverseOrder(){static const auto result=[]{std::array<std::uint32_t,C::PhysicalCount> values{};for(std::uint32_t logical=0;logical<C::PhysicalCount;++logical)values[Rx02::physicalOffset(logical)/C::PhysicalSize]=logical;return values;}();return result;}
}
std::shared_ptr<const M::SectorImage> read(const Scp::Image& image,const Decoding::DecodeOperation& decode,const I::CollectionProgressCallback& progress,std::stop_token token){
    checkCancellation(token);if(!decode)throw MediaError(DiagnosticCode::IsoDecoderMissing);
    if(image.tracks.size()>std::size_t(std::numeric_limits<std::int32_t>::max()))throw MediaError(DiagnosticCode::ScpOverflow);
    std::map<std::uint32_t,std::vector<I::Candidate>> candidates;std::int32_t completed=0;
    for(const auto& track:image.tracks){
        checkCancellation(token);
        Scp::visitDecodeWindows(track,[&](const auto& window){
            const auto decoded=decode(DecRx02::Constants::Id,std::make_shared<const Flux::Revolution>(window.flux));
            for(const auto& sector:decoded.sectors){
                checkCancellation(token);if(!sector.data||sector.data->size()!=C::PhysicalSize||sector.head!=0||sector.cylinder>=C::Tracks||sector.number<C::FirstSector||sector.number>C::PhysicalSectors)continue;
                const auto logical=inverseOrder()[std::size_t(sector.cylinder)*C::PhysicalSectors+std::size_t(sector.number-C::FirstSector)];candidates[logical].push_back({sector,window.revolution,{}});
            }
        },token);
        ++completed;if(progress)progress({I::CollectionProgressKind::TrackDecoded,completed,static_cast<std::int32_t>(image.tracks.size()),track.cylinder,track.head,0,static_cast<std::int32_t>(track.revolutions.size())});
    }
    if(candidates.empty())throw MediaError(DiagnosticCode::ScpNoDecodedSectors,{std::string(DecRx02::Constants::StructureName)});
    std::vector<M::SectorBlock> blocks;
    for(std::uint32_t logical=0;logical<C::LogicalCount;++logical){
        checkCancellation(token);const auto first=candidates.find(logical*C::Parts),second=candidates.find(logical*C::Parts+1);if(first==candidates.end()||second==candidates.end())continue;
        const auto& a=I::best(first->second,token);const auto& b=I::best(second->second,token);M::SectorBlock block;block.logicalBlock=logical;block.address={logical/C::LogicalPerTrack,0,logical%C::LogicalPerTrack+C::FirstSector};block.data=*a.sector.data;block.data.insert(block.data.end(),b.sector.data->begin(),b.sector.data->end());block.integrityValid=a.sector.integrityValid==true&&b.sector.integrityValid==true;block.revolution=std::max(a.revolution,b.revolution);blocks.push_back(std::move(block));
    }
    if(blocks.empty())throw MediaError(DiagnosticCode::ScpNoUsableSectors,{std::string(DecRx02::Constants::StructureName),std::to_string(candidates.size()),std::to_string(blocks.size())});
    M::SectorImageOptions options;options.capacity=C::Capacity;options.logicalBlockCount=C::LogicalCount;return sparseSectorImage(Rx02::geometry(),std::move(blocks),options);
}
}
