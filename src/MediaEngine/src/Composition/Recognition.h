#pragma once
#include "../Images/ReaderRegistry.h"
#include "../Images/ReadingService.h"
#include <RetroMediaLab/MediaEngine/DriveService.h>
namespace RetroMediaLab::MediaEngine {
namespace Scp {class Reader;}
class MediaRecognitionComposition final {
public:
    RETROMEDIALAB_MEDIAENGINE_API static MediaRecognitionComposition createDefault();
    const ReaderRegistry& registry()const noexcept{return registry_;}
    const MediaImageReadingService& readingService()const noexcept{return reading_;}
    const MediaDriveService& driveService()const noexcept{return drives_;}
    std::span<const std::shared_ptr<const ImageReader>> readers()const noexcept{return registry_.readers();}
    std::span<const std::string> supportedExtensions()const noexcept{return extensions_;}
    const std::shared_ptr<const Scp::Reader>& scpReader()const noexcept{return scp_;}
private:
    MediaRecognitionComposition(std::shared_ptr<const Scp::Reader>,std::vector<std::shared_ptr<const ImageReader>>);
    std::shared_ptr<const Scp::Reader> scp_;
    ReaderRegistry registry_;
    MediaDriveService drives_;
    MediaImageReadingService reading_;
    std::vector<std::string> extensions_;
};
}
