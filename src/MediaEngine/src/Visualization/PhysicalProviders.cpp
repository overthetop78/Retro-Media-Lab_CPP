#include "Providers.h"
#include "../Models/Sectors/Representation.h"
#include "../Models/Flux/Representation.h"
#include "../Formats/HardDisk/MfmEmu/Representation.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <array>
namespace RetroMediaLab::MediaEngine::Visualization {
namespace {
class Sector final:public Provider {
 inline static constexpr std::array Kinds{RepresentationKind::Sectors};
public:
 std::span<const RepresentationKind> kinds()const noexcept override{return Kinds;}
 bool canProvide(const MediaDocument& document)const override{return dynamic_cast<const SectorRepresentation*>(&document.representation())!=nullptr;}
 VisualizationDescriptor describe(const MediaDocument& document,std::stop_token token)const override{
  const auto& image=representation<SectorRepresentation>(document).image();const auto& geometry=image.geometry();VisualizationDescriptorFields result{RepresentationKind::Sectors,{},VisualizationProgressUnit::Track,VisualizationDirection::Ascending,{},{}};
  for(std::uint32_t head=0;head<geometry.heads;++head)result.surfaces.push_back(std::int32_t(head));
  for(std::uint32_t cylinder=0;cylinder<geometry.cylinders;++cylinder){checkCancellation(token);for(const auto surface:result.surfaces)result.elements.emplace_back(VisualizationElementFields{cylinder,surface});}return VisualizationDescriptor(std::move(result));
 }
};
class Flux final:public Provider {
 inline static constexpr std::array Kinds{RepresentationKind::Flux};
public:
 std::span<const RepresentationKind> kinds()const noexcept override{return Kinds;}
 bool canProvide(const MediaDocument& document)const override{return dynamic_cast<const FluxRepresentation*>(&document.representation())||dynamic_cast<const MfmEmu::Representation*>(&document.representation());}
 std::shared_ptr<const MediaDocument> prepare(std::shared_ptr<const MediaDocument> document,std::stop_token token)const override{checkCancellation(token);const auto mfm=dynamic_cast<const MfmEmu::Representation*>(&document->representation());if(!mfm)return document;auto logical=mfm->createLogicalView(document);checkCancellation(token);return logical?logical:document;}
 VisualizationDescriptor describe(const MediaDocument& document,std::stop_token token)const override{
  VisualizationDescriptorFields result{RepresentationKind::Flux,{},VisualizationProgressUnit::Track,VisualizationDirection::SourceDefined,{},{}};
  if(const auto mfm=dynamic_cast<const MfmEmu::Representation*>(&document.representation())){
   for(std::int32_t head=0;head<mfm->heads();++head)result.surfaces.push_back(head);
   for(std::int32_t cylinder=0;cylinder<mfm->cylinders();++cylinder){checkCancellation(token);for(const auto surface:result.surfaces)result.elements.emplace_back(VisualizationElementFields{cylinder,surface});}
  }else{
   const auto& flux=representation<FluxRepresentation>(document);result.surfaces.assign(flux.surfaces().begin(),flux.surfaces().end());for(const auto& track:flux.tracks()){checkCancellation(token);result.elements.emplace_back(VisualizationElementFields{track.cylinder(),track.head()});}
  }return VisualizationDescriptor(std::move(result));
 }
};
}
std::shared_ptr<const Provider> fluxProvider(){return std::make_shared<Flux>();}
std::shared_ptr<const Provider> sectorProvider(){return std::make_shared<Sector>();}
}
