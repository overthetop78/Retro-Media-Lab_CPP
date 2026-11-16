#include "FluxContainers.h"
#include "../../Models/Flux/Conversion.h"
#include "../../Formats/Scp/Writer.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <tuple>
namespace RetroMediaLab::MediaEngine::FluxContainerConversion {
namespace {
void require(bool value){if(!value)throw MediaError(DiagnosticCode::FluxContainerParity);}
auto headerFields(const Scp::Header& h){return std::tuple(h.version,h.diskType,h.revolutions,h.startTrack,h.endTrack,h.flags,h.bitCellEncoding,h.heads,h.resolution);}
}
void validateParity(const Hfe::Image& expected,const Hfe::Image& actual,std::stop_token token){
 checkCancellation(token);
 require(expected.revision==actual.revision&&expected.cylinders==actual.cylinders&&expected.heads==actual.heads&&expected.encoding==actual.encoding&&expected.bitRate==actual.bitRate&&expected.tracks.size()==actual.tracks.size()&&expected.writeProtected==actual.writeProtected);
 for(const auto& source:expected.tracks){checkCancellation(token);const auto target=std::find_if(actual.tracks.begin(),actual.tracks.end(),[&](const auto& t){return t.cylinder==source.cylinder&&t.head==source.head;});require(target!=actual.tracks.end());require(source.bitCellTicks==target->bitCellTicks&&source.bits==target->bits);}
}
void validateParity(const Scp::Image& expected,const Scp::Image& actual,std::stop_token token){
 checkCancellation(token);require(actual.checksumValid&&headerFields(expected.header)==headerFields(actual.header)&&expected.tracks.size()==actual.tracks.size());
 for(std::size_t index=0;index<expected.tracks.size();++index){
  checkCancellation(token);const auto& source=expected.tracks[index];const auto& target=actual.tracks[index];
  require(source.trackNumber==target.trackNumber&&source.cylinder==target.cylinder&&source.head==target.head&&source.revolutions.size()==target.revolutions.size());
  for(std::size_t revolution=0;revolution<source.revolutions.size();++revolution){checkCancellation(token);const auto& a=source.revolutions[revolution];const auto& b=target.revolutions[revolution];require(a.flux.indexTimeTicks==b.flux.indexTimeTicks&&a.declaredFluxCount==b.declaredFluxCount&&a.flux.intervals==b.flux.intervals);}
 }
}
void validateParity(const Hfe::Image& expected,const Scp::Image& actual,std::stop_token token){
 checkCancellation(token);require(actual.checksumValid&&actual.header.revolutions==Scp::Constants::CaptureRevolutions);
 for(const auto& source:expected.tracks){
  checkCancellation(token);const auto number=Scp::trackNumber(source.cylinder,source.head);
  const auto target=std::find_if(actual.tracks.begin(),actual.tracks.end(),[&](const auto& t){return t.trackNumber==number;});require(target!=actual.tracks.end());require(target->revolutions.size()==Scp::Constants::CaptureRevolutions);
  const auto& revolution=target->revolutions.front();require(source.bits==Flux::toBits(revolution.flux.intervals,revolution.flux.indexTimeTicks,source.bitCellTicks));
 }
}
}
