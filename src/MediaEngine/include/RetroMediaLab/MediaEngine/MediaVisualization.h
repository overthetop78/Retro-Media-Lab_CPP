#pragma once
#include "Export.h"
#include "RepresentationInfo.h"
#include <future>
#include <memory>
#include <span>
#include <stop_token>
#include <vector>
namespace RetroMediaLab::MediaEngine {
class MediaDocument;
enum class VisualizationDirection {Ascending,Descending,SourceDefined};
enum class VisualizationProgressUnit {Track,Sector,BlockRange,OpticalTrack,Segment};
struct VisualizationElementFields {
 std::int64_t position=0;std::optional<std::int32_t> surface;std::optional<std::int64_t> length;
 std::optional<std::int32_t> sessionNumber,trackNumber,layerNumber,faceNumber,channelNumber;
};
class VisualizationElement final {
public:
 RETROMEDIALAB_MEDIAENGINE_API explicit VisualizationElement(VisualizationElementFields);
 const VisualizationElementFields& fields()const noexcept{return fields_;}
private:VisualizationElementFields fields_;
};
struct VisualizationDescriptorFields {
 RepresentationKind representationKind;std::vector<std::int32_t> surfaces;VisualizationProgressUnit progressUnit;VisualizationDirection direction;
 std::vector<VisualizationElement> elements;std::optional<std::vector<std::int32_t>> layers;
};
class VisualizationDescriptor final {
public:
 RETROMEDIALAB_MEDIAENGINE_API explicit VisualizationDescriptor(VisualizationDescriptorFields);
 const VisualizationDescriptorFields& fields()const noexcept{return fields_;}
private:VisualizationDescriptorFields fields_;
};
RETROMEDIALAB_MEDIAENGINE_API VisualizationDescriptor describeMediaVisualization(std::shared_ptr<const MediaDocument>,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaDocument> prepareMediaVisualization(std::shared_ptr<const MediaDocument>,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::future<bool> readBlockVisualizationElementAsync(std::shared_ptr<const MediaDocument>,VisualizationElement,std::stop_token={});
}
