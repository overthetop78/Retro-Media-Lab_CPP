#pragma once
#include "../../Images/Readers.h"
namespace RetroMediaLab::MediaEngine::Raw {
enum class SpecializedKind {Coherent,Ucsd};
class SpecializedReader final:public ImageReader {
public:
    explicit SpecializedReader(SpecializedKind kind):kind_(kind){}
    RETROMEDIALAB_MEDIAENGINE_API const ReaderCapabilities& capabilities()const override;
    RETROMEDIALAB_MEDIAENGINE_API bool supportsFormatId(std::string_view)const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<bool> canReadAsync(RecognitionContext,std::stop_token={})const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::shared_ptr<const MediaDocument>> readAsync(RecognitionContext,std::stop_token={})const override;
private:SpecializedKind kind_;
};
}
