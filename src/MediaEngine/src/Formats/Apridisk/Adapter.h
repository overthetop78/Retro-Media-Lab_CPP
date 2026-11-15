#pragma once
#include "../../Images/Readers.h"
#include "../../Images/Writers.h"
namespace RetroMediaLab::MediaEngine::Apridisk {
class Reader final:public ImageReader {
public:
    RETROMEDIALAB_MEDIAENGINE_API const ReaderCapabilities& capabilities()const override;
    RETROMEDIALAB_MEDIAENGINE_API bool supportsFormatId(std::string_view)const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<bool> canReadAsync(RecognitionContext,std::stop_token={})const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::shared_ptr<const MediaDocument>> readAsync(RecognitionContext,std::stop_token={})const override;
};
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageWriter> writerAdapter();
}
