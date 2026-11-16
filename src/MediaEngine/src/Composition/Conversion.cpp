#include "Conversion.h"
#include "../Images/Conversion/FluxSectors.h"
#include "../Images/Conversion/Constants.h"
#include "../Images/Conversion/Sequential/SignalConverter.h"
#include "../Formats/FormatIds.h"
#include <algorithm>
#include <array>
namespace RetroMediaLab::MediaEngine {
namespace {
class SignalAdapter final:public RepresentationConverter {
 TapeSignalRepresentationConverter converter_;
public:
 std::string_view id()const noexcept override{return RepresentationConversionConstants::SignalId;}
 std::span<const RepresentationKind> sourceKinds()const noexcept override{static constexpr std::array kinds{RepresentationKind::Sequential};return kinds;}
 std::span<const RepresentationKind> targetKinds()const noexcept override{return sourceKinds();}
 bool canConvert(const MediaDocument& source,std::string_view target,RepresentationKind kind)const override{return converter_.canConvert(source,target,kind);}
 RepresentationConversionResult convert(std::shared_ptr<const MediaDocument> source,std::string target,RepresentationKind kind,const ConversionOptions& options,const RepresentationProgress& progress,std::stop_token token)const override{
  (void)progress;const auto result=converter_.convert(std::move(source),std::move(target),kind,options,token);
  RepresentationMessages diagnostics;for(const auto& diagnostic:result.diagnostics())diagnostics.push_back({diagnostic,{},{}});
  return {result.targetDocument(),std::move(diagnostics),{result.losses().begin(),result.losses().end()}};
 }
};
}
RepresentationConverterRegistry createRepresentationConverters(const MediaRecognitionComposition& recognition,const ScpReconstruction::Composition& scp){
 Definitions::Identifiers formats;
 for(const auto& reader:recognition.readers()){
  const auto& capabilities=reader->capabilities();
  if(std::find(capabilities.representationKinds.begin(),capabilities.representationKinds.end(),RepresentationKind::Sectors)!=capabilities.representationKinds.end())formats.insert(capabilities.formatIds.begin(),capabilities.formatIds.end());
 }
 formats.insert(std::string(Formats::IbmScan));for(const auto& [id,definition]:Definitions::rawDefinitions()){(void)definition;formats.insert(id);}
 return RepresentationConverterRegistry({std::make_shared<FluxToSectorConverter>(scp.reader(),std::move(formats)),std::make_shared<SectorToFluxConverter>(),std::make_shared<HfeCellConverter>(),std::make_shared<SignalAdapter>()});
}
}
