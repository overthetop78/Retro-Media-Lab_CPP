#pragma once
#include "Jfd.h"
#include "../../Images/Readers.h"
#include "../../Images/Writers.h"
namespace RetroMediaLab::MediaEngine::Jfd {
using FileLoader=std::function<std::vector<std::uint8_t>(const std::string&,std::stop_token)>;
class Reader final:public ImageReader {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit Reader(FileLoader = {});
    RETROMEDIALAB_MEDIAENGINE_API const ReaderCapabilities& capabilities()const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<bool> canReadAsync(RecognitionContext,std::stop_token={})const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::shared_ptr<const MediaDocument>> readAsync(RecognitionContext,std::stop_token={})const override;
private:FileLoader loader_;
};
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaDocument> readBytes(std::shared_ptr<const MediaSource>,std::span<const std::uint8_t>,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageWriter> writerAdapter();
std::map<std::string,std::string> metadata(const Image&,bool gzip);
std::optional<Image> restore(const std::map<std::string,std::string>&,std::stop_token);
}
