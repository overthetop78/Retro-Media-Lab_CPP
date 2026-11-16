#pragma once
#include "../../Models/Flux/ProtectedTrack.h"
#include "../../Images/Readers.h"
namespace RetroMediaLab::MediaEngine::I86f {
struct Track {std::int32_t logicalIndex=0;std::uint16_t flags=0;std::int32_t bitCount=0;Flux::Bits bits;};
struct Image {std::uint16_t flags=0;std::vector<Track> tracks;};
RETROMEDIALAB_MEDIAENGINE_API Image read(std::span<const std::uint8_t>,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API Flux::ProtectedTrackImage protectedImage(const Image&,std::stop_token={});
class Reader final:public ImageReader {
public:
    RETROMEDIALAB_MEDIAENGINE_API const ReaderCapabilities& capabilities()const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<bool> canReadAsync(RecognitionContext,std::stop_token={})const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::shared_ptr<const MediaDocument>> readAsync(RecognitionContext,std::stop_token={})const override;
};
}
