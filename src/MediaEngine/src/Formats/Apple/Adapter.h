#pragma once
#include "../../Images/Readers.h"
namespace RetroMediaLab::MediaEngine::Apple {
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> readContainer(MediaFileSystems::Bytes,std::string_view extension,std::optional<std::string_view> requested={},std::stop_token={});
class Reader final:public ImageReader {
public:
    RETROMEDIALAB_MEDIAENGINE_API const ReaderCapabilities& capabilities()const override;
    RETROMEDIALAB_MEDIAENGINE_API bool supportsFormatId(std::string_view)const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<bool> canReadAsync(RecognitionContext,std::stop_token={})const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::shared_ptr<const MediaDocument>> readAsync(RecognitionContext,std::stop_token={})const override;
};
}
