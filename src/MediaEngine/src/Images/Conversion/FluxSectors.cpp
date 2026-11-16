#include "FluxSectors.h"
#include "Constants.h"
#include "../DocumentFactory.h"
#include "../../Models/Flux/Representation.h"
#include "../../Models/Sectors/Representation.h"
#include "../../Formats/Scp/ProtectedImage.h"
#include "../../Formats/Scp/Adapter.h"
#include "../../Formats/Hfe/Constants.h"
#include "../../Formats/HxcStream/Constants.h"
#include "../../Formats/FormatIds.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaEngine/DiagnosticKeys.h>
#include <algorithm>
#include <array>
#include <set>
namespace RetroMediaLab::MediaEngine {
namespace {
constexpr std::array FluxKinds{RepresentationKind::Flux},SectorKinds{RepresentationKind::Sectors};
std::string coordinates(const FluxRepresentation& flux,bool heads){std::set<std::int32_t> values;for(const auto& track:flux.tracks())values.insert(heads?track.head():track.cylinder());std::string result;for(const auto value:values){if(!result.empty())result+=RepresentationConversionConstants::CoordinateSeparator;result+=std::to_string(value);}return result;}
}
FluxToSectorConverter::FluxToSectorConverter(ScpReconstruction::Reader reader,Definitions::Identifiers formats):reader_(std::move(reader)),formats_(std::move(formats)){}
std::string_view FluxToSectorConverter::id()const noexcept{return RepresentationConversionConstants::FluxSectorsId;}
std::span<const RepresentationKind> FluxToSectorConverter::sourceKinds()const noexcept{return FluxKinds;}
std::span<const RepresentationKind> FluxToSectorConverter::targetKinds()const noexcept{return SectorKinds;}
bool FluxToSectorConverter::canConvert(const MediaDocument& source,std::string_view target,RepresentationKind kind)const{return kind==RepresentationKind::Sectors&&dynamic_cast<const FluxRepresentation*>(&source.representation())&&formats_.contains(target);}
RepresentationConversionResult FluxToSectorConverter::convert(std::shared_ptr<const MediaDocument> source,std::string target,RepresentationKind kind,const ConversionOptions& options,const RepresentationProgress& progress,std::stop_token token)const{
 (void)options;checkCancellation(token);if(!source||!canConvert(*source,target,kind))throw MediaError(DiagnosticCode::RepresentationFluxSectors,{target});
 const auto& flux=dynamic_cast<const FluxRepresentation&>(source->representation());
 if(progress)progress({{std::string(DiagnosticKeys::RepresentationConvertingTracks),{coordinates(flux,false),coordinates(flux,true)},{}},RepresentationConversionConstants::StartPercent,source->mediaKind()});
 const auto same=Primitives::asciiIdentifierEqualsIgnoreCase;
 const std::optional<std::string> hint=same(target,Formats::CpcEmuDsk)||same(target,Formats::UcsdIbmMfm)?std::nullopt:std::optional<std::string>(target);
 ScpReconstruction::Progress relay;
 if(progress)relay=[progress,media=source->mediaKind()](const auto& item){if(item.kind==IsoReconstruction::CollectionProgressKind::TrackDecoded)progress({{std::string(DiagnosticKeys::RepresentationConvertingTrack),{std::to_string(item.cylinder),std::to_string(item.head)},{}},RepresentationConversionConstants::DecodeStartPercent+RepresentationConversionConstants::DecodePercentSpan*item.completed/std::max(1,item.total),media});};
 auto image=reader_.read(flux.image(),hint,relay,token);
 return {floppySectorDocument(source->mediaSource(),std::move(image)),{},{{std::string(DiagnosticKeys::RepresentationFluxLoss),{},{}}}};
}
SectorToFluxConverter::SectorToFluxConverter(Scp::SectorConverter converter):converter_(std::move(converter)){}
std::string_view SectorToFluxConverter::id()const noexcept{return RepresentationConversionConstants::SectorsFluxId;}
std::span<const RepresentationKind> SectorToFluxConverter::sourceKinds()const noexcept{return SectorKinds;}
std::span<const RepresentationKind> SectorToFluxConverter::targetKinds()const noexcept{return FluxKinds;}
bool SectorToFluxConverter::canConvert(const MediaDocument& source,std::string_view target,RepresentationKind kind)const{
 const auto sectors=dynamic_cast<const SectorRepresentation*>(&source.representation());const auto same=Primitives::asciiIdentifierEqualsIgnoreCase;
 return kind==RepresentationKind::Flux&&sectors&&(same(target,Scp::Constants::FormatId)||same(target,Hfe::Constants::FormatId)||same(target,HxcStream::Constants::FormatId))&&converter_.canCreate(sectors->image());
}
RepresentationConversionResult SectorToFluxConverter::convert(std::shared_ptr<const MediaDocument> source,std::string target,RepresentationKind kind,const ConversionOptions& options,const RepresentationProgress& progress,std::stop_token token)const{
 (void)options;(void)progress;checkCancellation(token);if(!source||!canConvert(*source,target,kind))throw MediaError(DiagnosticCode::RepresentationSectorsFlux,{source?source->formatId():std::string{},target});
 const auto image=converter_.create(dynamic_cast<const SectorRepresentation&>(source->representation()).image(),token);
 return {floppyFluxDocument(source->mediaSource(),std::string(Scp::Constants::FormatId),std::make_shared<const Flux::ProtectedTrackImage>(Scp::protectedImage(image,token)),Scp::metadata(image)),{},{{std::string(DiagnosticKeys::RepresentationSyntheticFluxLoss),{},{}}}};
}
}
