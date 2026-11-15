#pragma once
#include "Writers.h"
namespace RetroMediaLab::MediaEngine {
RETROMEDIALAB_MEDIAENGINE_API std::string normalizeExtension(std::string_view);
class WriterRegistry final {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit WriterRegistry(std::vector<std::shared_ptr<const ImageWriter>>);
    std::span<const std::shared_ptr<const ImageWriter>> writers()const noexcept{return writers_;}
    RETROMEDIALAB_MEDIAENGINE_API std::vector<std::shared_ptr<const ImageWriter>> findTargetCandidates(std::string_view targetFormat,std::string_view targetExtension)const;
    RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageWriter> resolve(const MediaDocument&,std::string_view targetFormat,std::string_view targetExtension)const;
    RETROMEDIALAB_MEDIAENGINE_API bool canWrite(const MediaDocument&,std::string_view targetFormat,std::string_view targetExtension)const;
private:std::vector<std::shared_ptr<const ImageWriter>> writers_;
};
}
