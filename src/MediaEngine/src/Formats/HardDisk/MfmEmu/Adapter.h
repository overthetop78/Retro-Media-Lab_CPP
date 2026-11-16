#pragma once
#include "../../../Images/Readers.h"
#include "../../../Images/Writers.h"
#include "../../../Models/Blocks/BlockImage.h"
namespace RetroMediaLab::MediaEngine::MfmEmu {
class Reader final:public ImageReader {
public:
    RETROMEDIALAB_MEDIAENGINE_API const ReaderCapabilities& capabilities()const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<bool> canReadAsync(RecognitionContext,std::stop_token={})const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::shared_ptr<const MediaDocument>> readAsync(RecognitionContext,std::stop_token={})const override;
};
class Writer final:public ImageWriter {
public:
    RETROMEDIALAB_MEDIAENGINE_API const WriterCapabilities& capabilities()const override;
    RETROMEDIALAB_MEDIAENGINE_API bool canWrite(const MediaDocument&,std::string_view,std::string_view)const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::vector<std::filesystem::path>> writeAsync(std::shared_ptr<const MediaDocument>,std::filesystem::path,std::string,std::stop_token={})const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::vector<std::filesystem::path>> createAsync(std::shared_ptr<const MediaDocument>,std::filesystem::path,std::stop_token={},bool preallocate=false)const;
};
RETROMEDIALAB_MEDIAENGINE_API std::future<void> createAsync(std::filesystem::path,HardDiskGeometry,std::optional<std::string> fileSystemId={},std::stop_token={},bool preallocate=false);
}
