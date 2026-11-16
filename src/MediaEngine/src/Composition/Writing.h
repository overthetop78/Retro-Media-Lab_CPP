#pragma once
#include "../Images/WritingService.h"
namespace RetroMediaLab::MediaEngine {
class MediaWritingComposition final {
public:
    RETROMEDIALAB_MEDIAENGINE_API static MediaWritingComposition createDefault(MediaDriveService={});
    const WriterRegistry& registry()const noexcept{return registry_;}
    const MediaImageWritingService& writingService()const noexcept{return writing_;}
    std::span<const std::shared_ptr<const ImageWriter>> writers()const noexcept{return registry_.writers();}
private:
    MediaWritingComposition(std::vector<std::shared_ptr<const ImageWriter>>,MediaDriveService);
    WriterRegistry registry_;MediaImageWritingService writing_;
};
}
