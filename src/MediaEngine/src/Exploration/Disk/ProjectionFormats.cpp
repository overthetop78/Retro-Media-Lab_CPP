#include "Projection.h"
#include "ProjectionConstants.h"
#include "../../Images/ExplorationMapper.h"
#include "../../Formats/FormatIds.h"
#include "../../Primitives/Identifiers.h"
#include "../Metadata/Metadata.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::DiskProjection {
namespace {
DiskBytes empty(){static const auto bytes=std::make_shared<const std::vector<std::uint8_t>>();return bytes;}
DiskSectorInfo sector(const MediaFileSystems::SectorImage& image,std::uint64_t logical,const Cache& cache){
 DiskSectorInfo result;result.logicalBlock=logical;const auto& geometry=image.geometry();const auto block=cache.block(image,logical);
 if(!block){const auto track=logical/geometry.sectorsPerTrack;result.cylinder=std::int64_t(track/geometry.heads);result.head=std::int64_t(track%geometry.heads);result.number=std::int64_t(logical%geometry.sectorsPerTrack);result.size=geometry.blockSize;result.data=empty();return result;}
 result.cylinder=block->address.cylinder;result.head=block->address.head;result.number=block->address.number;result.size=block->data.size();result.state=block->integrityValid==false?DiskSectorState::Invalid:DiskSectorState::Available;result.data=DiskBytes(block,&block->data);result.dataValid=block->integrityValid;if(block->tag)result.tag=DiskBytes(block,&*block->tag);result.formatCode=block->formatCode;result.diagnosticCode=block->diagnosticCode;result.revolutions.push_back(block->revolution);return result;
}
std::string encoding(std::string_view id){if(Primitives::ordinalIdentifierStartsWithIgnoreCase(id,Formats::AmigaPrefix))return std::string(Constants::AmigaEncoding);if(Primitives::ordinalIdentifierStartsWithIgnoreCase(id,Formats::IbmPrefix)||Primitives::ordinalIdentifierStartsWithIgnoreCase(id,Formats::AtariStPrefix))return std::string(Constants::IbmEncoding);return {};}
std::uint64_t entryCount(std::span<const FileSystemEntryInfo> entries){std::uint64_t count=0;std::vector<const FileSystemEntryInfo*> pending;for(const auto& entry:entries)pending.push_back(&entry);while(!pending.empty()){const auto entry=pending.back();pending.pop_back();++count;for(const auto& child:entry->children)pending.push_back(&child);}return count;}
std::shared_ptr<const FileSystemVolumeInfo> physicalVolume(const std::filesystem::path& path, const MediaFileSystems::SectorImage& image) {
 auto result=std::make_shared<FileSystemVolumeInfo>();
 const auto name=path.stem().u8string();
 result->name.assign(name.begin(),name.end());
 result->capacity=image.capacity();
 result->freeSpaceKnown=false;
 return result;
}
DiskFormatInfo format(const DiskExploration& disk,const SectorImageOwner& image,const ScpInspection::MatchOwner& match,const Cache& cache,std::stop_token token){
 DiskFormatInfo result;const auto& geometry=image->geometry();result.machineId=DiskMetadata::systemId(geometry.formatId).value_or(std::string{});result.formatId=geometry.formatId;result.encodingId=encoding(geometry.formatId);result.cylinders=geometry.cylinders;result.heads=geometry.heads;result.sectorsPerTrack=geometry.sectorsPerTrack;result.sectorSize=geometry.blockSize;result.capacity=image->capacity();
 for(std::uint64_t logical=0;logical<image->blockCount();++logical){checkCancellation(token);auto value=sector(*image,logical,cache);switch(value.state){case DiskSectorState::Available:++result.validSectors;break;case DiskSectorState::Invalid:++result.invalidSectors;break;case DiskSectorState::Missing:++result.missingSectors;break;}result.sectors.push_back(std::move(value));}
 if(match){
  result.volume=cache.volumes.at(match.get());if(!Primitives::isBlankIdentifier(result.volume->name))result.volumeName=result.volume->name;
  if(result.volume->freeSpaceKnown){result.freeBytes=result.volume->freeBytes;result.usedBytes=result.volume->capacity>=result.volume->freeBytes?result.volume->capacity-result.volume->freeBytes:0;}
  result.bootable=result.volume->bootable;result.entryCount=entryCount(result.volume->entries);result.diagnostics=result.volume->warnings;
 } else if (disk.capture) {
  result.volume=physicalVolume(disk.sourcePath,*image);
 }
 if(!result.bootable&&disk.metadata.content.hasValidAmigaBootLoader)result.bootable=true;
 result.content=disk.metadata.content;result.protectionId=disk.metadata.protectionId;return result;
}
}
std::shared_ptr<const FileSystemVolumeInfo> physicalVolume(const DiskExploration& disk){return physicalVolume(disk.sourcePath,*disk.image);}
std::vector<DiskFormatInfo> formats(const DiskExploration& disk,const Cache& cache,std::stop_token token){
 std::vector<DiskFormatInfo> result;std::vector<std::string> represented;
 const auto includes=[&](std::string_view id){return std::any_of(represented.begin(),represented.end(),[&](const auto& old){return Primitives::ordinalIdentifierEqualsIgnoreCase(old,id);});};
 for(const auto& match:disk.detected){checkCancellation(token);result.push_back(format(disk,match->image,match,cache,token));if(!includes(match->image->geometry().formatId))represented.push_back(match->image->geometry().formatId);}
 for(const auto& image:disk.decodedImages){checkCancellation(token);if(includes(image->geometry().formatId))continue;represented.push_back(image->geometry().formatId);result.push_back(format(disk,image,{},cache,token));}
 if(result.empty())result.push_back(format(disk,disk.image,{ },cache,token));return result;
}
}
