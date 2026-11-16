#pragma once
#include "Iso/Collector.h"
#include "../Formats/Scp/Scp.h"
namespace RetroMediaLab::MediaEngine::Rx02Reconstruction {
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> read(const Scp::Image&,const Decoding::DecodeOperation&,const IsoReconstruction::CollectionProgressCallback& = {},std::stop_token={});
}
