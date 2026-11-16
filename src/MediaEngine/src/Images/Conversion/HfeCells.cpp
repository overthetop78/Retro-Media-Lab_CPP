#include "FluxSectors.h"
#include "Constants.h"
#include "../DocumentFactory.h"
#include "../../Models/Flux/Representation.h"
#include "../../Formats/Hfe/Constants.h"
#include "../../Formats/Definitions/Constants.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaEngine/DiagnosticKeys.h>
#include <QLocale>
#include <algorithm>
#include <array>
#include <cmath>
#include <map>
namespace RetroMediaLab::MediaEngine {
namespace {
constexpr std::array Kinds{RepresentationKind::Flux};
const Definitions::DiskDefinition* definition(std::string_view id){const auto& definitions=Definitions::embedded();const auto found=definitions.find(id);return found==definitions.end()?nullptr:&found->second;}
}
std::string_view HfeCellConverter::id()const noexcept{return RepresentationConversionConstants::HfeCellsId;}
std::span<const RepresentationKind> HfeCellConverter::sourceKinds()const noexcept{return Kinds;}
std::span<const RepresentationKind> HfeCellConverter::targetKinds()const noexcept{return Kinds;}
bool HfeCellConverter::canConvert(const MediaDocument& source,std::string_view target,RepresentationKind kind)const{
 const auto flux=dynamic_cast<const FluxRepresentation*>(&source.representation());if(kind!=RepresentationKind::Flux||!flux||flux->tracks().empty())return false;
 if(!std::all_of(flux->tracks().begin(),flux->tracks().end(),[](const auto& track){return !track.revolutions().empty();}))return false;
 if(Primitives::asciiIdentifierEqualsIgnoreCase(target,Hfe::Constants::FormatId))return true;
 const auto selected=definition(target);return selected&&std::all_of(selected->tracks.begin(),selected->tracks.end(),[](const auto& track){return track.codec==Definitions::Constants::BitcellCodec;});
}
RepresentationConversionResult HfeCellConverter::convert(std::shared_ptr<const MediaDocument> source,std::string target,RepresentationKind kind,const ConversionOptions& options,const RepresentationProgress& progress,std::stop_token token)const{
 namespace C=RepresentationConversionConstants;(void)options;(void)progress;checkCancellation(token);
 if(!source||!canConvert(*source,target,kind))throw MediaError(DiagnosticCode::RepresentationHfe,{target});
 const auto& flux=dynamic_cast<const FluxRepresentation&>(source->representation());const auto selected=definition(target);double nanoseconds;
 if(selected){
  if(selected->tracks.empty())throw MediaError(DiagnosticCode::RepresentationHfe,{target});
  const auto clock=selected->tracks.front().parameters.find(C::ClockParameter);bool valid=false;
  nanoseconds=clock==selected->tracks.front().parameters.end()?0:QLocale::c().toDouble(QString::fromStdString(clock->second),&valid)*C::NanosecondsPerMicrosecond;
  if(!valid||!std::isfinite(nanoseconds)||nanoseconds<=0)throw MediaError(DiagnosticCode::RepresentationHfe,{target});
 }else{
  std::vector<double> estimates;for(const auto& track:flux.tracks()){checkCancellation(token);const auto& revolution=track.revolutions().front();estimates.push_back(Flux::estimateCell(revolution.flux().intervals,Flux::TimingMode::NonFm,token)*revolution.resolutionNanoseconds());}
  std::sort(estimates.begin(),estimates.end());const auto estimate=estimates[estimates.size()/C::MedianDivisor];
  nanoseconds=*std::min_element(C::NominalCells.begin(),C::NominalCells.end(),[&](auto left,auto right){return std::abs(left-estimate)<std::abs(right-estimate);});
 }
 struct Track {std::int32_t cylinder,head;Flux::Bits bits;};std::vector<Track> decoded;std::vector<std::int32_t> cylinders;std::map<std::int32_t,std::size_t> sizes;
 for(const auto& track:flux.tracks()){
  checkCancellation(token);const auto& revolution=track.revolutions().front();
  const auto cells=Flux::decodePll(revolution.flux().intervals,nanoseconds/revolution.resolutionNanoseconds(),token);
  const double exact=revolution.flux().indexTimeTicks*double(revolution.resolutionNanoseconds())/nanoseconds;
  if(!std::isfinite(exact)||exact<0||exact>C::MaximumBits-C::BitsPerByte)throw MediaError(DiagnosticCode::RepresentationHfe,{target});
  const auto lower=std::floor(exact);const auto fraction=exact-lower;
  auto count=std::int64_t(lower)+(fraction>C::Half||(fraction==C::Half&&std::int64_t(lower)%C::RoundingParity!=0)?1:0);
  count=(count+C::BitsPerByte-1)/C::BitsPerByte*C::BitsPerByte;Flux::Bits bits(std::size_t(count),0);
  std::copy_n(cells.bits().begin(),std::min(bits.size(),cells.bits().size()),bits.begin());
  if(!sizes.contains(track.cylinder()))cylinders.push_back(track.cylinder());sizes[track.cylinder()]=std::max(sizes[track.cylinder()],bits.size());decoded.push_back({track.cylinder(),track.head(),std::move(bits)});
 }
 std::vector<Flux::ProtectedTrack> tracks;
 for(const auto cylinder:cylinders)for(auto& track:decoded)if(track.cylinder==cylinder){checkCancellation(token);track.bits.resize(sizes[cylinder]);const auto count=std::int64_t(track.bits.size());tracks.emplace_back(track.cylinder,track.head,std::move(track.bits),std::vector<Flux::TrackTimingSegment>{{0,count,nanoseconds}},std::vector<Decoding::Structure>{},std::vector<Flux::TrackFeature>{},std::vector<Flux::TrackFluxRevolution>{});}
 auto metadata=source->metadata();metadata.erase(std::string(Hfe::Constants::BitRateKey));if(selected)metadata[std::string(Hfe::Constants::EncodingKey)]=std::to_string(nanoseconds==C::FmCellNanoseconds?Hfe::Constants::IsoFm:Hfe::Constants::IsoMfm);
 return {floppyFluxDocument(source->mediaSource(),std::move(target),std::make_shared<const Flux::ProtectedTrackImage>(std::move(tracks),flux.image().writeProtected()),std::move(metadata)),{},{{std::string(DiagnosticKeys::RepresentationHfeLoss),{},{}}}};
}
}
