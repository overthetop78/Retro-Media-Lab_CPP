#pragma once
#include "../Decoding/Sequential/Registry.h"
#include <RetroMediaLab/MediaFileSystems/MediaFileSystemReader.h>
namespace RetroMediaLab::MediaEngine {
class SequentialContentAdapter final:public MediaFileSystems::MediaFileSystemReader {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit SequentialContentAdapter(SequentialDecoderRegistry);
    RETROMEDIALAB_MEDIAENGINE_API std::string_view id()const noexcept override;
    RETROMEDIALAB_MEDIAENGINE_API bool canRead(const MediaFileSystems::ImageDocument&,const MediaFileSystems::VolumeDescriptor&,std::stop_token={})const override;
    RETROMEDIALAB_MEDIAENGINE_API MediaFileSystems::FileSystemVolume read(const MediaFileSystems::ImageDocument&,const MediaFileSystems::VolumeDescriptor&,std::stop_token={})const override;
private:SequentialDecoderRegistry decoders_;
};
}
