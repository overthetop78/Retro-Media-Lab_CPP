#pragma once
#include "../../../Images/Writers.h"
namespace RetroMediaLab::MediaEngine::HardDiskRaw {
class Writer final:public ImageWriter {
public:
    RETROMEDIALAB_MEDIAENGINE_API const WriterCapabilities& capabilities()const override;
    RETROMEDIALAB_MEDIAENGINE_API bool canWrite(const MediaDocument&,std::string_view targetFormat,std::string_view targetExtension)const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::vector<std::filesystem::path>> writeAsync(std::shared_ptr<const MediaDocument>,std::filesystem::path,std::string targetFormat,std::stop_token={})const override;
};
}
