#pragma once
#include "PhysicalMediaTypes.h"
#include "TapeSignal.h"
#include "DiskExplorationTypes.h"
#include <filesystem>
namespace RetroMediaLab::MediaEngine {
class MediaDocument;
RETROMEDIALAB_MEDIAENGINE_API PhysicalDataUnit createPhysicalTapeUnit(const MediaTapeSignal&,std::int64_t firstSample=0);
RETROMEDIALAB_MEDIAENGINE_API MediaTapeSignal readPhysicalTapeSignal(std::span<const PhysicalDataUnit>);
RETROMEDIALAB_MEDIAENGINE_API MediaTapeSignal createAcquiredTapeSignal(const PhysicalAcquisitionResult&);
RETROMEDIALAB_MEDIAENGINE_API PhysicalWritePlan createTapeWritePlan(const MediaTapeSignal&);
RETROMEDIALAB_MEDIAENGINE_API std::future<PhysicalWritePlan> createTapeWritePlanAsync(std::filesystem::path,PhysicalMetadata parameters={},std::stop_token={});
struct PreparedFloppyAcquisition {std::shared_ptr<const MediaDocument> document;std::shared_ptr<const DiskCaptureSnapshot> capture;};
RETROMEDIALAB_MEDIAENGINE_API DiskTrackInfo createAcquiredFloppyTrack(const PhysicalDataUnit&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const DiskCaptureSnapshot> createAcquiredFloppyCapture(const PhysicalAcquisitionResult&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API PreparedFloppyAcquisition prepareFloppyAcquisition(const std::filesystem::path&,const PhysicalAcquisitionResult&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API PhysicalWritePlan createFloppyWritePlan(const DiskCaptureSnapshot&,int revolution=0,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::future<PhysicalWritePlan> createFloppyWritePlanAsync(std::filesystem::path,std::optional<std::string> format={},int revolution=0,std::stop_token={});
}
