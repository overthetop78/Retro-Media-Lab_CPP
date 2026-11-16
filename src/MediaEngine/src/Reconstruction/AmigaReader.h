#pragma once
#include "../Formats/Scp/Scp.h"
#include "../Decoding/Decoder.h"
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include <span>
namespace RetroMediaLab::MediaEngine::AmigaReconstruction {
RETROMEDIALAB_MEDIAENGINE_API std::int32_t inferSectorsPerTrack(std::span<const MediaFileSystems::SectorAddress>);
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> read(const Scp::Image&,const Decoding::DecodeOperation&,std::stop_token={});
}
