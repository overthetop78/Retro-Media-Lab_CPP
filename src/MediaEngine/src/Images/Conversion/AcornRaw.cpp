#include "Acorn.h"
#include "AcornConstants.h"
#include "../DocumentFactory.h"
#include "../../Models/Sectors/Representation.h"
#include "../../Models/Sectors/SectorImages.h"
#include "../../Formats/Adf/Internal.h"
#include "../../Formats/BbcDfs/BbcDfsImages.h"
#include "../../Formats/FormatIds.h"
#include "../../Sources/RandomAccessSources.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaEngine/DiagnosticKeys.h>
#include <RetroMediaLab/MediaFileSystems/Adfs.h>
#include <RetroMediaLab/MediaFileSystems/FileCore.h>
#include <algorithm>
#include <array>
namespace RetroMediaLab::MediaEngine::AcornConversion {
namespace M=MediaFileSystems;
namespace Details {
namespace {
std::optional<std::vector<std::uint8_t>> rawBytes(const M::SectorImage& image,std::uint64_t capacity,std::stop_token token,const std::function<bool(M::Bytes)>& validateHeader = {}){
 if(image.geometry().blockSize!=Constants::RawBlockSize||image.capacity()!=capacity||image.blockCount()*Constants::RawBlockSize!=capacity)return {};
 std::uint64_t count=0;image.visitAvailableBlocks([&](auto){++count;return true;},token);if(count!=image.blockCount())return {};
 if(validateHeader){if(!image.containsBlock(0))return {};const auto first=image.block(0,token);if(!first||first->data.size()!=Constants::RawBlockSize||!validateHeader(first->data))return {};}
 std::vector<std::uint8_t> bytes(std::size_t(capacity),0);bool valid=true;
 image.visitAvailableBlocks([&](auto logical){
  checkCancellation(token);const auto block=image.block(logical,token);
  if(!block||block->data.size()!=Constants::RawBlockSize||block->integrityValid==false||block->headerIntegrityValid==false||block->dataIntegrityValid==false||block->deletedDataMark==true||(block->tag&&!block->tag->empty())||block->formatCode||block->diagnosticCode||block->recordedAddress||block->recordedSizeCode||block->recordedTrackOrder||!block->recordedErrorCheckCode.empty()||!block->recordedDataPrefix.empty()){valid=false;return false;}
  std::copy(block->data.begin(),block->data.end(),bytes.begin()+std::size_t(logical*Constants::RawBlockSize));return true;
 },token);
 return valid?std::optional(std::move(bytes)):std::nullopt;
}
}
std::shared_ptr<const M::SectorImage> raw(const MediaDocument& source,std::string_view target,bool dfs,std::stop_token token){
 const auto sectors=dynamic_cast<const SectorRepresentation*>(&source.representation());
 if(source.mediaKind()!=MediaKind::Floppy||!sectors||(!Primitives::asciiIdentifierStartsWithIgnoreCase(source.formatId(),Formats::IbmPrefix)&&!Primitives::asciiIdentifierStartsWithIgnoreCase(source.formatId(),Formats::AtariStPrefix)))return {};
 if(dfs){
  const auto profile=BbcDfs::findGeometry(target);if(!profile)return {};
  const auto capacity=std::uint64_t(profile->blockSize)*profile->cylinders*profile->heads*profile->sectorsPerTrack;
  auto bytes=rawBytes(sectors->image(),capacity,token);if(!bytes||!BbcDfs::hasCatalogues(*bytes,*profile))return {};
  return sourceSectorImage(memorySource(*bytes),*profile,0,{},SourceSectorMetadata{0,0,false});
 }
 const auto profile=Adf::findGeometry(target);
 if(!profile||!Primitives::asciiIdentifierStartsWithIgnoreCase(profile->geometry.formatId,Formats::AcornAdfsPrefix)||profile->geometry.formatId==Formats::AcornAdfs1600)return {};
 const auto capacity=Adf::Internal::capacity(*profile);
 const auto validHeader=[&](M::Bytes header){
  const auto oldMap=M::FileCore::oldMapDeclaredCapacity(header);bool newMap=false;
  if(profile->geometry.blockSize==Constants::ArchimedesBlockSize){const auto record=M::FileCore::parseDiscRecord(header.subspan(M::FileCore::Layout::DiscRecordOffset),capacity);newMap=record&&record->discSize==capacity&&(std::uint64_t(1)<<record->log2SectorSize)==profile->geometry.blockSize;}
  return (oldMap&&*oldMap==capacity)||newMap;
 };
 auto bytes=rawBytes(sectors->image(),capacity,token,validHeader);if(!bytes)return {};
 auto candidate=Adf::Internal::image(memorySource(*bytes),*profile);
 return M::Adfs::canRead(*candidate,token)?candidate:std::shared_ptr<const M::SectorImage>{};
}
}
Converter::Converter(Mode mode):mode_(mode){}
std::string_view Converter::id()const noexcept{return mode_==Mode::ContainerAdf?Constants::ContainerAdfId:mode_==Mode::RawAdf?Constants::RawAdfId:Constants::RawDfsId;}
std::span<const RepresentationKind> Converter::sourceKinds()const noexcept{static constexpr std::array kinds{RepresentationKind::Sectors};return kinds;}
std::span<const RepresentationKind> Converter::targetKinds()const noexcept{return sourceKinds();}
bool Converter::canConvert(const MediaDocument& source,std::string_view target,RepresentationKind kind)const{
 if(kind!=RepresentationKind::Sectors)return false;
 return mode_==Mode::ContainerAdf?Details::container(source,target,{}).has_value():bool(Details::raw(source,target,mode_==Mode::RawDfs,{}));
}
RepresentationConversionResult Converter::convert(std::shared_ptr<const MediaDocument> source,std::string target,RepresentationKind kind,const ConversionOptions& options,const RepresentationProgress& progress,std::stop_token token)const{
 (void)options;(void)progress;checkCancellation(token);
 if(!source||kind!=RepresentationKind::Sectors)throw MediaError(DiagnosticCode::AcornRepresentation,{source?source->formatId():std::string{},target});
 std::shared_ptr<const M::SectorImage> converted;RepresentationMessages losses;
 if(mode_==Mode::ContainerAdf){
  const auto profile=Details::container(*source,target,token);if(!profile)throw MediaError(DiagnosticCode::AcornRepresentation,{source->formatId(),target});
  converted=Details::containerImage(*source,*profile,token);
  const auto same=Primitives::asciiIdentifierEqualsIgnoreCase;
  losses.push_back({std::string(same(source->formatId(),Formats::Jfd)?DiagnosticKeys::AcornJfdLoss:same(source->formatId(),Formats::CpcEmuDsk)?DiagnosticKeys::AcornCpcLoss:DiagnosticKeys::AcornTd0Loss),{},{}});
  if(dynamic_cast<const SectorRepresentation&>(source->representation()).image().geometry().cylinders!=profile->geometry.cylinders)losses.push_back({std::string(DiagnosticKeys::AcornTrailingLoss),{},{}});
 }else{
  converted=Details::raw(*source,target,mode_==Mode::RawDfs,token);
  if(!converted)throw MediaError(DiagnosticCode::AcornRepresentation,{source->formatId(),target});
  losses.push_back({std::string(mode_==Mode::RawDfs?DiagnosticKeys::AcornRawDfsLoss:DiagnosticKeys::AcornRawAdfLoss),{},{}});
 }
 return {floppySectorDocument(source->mediaSource(),std::move(converted)),{},std::move(losses)};
}
}
