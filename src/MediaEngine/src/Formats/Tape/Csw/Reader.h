#pragma once
#include "../Common/Pcm.h"
#include "../../../Images/Readers.h"
#include "../../../Images/Writers.h"
namespace RetroMediaLab::MediaEngine::Csw {
class Reader final:public ImageReader {
public:
    RETROMEDIALAB_MEDIAENGINE_API const ReaderCapabilities& capabilities()const override;
    RETROMEDIALAB_MEDIAENGINE_API bool supportsFormatId(std::string_view)const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<bool> canReadAsync(RecognitionContext,std::stop_token={})const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::shared_ptr<const MediaDocument>> readAsync(RecognitionContext,std::stop_token={})const override;
};
class Writer final:public ImageWriter {
public:
    RETROMEDIALAB_MEDIAENGINE_API const WriterCapabilities& capabilities()const override;
    RETROMEDIALAB_MEDIAENGINE_API bool canWrite(const MediaDocument&,std::string_view,std::string_view)const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::vector<std::filesystem::path>> writeAsync(std::shared_ptr<const MediaDocument>,std::filesystem::path,std::string,std::stop_token={})const override;
};
}
