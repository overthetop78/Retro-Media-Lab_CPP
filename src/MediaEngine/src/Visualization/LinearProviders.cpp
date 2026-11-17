#include "Providers.h"
#include "Constants.h"
#include "../Models/Optical/Image.h"
#include "../Models/Sequential/Image.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <array>
#include <algorithm>
#include <limits>
#include <map>
#include <tuple>
namespace RetroMediaLab::MediaEngine::Visualization {
namespace {
class Optical final:public Provider {
 inline static constexpr std::array Kinds{RepresentationKind::OpticalTracks};
public:
 std::span<const RepresentationKind> kinds()const noexcept override{return Kinds;}
 bool canProvide(const MediaDocument& document)const override{return dynamic_cast<const OpticalImage*>(&document.representation())!=nullptr;}
 VisualizationDescriptor describe(const MediaDocument& document,std::stop_token token)const override{
  const auto& optical=representation<OpticalImage>(document).options();VisualizationDescriptorFields result{RepresentationKind::OpticalTracks,{},VisualizationProgressUnit::OpticalTrack,VisualizationDirection::Ascending,{},{}};
  if(optical.faceCount)for(std::int32_t face=0;face<*optical.faceCount;++face)result.surfaces.push_back(face);
  if(optical.layerCount){result.layers.emplace();for(std::int32_t layer=0;layer<*optical.layerCount;++layer)result.layers->push_back(layer);}
  const auto surface=result.surfaces.size()==1?std::optional<std::int32_t>(0):std::nullopt;if(optical.tracks)for(const auto& track:*optical.tracks){checkCancellation(token);result.elements.emplace_back(VisualizationElementFields{track->firstSector(),surface,track->sectorCount(),track->sessionNumber(),track->trackNumber()});}return VisualizationDescriptor(std::move(result));
 }
};
using Lane=std::tuple<std::optional<int>,std::optional<int>,std::optional<int>>;
struct LaneLess {bool operator()(const Lane& a,const Lane& b)const{const auto key=[](const Lane& value){return std::tuple{std::get<Constants::LaneFace>(value).value_or(std::numeric_limits<int>::max()),std::get<Constants::LaneTrack>(value).value_or(std::numeric_limits<int>::max()),std::get<Constants::LaneChannel>(value).value_or(std::numeric_limits<int>::max())};};return key(a)<key(b);}};
class Sequential final:public Provider {
 inline static constexpr std::array Kinds{RepresentationKind::Sequential};
public:
 std::span<const RepresentationKind> kinds()const noexcept override{return Kinds;}
 bool canProvide(const MediaDocument& document)const override{return dynamic_cast<const SequentialImage*>(&document.representation())!=nullptr;}
 VisualizationDescriptor describe(const MediaDocument& document,std::stop_token token)const override{
  const auto& source=representation<SequentialImage>(document).segments();VisualizationDescriptorFields result{RepresentationKind::Sequential,{},VisualizationProgressUnit::Segment,VisualizationDirection::Ascending,{},{}};if(!source)return VisualizationDescriptor(std::move(result));
  std::map<Lane,std::int32_t> lanes;std::vector<Lane> ordered;for(const auto& segment:*source){checkCancellation(token);const auto& fields=segment.fields();Lane lane{fields.faceNumber,fields.trackNumber,fields.channelNumber};if(lanes.try_emplace(lane).second)ordered.push_back(lane);}std::stable_sort(ordered.begin(),ordered.end(),LaneLess{});for(const auto& lane:ordered){const auto number=std::int32_t(result.surfaces.size());lanes.at(lane)=number;result.surfaces.push_back(number);}
  bool forward=true,reverse=true;for(const auto& segment:*source){checkCancellation(token);const auto& fields=segment.fields();forward=forward&&fields.direction==SequentialTravelDirection::Forward;reverse=reverse&&fields.direction==SequentialTravelDirection::Reverse;result.elements.emplace_back(VisualizationElementFields{fields.start?fields.start->count():fields.position,lanes.at({fields.faceNumber,fields.trackNumber,fields.channelNumber}),fields.duration?std::optional<std::int64_t>(std::max<std::int64_t>(1,fields.duration->count())):fields.length,{},fields.trackNumber,{},fields.faceNumber,fields.channelNumber});}
  result.direction=forward?VisualizationDirection::Ascending:reverse?VisualizationDirection::Descending:VisualizationDirection::SourceDefined;return VisualizationDescriptor(std::move(result));
 }
};
}
std::shared_ptr<const Provider> opticalProvider(){return std::make_shared<Optical>();}
std::shared_ptr<const Provider> sequentialProvider(){return std::make_shared<Sequential>();}
}
