#include "TrackEncoding.h"
#include "TrackEncodingConstants.h"
#include "Geometry.h"
#include "SectorOrder.h"
#include "TrackSelectionConstants.h"
#include "../FormatIds.h"
#include "../../Codecs/AppleII/Constants.h"
#include "../../Codecs/AppleRwts18/Constants.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <array>
#include <map>
namespace RetroMediaLab::MediaEngine::Apple {
namespace {
using Image=MediaFileSystems::SectorImage;
using Primitives::asciiIdentifierEqualsIgnoreCase;
[[noreturn]] void invalidSector(std::string_view family,std::uint32_t cylinder,std::uint32_t sector,std::size_t actual,std::uint32_t expected){throw MediaError(DiagnosticCode::AppleTrackSector,{std::string(family),std::to_string(cylinder),std::to_string(sector),std::to_string(actual),std::to_string(expected)});}
std::vector<Encoding::Sector> dos(const Image& image,std::uint32_t cylinder,std::uint32_t count){
    std::map<std::int64_t,std::shared_ptr<const MediaFileSystems::SectorBlock>> blocks;
    image.visitAvailableBlocks([&](auto logical){const auto block=image.block(logical);if(block&&block->address.cylinder==cylinder&&block->address.head==Geometry::Tagged::FirstHead)
        if(!blocks.emplace(block->address.number,block).second)throw MediaError(DiagnosticCode::AppleTrackDuplicate,{std::to_string(cylinder),std::to_string(block->address.number)});return true;});
    std::vector<Encoding::Sector> sectors;sectors.reserve(count);
    for(std::uint32_t sector=0;sector<count;++sector){const auto found=blocks.find(sector);if(found==blocks.end()||found->second->data.size()!=Geometry::AppleII::SectorSize)invalidSector(TrackEncodingConstants::StandardFamily,cylinder,sector,found==blocks.end()?0:found->second->data.size(),Geometry::AppleII::SectorSize);
        Encoding::Sector item;item.number=std::int32_t(sector);item.data=found->second->data;sectors.push_back(std::move(item));}
    return sectors;
}
std::vector<Encoding::Sector> prodos(const Image& image,std::uint32_t cylinder){
    std::vector<Encoding::Sector> sectors(Geometry::AppleII::SectorsPerTrack);
    for(std::uint32_t blockNumber=0;blockNumber<Geometry::AppleII::ProDosBlocksPerTrack;++blockNumber){
        const auto logical=std::uint64_t(cylinder)*Geometry::AppleII::ProDosBlocksPerTrack+blockNumber;const auto block=image.block(logical);
        if(!block||block->data.size()!=Geometry::AppleII::ProDosBlockSize)invalidSector(TrackEncodingConstants::StandardFamily,cylinder,blockNumber,block?block->data.size():0,Geometry::AppleII::ProDosBlockSize);
        for(std::int32_t part=0;part<Geometry::AppleII::SectorsPerProDosBlock;++part){const auto physical=proDosToPhysicalSector(std::int32_t(blockNumber)*Geometry::AppleII::SectorsPerProDosBlock+part);auto& sector=sectors[physical];sector.number=physical;
            const auto start=block->data.begin()+part*Geometry::AppleII::SectorSize;sector.data.assign(start,start+Geometry::AppleII::SectorSize);}
    }
    return sectors;
}
void maximum(std::string_view family,std::uint32_t cylinder,std::size_t bits,std::int32_t max){if(max<0||bits>std::size_t(max))throw MediaError(DiagnosticCode::AppleTrackTooLong,{std::string(family),std::to_string(cylinder),std::to_string(bits),std::to_string(max)});}
}
TrackEncodingService::TrackEncodingService()=default;
TrackEncodingService::TrackEncodingService(Encoding::Registry encoders):encoders_(std::move(encoders)){}
bool TrackEncodingService::supportsStandard(std::string_view id) noexcept{
    constexpr std::array ids{Formats::AppleIIDos32,Formats::AppleIIDos33,AppleII::Constants::Id,Formats::AppleIIAppleDos113,Formats::AppleIIAppleDos140,Formats::AppleIIProDos140,Formats::AppleIIProDos};
    for(const auto candidate:ids)if(asciiIdentifierEqualsIgnoreCase(id,candidate))return true;return false;
}
std::vector<Flux::Bits> TrackEncodingService::encodeStandard(const Image& image,std::int32_t max,std::stop_token token) const{
    const auto& g=image.geometry();if(!supportsStandard(g.formatId))throw MediaError(DiagnosticCode::AppleTrackUnsupported,{std::string(TrackEncodingConstants::StandardFamily),g.formatId});
    const auto count=asciiIdentifierEqualsIgnoreCase(g.formatId,Formats::AppleIIDos32)||asciiIdentifierEqualsIgnoreCase(g.formatId,Formats::AppleIIAppleDos113)?Geometry::AppleII::Dos32SectorsPerTrack:Geometry::AppleII::SectorsPerTrack;
    const auto expected=g.blockSize==Geometry::AppleII::ProDosBlockSize?Geometry::AppleII::ProDosBlocksPerTrack:count;
    if(g.heads!=Geometry::SingleHead||g.cylinders==0||g.sectorsPerTrack!=std::uint32_t(expected))throw MediaError(DiagnosticCode::AppleTrackGeometry,{std::to_string(g.cylinders),std::to_string(g.heads),std::to_string(g.sectorsPerTrack)});
    std::vector<Flux::Bits> tracks;tracks.reserve(g.cylinders);
    for(std::uint32_t cylinder=0;cylinder<g.cylinders;++cylinder){
        checkCancellation(token);Encoding::Request request;request.cylinder=std::int32_t(cylinder);request.head=Geometry::Tagged::FirstHead;request.sectors=g.blockSize==Geometry::AppleII::ProDosBlockSize?prodos(image,cylinder):dos(image,cylinder,std::uint32_t(count));request.attributes=std::map<std::string,std::int32_t>{{std::string(AppleII::Constants::SectorsPerTrackAttribute),count}};
        auto encoded=encoders_.encode(AppleII::Constants::Id,request,token);maximum(TrackEncodingConstants::StandardFamily,cylinder,encoded.bits.size(),max);tracks.push_back(std::move(encoded.bits));
    }
    return tracks;
}
std::vector<Flux::Bits> TrackEncodingService::encodeRwts18(const Image& image,std::int32_t max,std::stop_token token) const{
    if(!asciiIdentifierEqualsIgnoreCase(image.geometry().formatId,Formats::AppleIIRwts18))throw MediaError(DiagnosticCode::AppleTrackUnsupported,{std::string(TrackEncodingConstants::Rwts18Family),image.geometry().formatId});
    std::vector<Flux::Bits> tracks;tracks.reserve(image.geometry().cylinders);
    for(std::uint32_t cylinder=0;cylinder<image.geometry().cylinders;++cylinder){
        checkCancellation(token);Encoding::Request request;request.cylinder=std::int32_t(cylinder);request.head=Geometry::Tagged::FirstHead;
        for(std::uint32_t sector=0;sector<TrackSelectionConstants::RwtsSectorCount;++sector){const auto block=image.block(std::uint64_t(cylinder)*TrackSelectionConstants::RwtsSectorCount+sector);if(!block||block->data.size()!=TrackSelectionConstants::RwtsSectorSize)invalidSector(TrackEncodingConstants::Rwts18Family,cylinder,sector,block?block->data.size():0,TrackSelectionConstants::RwtsSectorSize);
            Encoding::Sector item;item.number=std::int32_t(sector);item.data=block->data;request.sectors.push_back(std::move(item));}
        auto encoded=encoders_.encode(AppleRwts18::Constants::Id,request,token);maximum(TrackEncodingConstants::Rwts18Family,cylinder,encoded.bits.size(),max);tracks.push_back(std::move(encoded.bits));
    }
    return tracks;
}
}
