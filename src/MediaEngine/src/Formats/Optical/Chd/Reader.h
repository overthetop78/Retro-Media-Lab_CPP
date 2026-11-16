#pragma once
#include "Constants.h"
#include "../../../Images/Readers.h"
namespace RetroMediaLab::MediaEngine::ChdOptical {
struct TrackMetadata {int number=0;OpticalTrackMode mode=OpticalTrackMode::Audio;int dataSize=0,subchannelSize=0;std::int64_t frames=0,pregap=0,postgap=0;bool storedPregap=false;};
struct Layout {std::int64_t logicalBytes=0;std::uint32_t hunkBytes=0,unitBytes=0,version=Constants::Version5;std::vector<std::uint32_t> storedHunks;std::vector<TrackMetadata> tracks;bool dvd=false,decoded=false;std::string compression=std::string(Constants::NoCompression);};
void readMetadata(const RecognitionContext&,std::uint64_t,Layout&,std::stop_token);
Layout readLayout(const RecognitionContext&,std::stop_token);
class Reader final:public ImageReader {
public:
    RETROMEDIALAB_MEDIAENGINE_API const ReaderCapabilities& capabilities()const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<bool> canReadAsync(RecognitionContext,std::stop_token={})const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::shared_ptr<const MediaDocument>> readAsync(RecognitionContext,std::stop_token={})const override;
};
}
