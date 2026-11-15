#pragma once
#include "MediaDocument.h"
#include <filesystem>
namespace RetroMediaLab::MediaEngine {
struct WriterCapabilities {std::string id;std::vector<std::string> formatIds,producedFileExtensions;std::vector<RepresentationKind> representationKinds;bool producesMultipleFiles=false;};
class RETROMEDIALAB_MEDIAENGINE_NOVTABLE ImageWriter {
public:
    virtual ~ImageWriter()=default;
    virtual const WriterCapabilities& capabilities()const=0;
    RETROMEDIALAB_MEDIAENGINE_API virtual bool supportsTarget(std::string_view targetFormat,std::string_view targetExtension)const;
    virtual bool canWrite(const MediaDocument&,std::string_view targetFormat,std::string_view targetExtension)const=0;
    virtual std::future<std::vector<std::filesystem::path>> writeAsync(std::shared_ptr<const MediaDocument>,std::filesystem::path output,std::string targetFormat,std::stop_token={})const=0;
};
}
