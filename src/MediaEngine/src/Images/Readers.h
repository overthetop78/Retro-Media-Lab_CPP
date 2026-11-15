#pragma once
#include "RecognitionContext.h"
#include "MediaDocument.h"
namespace RetroMediaLab::MediaEngine {
struct ReaderCapabilities {
    std::vector<std::string> formatIds,extensions;
    std::vector<std::vector<std::uint8_t>> signatures;
    std::vector<std::string> associatedFileExtensions;
    std::vector<MediaKind> mediaKinds;
    std::vector<RepresentationKind> representationKinds;
};
class RETROMEDIALAB_MEDIAENGINE_NOVTABLE ImageReader {
public:
    virtual ~ImageReader()=default;
    virtual const ReaderCapabilities& capabilities()const=0;
    RETROMEDIALAB_MEDIAENGINE_API virtual bool supportsFormatId(std::string_view)const;
    virtual std::future<bool> canReadAsync(RecognitionContext,std::stop_token={})const=0;
    virtual std::future<std::shared_ptr<const MediaDocument>> readAsync(RecognitionContext,std::stop_token={})const=0;
};
}
