#pragma once
#include "FileSystemModels.h"
#include "MediaDocument.h"
namespace RetroMediaLab::MediaFileSystems {
class MediaFileSystemReader {
public:
    virtual ~MediaFileSystemReader() = default;
    virtual std::string_view id() const noexcept = 0;
    virtual bool canRead(const ImageDocument&, const VolumeDescriptor&, std::stop_token = {}) const = 0;
    virtual FileSystemVolume read(const ImageDocument&, const VolumeDescriptor&, std::stop_token = {}) const = 0;
};
}
