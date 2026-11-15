#pragma once
#include "MediaFileSystemReader.h"
namespace RetroMediaLab::MediaFileSystems {
class FamicomDiskReader final : public MediaFileSystemReader {
public:
    RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string_view id() const noexcept override;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API bool canRead(const ImageDocument&,const VolumeDescriptor&,std::stop_token = {}) const override;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemVolume read(const ImageDocument&,const VolumeDescriptor&,std::stop_token = {}) const override;
};
}
