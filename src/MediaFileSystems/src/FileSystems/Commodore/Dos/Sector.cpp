#include "Internal.h"
#include <limits>
namespace RetroMediaLab::MediaFileSystems::CommodoreDos::Internal {
std::optional<std::int32_t> logicalBlock(const SectorImage& image,std::int32_t track,std::int32_t sector) {
    try {
        if(image.geometry().formatId==Commodore::Constants::Format1581)return Commodore::block1581(track,sector);
        if(image.geometry().cylinders>std::uint32_t(std::numeric_limits<std::int32_t>::max()))return {};
        const auto tracks=std::int32_t(image.geometry().cylinders);const auto side=track>tracks?1:0;const auto sideTrack=side?std::int64_t(track)-tracks:track;
        if(sideTrack<std::numeric_limits<std::int32_t>::min()||sideTrack>std::numeric_limits<std::int32_t>::max())return {};
        return image.geometry().heads==Commodore::Constants::TwoSides?Commodore::block1571(std::int32_t(sideTrack),sector,tracks,side):Commodore::sideBlock1541(std::int32_t(sideTrack),sector,tracks);
    }catch(const MediaError& error) {
        switch(error.diagnostic().code){case DiagnosticCode::CommodoreTrack:case DiagnosticCode::CommodoreTrackCount:case DiagnosticCode::CommodoreSide:case DiagnosticCode::CommodoreSector:case DiagnosticCode::CommodoreBlock:return {};default:throw;}
    }
}
SectorRead sector(const Image& image,std::int32_t track,std::int32_t number) {
    checkCancellation(image.token);const auto logical=logicalBlock(image.source,track,number);
    if(!logical)return {SectorStatus::InvalidCoordinate,{}};
    auto block=image.source.block(std::uint64_t(*logical),image.token);
    if(!block)return {SectorStatus::Missing,{}};
    const auto status=block->data.size()==Constants::SectorSize?SectorStatus::Success:SectorStatus::Truncated;
    return {status,std::move(block)};
}
std::optional<Recognition> recognize(const Image& image) {
    checkCancellation(image.token);const auto definition=layout(image.source.geometry().formatId);
    if(!definition||image.source.geometry().blockSize!=Constants::SectorSize)return {};
    const auto header=sector(image,definition->headerTrack,definition->headerSector);
    if(header.status!=SectorStatus::Success||header.block->data[Constants::TypeOffset]!=definition->signature)return {};
    const auto& data=header.block->data;const auto track=data[0]?data[0]:definition->directoryTrack;const auto number=data[0]?data[1]:definition->directorySector;
    if(!plausible(image,track,number))return {};
    return Recognition{*definition,data,track,number};
}
}
