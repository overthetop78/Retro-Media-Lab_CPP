#pragma once
#include "Scp.h"
#include "../../Encoding/Track.h"
#include "../../Sources/AtomicImageWriter.h"
namespace RetroMediaLab::MediaEngine::Scp {
// The callback is synchronous and consumes each span before returning. The image stays const for the call.
RETROMEDIALAB_MEDIAENGINE_API void write(const Image&,const ImageOutput&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API void write(const Image&,const std::filesystem::path&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API Revolution syntheticRevolution(const Encoding::EncodedTrack&,std::uint8_t resolution,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::uint8_t trackNumber(std::int32_t cylinder,std::int32_t head);
}
