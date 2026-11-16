#include <RetroMediaLab/MediaEngine/MediaVisualization.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include "Constants.h"
#include <set>
namespace RetroMediaLab::MediaEngine {
namespace {
void require(bool condition,std::string_view name){if(!condition)throw MediaError(DiagnosticCode::VisualizationValue,{std::string(name)});}
std::set<std::int32_t> unique(std::span<const std::int32_t> values,std::string_view name){std::set<std::int32_t> result;for(const auto value:values){require(value>=0,name);require(result.insert(value).second,name);}return result;}
}
VisualizationElement::VisualizationElement(VisualizationElementFields fields):fields_(std::move(fields)){
 using namespace Visualization::Constants;const auto& f=fields_;
 require(f.position>=0,Position);require(!f.surface||*f.surface>=0,Surface);require(!f.length||*f.length>0,Length);require(!f.sessionNumber||*f.sessionNumber>0,Session);require(!f.trackNumber||*f.trackNumber>=0,Track);require(!f.layerNumber||*f.layerNumber>=0,Layer);require(!f.faceNumber||*f.faceNumber>=0,Face);require(!f.channelNumber||*f.channelNumber>=0,Channel);
}
VisualizationDescriptor::VisualizationDescriptor(VisualizationDescriptorFields fields):fields_(std::move(fields)){
 using namespace Visualization::Constants;const auto surfaces=unique(fields_.surfaces,Surfaces);const auto layers=fields_.layers?unique(*fields_.layers,Layers):std::set<std::int32_t>{};
 for(const auto& element:fields_.elements){const auto& f=element.fields();if(f.surface&&!surfaces.contains(*f.surface))throw MediaError(DiagnosticCode::VisualizationReference,{std::string(Surface)});if(f.layerNumber&&!layers.contains(*f.layerNumber))throw MediaError(DiagnosticCode::VisualizationReference,{std::string(Layer)});}
}
}
