#pragma once
#include "Images.h"
#include "../../Images/Readers.h"
#include "../../Images/Writers.h"
namespace RetroMediaLab::MediaEngine::CommodoreDisks {
class ImageReaderAdapter final:public ImageReader {
public:
    explicit ImageReaderAdapter(Kind kind):kind_(kind){}
    RETROMEDIALAB_MEDIAENGINE_API const ReaderCapabilities& capabilities()const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<bool> canReadAsync(RecognitionContext,std::stop_token={})const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::shared_ptr<const MediaDocument>> readAsync(RecognitionContext,std::stop_token={})const override;
private:Kind kind_;
};
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageWriter> dosWriterAdapter();
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageWriter> d81WriterAdapter();
}
