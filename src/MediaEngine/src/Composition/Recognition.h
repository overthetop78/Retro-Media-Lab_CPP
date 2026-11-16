#pragma once
#include "../Images/ReaderRegistry.h"
namespace RetroMediaLab::MediaEngine {
namespace Scp {class Reader;}
class MediaRecognitionComposition final {
public:
    RETROMEDIALAB_MEDIAENGINE_API static MediaRecognitionComposition createDefault();
    const ReaderRegistry& registry()const noexcept{return registry_;}
    std::span<const std::shared_ptr<const ImageReader>> readers()const noexcept{return registry_.readers();}
    std::span<const std::string> supportedExtensions()const noexcept{return extensions_;}
    const std::shared_ptr<const Scp::Reader>& scpReader()const noexcept{return scp_;}
private:
    MediaRecognitionComposition(std::shared_ptr<const Scp::Reader>,std::vector<std::shared_ptr<const ImageReader>>);
    std::shared_ptr<const Scp::Reader> scp_;
    ReaderRegistry registry_;
    std::vector<std::string> extensions_;
};
}
