#pragma once
#include "MediaFileSystemReader.h"
#include <memory>
#include <span>
namespace RetroMediaLab::MediaFileSystems {
class SectorReader : public MediaFileSystemReader {
public:
    virtual ~SectorReader() = default;
    virtual std::string_view id() const noexcept = 0;
    virtual std::span<const std::string_view> catalogFormatIds() const noexcept = 0;
    virtual bool canRead(const SectorImage&,std::stop_token={}) const = 0;
    virtual FileSystemVolume read(const SectorImage&,std::stop_token={}) const = 0;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API bool canRead(const ImageDocument&,const VolumeDescriptor&,std::stop_token={}) const override;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemVolume read(const ImageDocument&,const VolumeDescriptor&,std::stop_token={}) const override;
};
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::vector<std::shared_ptr<const SectorReader>> createDefaultSectorReaders();
}
