#pragma once
#include "CoherentImages.h"
#include "../Scp/Scp.h"
namespace RetroMediaLab::MediaEngine::Commodore900 {
using TrackProgress=std::function<void(const Scp::Track&,std::size_t completed,std::size_t total)>;
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> readFlux(const Scp::Image&,TrackProgress={},std::stop_token={});
}
