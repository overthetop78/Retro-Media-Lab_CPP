#include "Acorn.h"
#include "AcornConstants.h"
#include "../../Models/Sectors/Representation.h"
#include "../../Models/Sectors/SectorImages.h"
#include "../../Formats/Adf/Internal.h"
#include "../../Formats/FormatIds.h"
#include "../../Encoding/Timings.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaFileSystems/FileCore.h>
namespace RetroMediaLab::MediaEngine::AcornConversion::Details {
namespace M=MediaFileSystems;
std::optional<Adf::Profile> container(const MediaDocument& source,std::string_view target,std::stop_token token){
 const auto same=Primitives::asciiIdentifierEqualsIgnoreCase;
 const bool jfd=same(source.formatId(),Formats::Jfd),cpc=same(source.formatId(),Formats::CpcEmuDsk),td0=same(source.formatId(),Formats::UcsdIbmMfm);
 const auto sectors=dynamic_cast<const SectorRepresentation*>(&source.representation());
 if(source.mediaKind()!=MediaKind::Floppy||(!jfd&&!cpc&&!td0)||!sectors)return {};
 const auto profile=Adf::findGeometry(target);if(!profile||!Primitives::asciiIdentifierStartsWithIgnoreCase(profile->geometry.formatId,Formats::AcornAdfsPrefix))return {};
 const auto& g=profile->geometry;const auto& image=sectors->image();const auto& observed=image.geometry();
 const auto count=std::uint64_t(g.cylinders)*g.heads*g.sectorsPerTrack;
 if(observed.addressing!=M::SectorAddressingKind::Physical||observed.blockSize!=g.blockSize||observed.cylinders<g.cylinders||observed.heads!=g.heads||observed.sectorsPerTrack!=g.sectorsPerTrack||image.blockCount()<count)return {};
 std::uint64_t available=0;image.visitAvailableBlocks([&](auto){++available;return true;},token);if(available!=count)return {};
 if(g.blockSize==Constants::BbcBlockSize&&observed.cylinders!=g.cylinders){
  if(!image.containsBlock(0)||!image.containsBlock(1))return {};
  const auto first=image.block(0,token),second=image.block(1,token);
  if(!first||!second||first->data.size()!=Constants::BbcBlockSize||second->data.size()!=Constants::BbcBlockSize)return {};
  std::vector<std::uint8_t> map=first->data;map.insert(map.end(),second->data.begin(),second->data.end());const auto declared=M::FileCore::oldMapDeclaredCapacity(map);
  if(!declared||*declared!=Adf::Internal::capacity(*profile))return {};
 }
 const auto density=Constants::DensityBase/Encoding::bitCellTicks(g.formatId);
 const auto size=g.blockSize==Constants::BbcBlockSize?Constants::BbcSizeCode:Constants::ArchimedesSizeCode;
 const auto rate=same(g.formatId,Formats::AcornAdfs1600)?Constants::HighDensityRate:Constants::DoubleDensityRate;
 bool valid=true;image.visitAvailableBlocks([&](auto logical){
  checkCancellation(token);const auto block=image.block(logical,token);if(!block||logical>=count){valid=false;return false;}
  const auto perCylinder=std::uint64_t(g.heads)*g.sectorsPerTrack;
  const M::SectorAddress address{std::int64_t(logical/perCylinder),std::int64_t(logical/g.sectorsPerTrack%g.heads),std::int64_t(logical%g.sectorsPerTrack)+profile->firstSectorNumber};
  auto recorded=address;if(g.blockSize==Constants::BbcBlockSize)recorded.head=0;
  valid=block->address==address&&block->recordedAddress.value_or(block->address)==recorded&&block->data.size()==g.blockSize&&block->integrityValid==true&&block->headerIntegrityValid!=false&&block->dataIntegrityValid!=false&&block->deletedDataMark!=true;
  if(jfd)valid=valid&&block->formatCode==density&&(!block->tag||block->tag->size()<=Constants::JfdMaximumTag);
  else if(cpc)valid=valid&&block->formatCode==size&&(!block->tag||block->tag->empty());
  else valid=valid&&block->formatCode==rate&&block->diagnosticCode==Constants::GoodDiagnostic&&block->recordedSizeCode==size&&(!block->tag||block->tag->empty());
  valid=valid&&(!block->recordedSizeCode||block->recordedSizeCode==size);return valid;
 },token);
 return valid?profile:std::nullopt;
}
std::shared_ptr<const M::SectorImage> containerImage(const MediaDocument& source,const Adf::Profile& profile,std::stop_token token){
 const auto& image=dynamic_cast<const SectorRepresentation&>(source.representation()).image();const auto jfd=Primitives::asciiIdentifierEqualsIgnoreCase(source.formatId(),Formats::Jfd);std::vector<M::SectorBlock> blocks;
 image.visitAvailableBlocks([&](auto logical){checkCancellation(token);auto block=*image.block(logical,token);if(!jfd){block.formatCode.reset();block.tag.reset();block.recordedTrackOrder.reset();block.diagnosticCode.reset();}blocks.push_back(std::move(block));return true;},token);
 return sparseSectorImage(profile.geometry,std::move(blocks));
}
}
