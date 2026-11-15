#pragma once
#include "../../Images/Readers.h"
namespace RetroMediaLab::MediaEngine::Raw {
enum class ReaderKind {Ibm,Msx};
class Reader final:public ImageReader {
public:
    explicit Reader(ReaderKind kind):kind_(kind){}
    RETROMEDIALAB_MEDIAENGINE_API const ReaderCapabilities& capabilities()const override;
    RETROMEDIALAB_MEDIAENGINE_API bool supportsFormatId(std::string_view)const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<bool> canReadAsync(RecognitionContext,std::stop_token={})const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::shared_ptr<const MediaDocument>> readAsync(RecognitionContext,std::stop_token={})const override;
private:ReaderKind kind_;
};
}
