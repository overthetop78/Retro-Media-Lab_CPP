#pragma once
#include "Records.h"
#include "../../Images/Readers.h"
namespace RetroMediaLab::MediaEngine::TeleDisk {
RETROMEDIALAB_MEDIAENGINE_API Image readDetailed(MediaFileSystems::Bytes,std::stop_token={});
class Reader final:public ImageReader {
public:
    RETROMEDIALAB_MEDIAENGINE_API const ReaderCapabilities& capabilities()const override;
    RETROMEDIALAB_MEDIAENGINE_API bool supportsFormatId(std::string_view)const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<bool> canReadAsync(RecognitionContext,std::stop_token={})const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::shared_ptr<const MediaDocument>> readAsync(RecognitionContext,std::stop_token={})const override;
};
}
