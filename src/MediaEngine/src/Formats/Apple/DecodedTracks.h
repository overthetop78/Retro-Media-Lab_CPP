#pragma once
#include "../../Decoding/Models.h"
#include "../../Models/Flux/Flux.h"
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
namespace RetroMediaLab::MediaEngine::Apple {
struct DecodedTrack {std::int32_t track;std::vector<Decoding::Sector> sectors;};
struct TrackCandidates {std::vector<Decoding::Sector> standard,rwts18;std::int32_t standardScore=0,rwts18Score=0;};
RETROMEDIALAB_MEDIAENGINE_API TrackCandidates decodeTrack(Flux::BitView,std::int32_t track,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> buildStandard(std::span<const DecodedTrack>);
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> buildRwts18(std::span<const DecodedTrack>);
}
