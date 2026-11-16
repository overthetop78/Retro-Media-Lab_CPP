#pragma once
#include "../../../Models/Flux/Conversion.h"
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
namespace RetroMediaLab::MediaEngine::Adaptec {
struct Field {std::vector<std::uint8_t> id;MediaFileSystems::SectorBlock sector;};
struct WriteWindow {std::uint32_t startNanoseconds,durationNanoseconds;std::vector<std::uint32_t> transitions;};
RETROMEDIALAB_MEDIAENGINE_API std::uint32_t errorCheck(std::span<const std::uint8_t>);
RETROMEDIALAB_MEDIAENGINE_API Flux::NanosecondRevolution encode(std::span<const MediaFileSystems::SectorBlock>,bool sequential,std::uint32_t period,std::int32_t sizeHint=0,std::span<const std::uint8_t> parameters={},std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::vector<Field> decode(const Flux::NanosecondRevolution&,std::int32_t cylinder,std::int32_t head,std::int32_t sizeHint=0,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API WriteWindow createSectorWrite(const Flux::NanosecondRevolution&,std::int32_t cylinder,std::int32_t head,std::int64_t logicalAddress,std::span<const std::uint8_t> payload,std::stop_token={});
}
