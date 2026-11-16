#pragma once
#include "I86f.h"
#include "../../Decoding/Decoder.h"
namespace RetroMediaLab::MediaEngine::I86f {
RETROMEDIALAB_MEDIAENGINE_API std::optional<Flux::Revolution> sectorRevolution(Flux::BitView,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> reconstructSectors(const Image&,const Decoding::DecodeOperation&,std::stop_token={});
}
