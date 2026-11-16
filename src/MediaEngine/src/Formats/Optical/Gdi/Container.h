#pragma once
#include "Constants.h"
#include "../../../Images/Readers.h"
#include "../../../Images/Writers.h"
#include "../../../Models/Optical/Image.h"
namespace RetroMediaLab::MediaEngine::Gdi {
struct TrackDeclaration {int number=0;std::int64_t lba=0;int control=0,sectorSize=0;std::string fileName;std::int64_t fileOffset=0;};
using Descriptor=std::vector<TrackDeclaration>;
Descriptor readDescriptor(const std::filesystem::path&,std::stop_token);
std::filesystem::path resolveDataPath(const std::filesystem::path&,std::string_view);
std::vector<std::filesystem::path> associatedPaths(const std::filesystem::path&,const Descriptor&);
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
};
}
