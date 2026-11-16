#include "Representation.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
RepresentationConversionResult::RepresentationConversionResult(std::shared_ptr<const MediaDocument> document,RepresentationMessages diagnostics,RepresentationMessages losses)
 :document_(std::move(document)),diagnostics_(std::move(diagnostics)),losses_(std::move(losses)){
 if(!document_)throw MediaError(DiagnosticCode::RepresentationContract);
 for(const auto& messages:{&diagnostics_,&losses_})for(const auto& message:*messages)if(Primitives::isBlankIdentifier(message.resourceKey))throw MediaError(DiagnosticCode::RepresentationContract);
}
RepresentationConverterRegistry::RepresentationConverterRegistry(std::vector<std::shared_ptr<const RepresentationConverter>> converters):converters_(std::move(converters)){
 for(std::size_t i=0;i<converters_.size();++i){
  if(!converters_[i])throw MediaError(DiagnosticCode::RepresentationConverterNull);
  for(std::size_t previous=0;previous<i;++previous)if(Primitives::ordinalIdentifierEqualsIgnoreCase(converters_[previous]->id(),converters_[i]->id()))throw MediaError(DiagnosticCode::RepresentationConverterDuplicate,{std::string(converters_[i]->id())});
 }
}
std::shared_ptr<const RepresentationConverter> RepresentationConverterRegistry::resolve(const MediaDocument& source,std::string_view target,RepresentationKind kind)const{
 if(Primitives::isBlankIdentifier(target))throw MediaError(DiagnosticCode::RepresentationContract);
 const auto sourceKind=source.representationInfo().representationKind();
 for(const auto& converter:converters_){
  const auto inputs=converter->sourceKinds(),outputs=converter->targetKinds();
  if(std::find(inputs.begin(),inputs.end(),sourceKind)!=inputs.end()&&std::find(outputs.begin(),outputs.end(),kind)!=outputs.end()&&converter->canConvert(source,target,kind))return converter;
 }
 return {};
}
}
