#pragma once
#include "Stream.h"
#include "../../Images/Readers.h"
#include "../../Images/WriterAdapters.h"
#include "../../Sources/AtomicImageSetWriter.h"
namespace RetroMediaLab::MediaEngine::HxcStream {
using Publisher=std::function<void(std::span<const ImageFileOutput>,std::stop_token)>;
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageWriter> writerAdapter(Publisher={});
class Reader final:public ImageReader {
public:
    RETROMEDIALAB_MEDIAENGINE_API const ReaderCapabilities& capabilities()const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<bool> canReadAsync(RecognitionContext,std::stop_token={})const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::shared_ptr<const MediaDocument>> readAsync(RecognitionContext,std::stop_token={})const override;
};
}
