#pragma once
#include "../../Images/Writers.h"
#include "../../Images/Readers.h"
#include "FileCache.h"
#include "CaptureReader.h"
#include <map>
namespace RetroMediaLab::MediaEngine::Scp {
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageWriter> writerAdapter();
RETROMEDIALAB_MEDIAENGINE_API std::map<std::string,std::string> metadata(const Image&);
class Reader final:public ImageReader,public CaptureReader {
public:
    RETROMEDIALAB_MEDIAENGINE_API const ReaderCapabilities& capabilities()const override;
    RETROMEDIALAB_MEDIAENGINE_API bool supportsFormatId(std::string_view)const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<bool> canReadAsync(RecognitionContext,std::stop_token={})const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::shared_ptr<const MediaDocument>> readAsync(RecognitionContext,std::stop_token={})const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<SharedImage> readFileAsync(std::filesystem::path,std::stop_token={})const override;
    RETROMEDIALAB_MEDIAENGINE_API void remember(const std::filesystem::path&,SharedImage)const override;
private:FileCache cache_;
};
}
