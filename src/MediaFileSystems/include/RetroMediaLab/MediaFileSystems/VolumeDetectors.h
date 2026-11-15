#pragma once
#include "MediaDocument.h"
#include <future>
namespace RetroMediaLab::MediaFileSystems {
class VolumeDetector {
public:
    virtual ~VolumeDetector() = default;
    virtual std::future<std::optional<VolumeDetectionResult>> detectAsync(std::shared_ptr<const ImageDocument>, std::stop_token = {}) const = 0;
};
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::vector<std::shared_ptr<const VolumeDetector>> createDefaultVolumeDetectors();
class VolumeDetectorRegistry {
public:
    RETROMEDIALAB_MEDIAFILESYSTEMS_API explicit VolumeDetectorRegistry(std::vector<std::shared_ptr<const VolumeDetector>>);
    std::span<const std::shared_ptr<const VolumeDetector>> detectors() const noexcept { return detectors_; }
    RETROMEDIALAB_MEDIAFILESYSTEMS_API std::future<VolumeDetectionResult> detectAsync(std::shared_ptr<const ImageDocument>, std::stop_token = {}) const;
private:
    std::vector<std::shared_ptr<const VolumeDetector>> detectors_;
};
}
