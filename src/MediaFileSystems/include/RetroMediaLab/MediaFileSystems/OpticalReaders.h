#pragma once
#include "MediaFileSystemReader.h"
namespace RetroMediaLab::MediaFileSystems {
class Iso9660Reader final : public MediaFileSystemReader {
public:
    RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string_view id() const noexcept override;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API bool canRead(const ImageDocument&,const VolumeDescriptor&,std::stop_token = {}) const override;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemVolume read(const ImageDocument&,const VolumeDescriptor&,std::stop_token = {}) const override;
};
class JolietReader final : public MediaFileSystemReader {
public:
    RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string_view id() const noexcept override;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API bool canRead(const ImageDocument&,const VolumeDescriptor&,std::stop_token = {}) const override;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemVolume read(const ImageDocument&,const VolumeDescriptor&,std::stop_token = {}) const override;
};
class RockRidgeReader final : public MediaFileSystemReader {
public:
    RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string_view id() const noexcept override;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API bool canRead(const ImageDocument&,const VolumeDescriptor&,std::stop_token = {}) const override;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemVolume read(const ImageDocument&,const VolumeDescriptor&,std::stop_token = {}) const override;
};
class UdfReader final : public MediaFileSystemReader {
public:
    RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string_view id() const noexcept override;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API bool canRead(const ImageDocument&,const VolumeDescriptor&,std::stop_token = {}) const override;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemVolume read(const ImageDocument&,const VolumeDescriptor&,std::stop_token = {}) const override;
};
class XdvdfsReader final : public MediaFileSystemReader {
public:
    RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string_view id() const noexcept override;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API bool canRead(const ImageDocument&,const VolumeDescriptor&,std::stop_token = {}) const override;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemVolume read(const ImageDocument&,const VolumeDescriptor&,std::stop_token = {}) const override;
};
}
