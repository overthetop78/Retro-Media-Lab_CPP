#include "SectorReader.h"
#include "Constants.h"
#include "../Raw/RawImages.h"
#include "../Raw/Constants.h"
#include "../../Codecs/Iso/FmConstants.h"
#include "../../Codecs/Iso/MfmConstants.h"
#include "../../Reconstruction/Iso/Builder.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaEngine::I86f {
std::optional<Flux::Revolution> sectorRevolution(Flux::BitView bits,std::stop_token token){
 checkCancellation(token);if(bits.size()>Constants::MaximumFluxTicks/Constants::TicksPerCell)throw MediaError(DiagnosticCode::FluxIntervalOverflow);
 Flux::Revolution revolution;revolution.indexTimeTicks=static_cast<std::uint32_t>(bits.size()*Constants::TicksPerCell);std::uint32_t cells=0;
 for(auto bit:bits){checkCancellation(token);if(bit>1)throw MediaError(DiagnosticCode::FluxInvalidBit);++cells;if(!bit)continue;revolution.intervals.push_back(cells*Constants::TicksPerCell);cells=0;}
 if(revolution.intervals.empty())return {};return revolution;
}
std::shared_ptr<const MediaFileSystems::SectorImage> reconstructSectors(const Image& image,const Decoding::DecodeOperation& decode,std::stop_token token){
 checkCancellation(token);if(!decode)throw MediaError(DiagnosticCode::RegistryNullDecoder);IsoReconstruction::Candidates candidates;
 for(const auto& track:image.tracks){checkCancellation(token);auto revolution=sectorRevolution(track.bits,token);if(!revolution)continue;
  const auto id=(track.flags&Constants::EncodingMask)==Constants::MfmEncoding?IsoMfm::Constants::Id:IsoFm::Constants::Id;
  auto decoded=decode(id,std::make_shared<const Flux::Revolution>(std::move(*revolution)));
  for(auto& sector:decoded.sectors){checkCancellation(token);if(!sector.data||sector.number<0)continue;const MediaFileSystems::SectorAddress address{sector.cylinder,sector.head,sector.number};candidates.add(address,{std::move(sector),0,track.logicalIndex});}
 }
 if(candidates.empty())throw MediaError(DiagnosticCode::IsoNoAddressedCandidates);
 const auto measured=IsoReconstruction::measure(candidates,token);IsoReconstruction::UniformRequest request;
 request.sectorSize=measured.sectorSize;request.cylinders=measured.cylinders;request.heads=measured.heads;request.sectorsPerTrack=measured.sectorsPerTrack;
 if(measured.sectorSize==Raw::Constants::SectorSize){std::uint64_t capacity=measured.sectorSize;for(const auto factor:{measured.cylinders,measured.heads,measured.sectorsPerTrack}){if(factor<=0||capacity>std::uint64_t(std::numeric_limits<std::int32_t>::max())/factor)throw MediaError(DiagnosticCode::IsoGeometryOverflow);capacity*=factor;}const auto known=Raw::ibmFromCapacity(capacity);request.formatId=known?known->formatId:std::string(Raw::Constants::IbmPrefix)+std::to_string(capacity/Raw::Constants::BytesPerKibibyte);}
 else request.formatId=std::string(Constants::SectorFormatPrefix)+std::to_string(measured.sectorSize)+Constants::GeometrySeparator+std::to_string(measured.cylinders)+Constants::GeometrySeparator+std::to_string(measured.heads)+Constants::GeometrySeparator+std::to_string(measured.sectorsPerTrack);
 request.sectorIndex=[zero=measured.zeroBased,order=measured.sectorOrder](const MediaFileSystems::SectorAddress& address){if(!zero)return static_cast<std::int32_t>(address.number-1);const auto found=std::find(order.begin(),order.end(),address.number);return found==order.end()?-1:static_cast<std::int32_t>(found-order.begin());};
 return IsoReconstruction::createUniform(candidates,request,token);
}
}
