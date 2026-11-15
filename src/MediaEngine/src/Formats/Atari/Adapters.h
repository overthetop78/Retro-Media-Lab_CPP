#pragma once
#include "../../Images/Readers.h"
#include "../../Images/Writers.h"
namespace RetroMediaLab::MediaEngine::Atari {
enum class Kind {St,Msa,Atr,Atx};
class Reader final:public ImageReader {
public:
    explicit Reader(Kind kind):kind_(kind){}
    RETROMEDIALAB_MEDIAENGINE_API const ReaderCapabilities& capabilities()const override;
    RETROMEDIALAB_MEDIAENGINE_API bool supportsFormatId(std::string_view)const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<bool> canReadAsync(RecognitionContext,std::stop_token={})const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::shared_ptr<const MediaDocument>> readAsync(RecognitionContext,std::stop_token={})const override;
private:Kind kind_;
};
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageWriter> atrWriterAdapter();
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageWriter> stWriterAdapter();
}
