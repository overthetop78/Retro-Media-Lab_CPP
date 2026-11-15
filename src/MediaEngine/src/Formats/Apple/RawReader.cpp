#include "Raw.h"
#include <limits>
#include "Geometry.h"
#include "SectorOrder.h"
#include "Constants.h"
#include "../FormatIds.h"
#include "../../Models/Sectors/SectorImages.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Apple {
namespace {
ReadResult build(Bytes data,std::string_view id,MatchKind match,bool zoned,std::uint32_t heads,std::uint32_t sectorBytes,std::uint32_t sectors,std::uint32_t cylinders,std::stop_token token){
    std::vector<MediaFileSystems::SectorBlock> blocks;
    const auto count=data.size()/sectorBytes;blocks.reserve(count);
    for(std::size_t logical=0;logical<count;++logical){
        checkCancellation(token);MediaFileSystems::SectorBlock block;block.logicalBlock=logical;
        if(zoned)block.address=Geometry::MacintoshGcr::address(std::int32_t(logical),std::int32_t(heads));
        else block.address={std::int64_t(logical/(heads*sectors)),std::int64_t(logical/sectors%heads),std::int64_t(logical%sectors)};
        const auto payload=data.subspan(logical*sectorBytes,sectorBytes);block.data.assign(payload.begin(),payload.end());blocks.push_back(std::move(block));
    }
    MediaFileSystems::SectorImageOptions options;options.capacity=data.size();options.logicalBlockCount=count;
    return {sparseSectorImage({std::string(id),sectorBytes,cylinders,heads,sectors},std::move(blocks),options),match};
}
ReadResult appleII(Bytes data,std::string_view id,MatchKind match,bool prodos,std::stop_token token){return build(data,id,match,false,Geometry::SingleHead,prodos?Geometry::AppleII::ProDosBlockSize:Geometry::AppleII::SectorSize,prodos?Geometry::AppleII::ProDosBlocksPerTrack:Geometry::AppleII::SectorsPerTrack,Geometry::AppleII::TrackCount,token);}
}
ReadResult readRaw(Bytes data,std::string_view extension,std::stop_token token){
    checkCancellation(token);
    const auto* layout=data.size()<=std::size_t(std::numeric_limits<std::int32_t>::max())?Geometry::findLayout(std::int32_t(data.size())):nullptr;
    if(!layout)throw MediaError(DiagnosticCode::RawUnknownGeometry,{std::to_string(data.size())});
    using Primitives::asciiIdentifierEqualsIgnoreCase;
    if(data.size()==Geometry::AppleII::Dos32Capacity)return build(data,Formats::AppleIIDos32,MatchKind::GeometryFallback,false,Geometry::SingleHead,Geometry::AppleII::SectorSize,Geometry::AppleII::Dos32SectorsPerTrack,Geometry::AppleII::TrackCount,token);
    if(data.size()==Geometry::AppleII::Capacity){
        if(asciiIdentifierEqualsIgnoreCase(extension,Constants::Po))return appleII(data,looksLikeSos(data)?Formats::AppleIIISos:Formats::AppleIIProDos,looksLikeSos(data)?MatchKind::ValidatedStructure:MatchKind::ExtensionHint,true,token);
        if(looksLikeProDos(data))return appleII(data,Formats::AppleIIProDos,MatchKind::ValidatedStructure,true,token);
        if(looksLikeDos33(data))return appleII(data,Formats::AppleIIDos33,MatchKind::ValidatedStructure,false,token);
        const auto converted=dosToProDos(data);
        if(looksLikeSos(converted))return appleII(converted,Formats::AppleIIISos,MatchKind::ValidatedStructure,true,token);
        if(looksLikeProDos(converted))return appleII(converted,Formats::AppleIIProDos,MatchKind::ValidatedStructure,true,token);
        return appleII(data,Formats::AppleIIDos33,MatchKind::GeometryFallback,false,token);
    }
    const bool single=data.size()==Geometry::MacintoshGcr::Capacity400K;
    const bool mfm=data.size()==Geometry::MacintoshMfm::Capacity;
    std::string_view id;
    if(single&&looksLikeLisaOffice(data))id=Formats::AppleLisaRaw;
    else if(const auto mac=probeMacintosh(data))id=mfm?Formats::Mac1440:*mac==MediaFileSystems::Apple::MacintoshKind::Mfs?Formats::AppleMacMfs:Formats::AppleMacHfs;
    else if(!single&&looksLikeProDos(data))id=Formats::AppleIIProDos;
    else throw MediaError(DiagnosticCode::AppleRawUnrecognized,{std::to_string(data.size()),std::string(extension)});
    return build(data,id,MatchKind::ValidatedStructure,!mfm,single?Geometry::SingleHead:Geometry::DoubleHeads,Geometry::MacintoshGcr::BlockSize,mfm?Geometry::MacintoshMfm::SectorsPerTrack:Geometry::MacintoshGcr::MaximumSectorsPerTrack,Geometry::EightyCylinders,token);
}
}
