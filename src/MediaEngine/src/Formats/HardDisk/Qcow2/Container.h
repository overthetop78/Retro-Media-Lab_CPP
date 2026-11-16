#pragma once
#include "Constants.h"
#include "../../../Images/Readers.h"
#include "../../../Images/Writers.h"
#include "../../../Models/Blocks/BlockImage.h"
namespace RetroMediaLab::MediaEngine::Qcow2 {
struct Header {
    std::uint32_t version=0,clusterSize=0,l1Size=0,refcountClusters=0;
    std::int64_t virtualSize=0,l1Offset=0,refcountOffset=0;
};
Header readHeader(const RecognitionContext&,std::stop_token);
void validateCluster(std::uint64_t offset,const Header&,std::int64_t fileLength,std::string_view structure);
std::uint64_t clusterOffset(std::uint64_t entry,std::uint32_t clusterSize,std::string_view structure);
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
