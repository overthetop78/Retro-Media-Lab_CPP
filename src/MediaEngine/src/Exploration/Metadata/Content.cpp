#include "Metadata.h"
#include "Constants.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::DiskMetadata {
namespace {
using namespace Constants;
std::uint32_t big(MediaFileSystems::Bytes bytes,std::size_t offset){std::uint32_t result=0;for(std::size_t byte=0;byte<sizeof(result);++byte)result=(result<<ByteBits)|bytes[offset+byte];return result;}
bool starts(MediaFileSystems::Bytes bytes,std::string_view signature){return bytes.size()>=signature.size()&&std::equal(signature.begin(),signature.end(),bytes.begin());}
bool containsInOrder(MediaFileSystems::Bytes bytes){
 const auto first=std::search(bytes.begin(),bytes.end(),CrackedBy.begin(),CrackedBy.end());if(first==bytes.end())return false;
 const auto following=first+std::ptrdiff_t(CrackedBy.size());const auto second=std::search(following,bytes.end(),TheCompany.begin(),TheCompany.end());return second!=bytes.end()&&std::size_t(second-following)<=MaximumSignatureGap;
}
bool atLeastPercent(std::uint64_t available,std::uint64_t total,std::uint32_t percentage){const auto required=(total/Percent)*percentage+((total%Percent)*percentage+Percent-1)/Percent;return available>=required;}
bool validBoot(const MediaFileSystems::SectorImage& image,MediaFileSystems::Bytes bytes,std::uint64_t available){
 if(!Primitives::ordinalIdentifierStartsWithIgnoreCase(image.geometry().formatId,Formats::AmigaPrefix)||available!=image.blockCount()||bytes.size()<BootBlockLength||!starts(bytes,DosSignature))return false;
 std::uint32_t sum=0;for(std::size_t offset=0;offset<BootBlockLength;offset+=sizeof(sum)){const auto previous=sum;sum+=big(bytes,offset);if(sum<previous)++sum;}
 return sum==std::numeric_limits<std::uint32_t>::max()&&std::any_of(bytes.begin()+BootPayloadOffset,bytes.begin()+BootBlockLength,[](auto value){return value!=0;});
}
bool catalogless(const MediaFileSystems::SectorImage& image,MediaFileSystems::Bytes bytes,std::uint64_t available,std::uint64_t occupied){
 if(!Primitives::ordinalIdentifierStartsWithIgnoreCase(image.geometry().formatId,Formats::AmigaPrefix)||!image.geometry().blockSize||bytes.size()<BootBlockLength||!image.blockCount()||!atLeastPercent(available,image.blockCount(),MinimumAvailablePercentage))return false;
 const auto payload=std::size_t(std::count_if(bytes.begin()+BootPayloadOffset,bytes.begin()+BootBlockLength,[](auto value){return value!=0;}));
 return payload>=MinimumBootPayloadBytes&&atLeastPercent(occupied,image.blockCount(),MinimumOccupiedPercentage);
}
}
AtnDetection detectAtnArchive(MediaFileSystems::Bytes bytes,std::uint32_t sectorSize){
 AtnDetection result;if(!sectorSize||bytes.size()<sectorSize||bytes.size()<AtnHeaderSize)return result;
 for(std::size_t offset=0;offset<=bytes.size()-AtnHeaderSize;offset+=sectorSize){
  if(!starts(bytes.subspan(offset),AtnSignature))continue;const auto expanded=big(bytes,offset+AtnExpandedOffset),compressed=big(bytes,offset+AtnCompressedOffset);
  if(!compressed||!expanded||std::uint64_t(expanded)>std::uint64_t(compressed)*MaximumExpansionRatio||std::uint64_t(offset)+AtnHeaderSize+compressed>bytes.size())continue;++result.memberCount;
 }
 result.recognized=result.memberCount>=MinimumAtnMembers;return result;
}
DiskContentMetadata content(const MediaFileSystems::SectorImage& image,std::stop_token token){
 checkCancellation(token);DiskContentMetadata result;if(image.capacity()>std::uint64_t(MaximumFlattenLength))return result;
 std::uint64_t available=0;image.visitAvailableBlocks([&](auto){++available;return true;},token);if(!available)return result;
 std::vector<std::uint8_t> bytes(std::size_t(image.capacity()));std::uint64_t occupied=0;
 image.visitAvailableBlocks([&](auto number){
  checkCancellation(token);const auto block=image.block(number,token);if(std::any_of(block->data.begin(),block->data.end(),[](auto value){return value!=0;}))++occupied;
  if(number>image.capacity()/image.geometry().blockSize)return true;const auto offset=number*image.geometry().blockSize;
  if(offset>bytes.size()||block->data.size()>bytes.size()-std::size_t(offset))return true;
  std::copy(block->data.begin(),block->data.end(),bytes.begin()+std::ptrdiff_t(offset));return true;
 },token);
 if(containsInOrder(bytes))result.modificationId=std::string(DiskContentIds::CrackTheCompany);
 const auto atn=detectAtnArchive(bytes,image.geometry().blockSize);
 if(atn.recognized){result.organizationId=std::string(DiskContentIds::OrganizationAtnArchive);result.organizationMemberCount=atn.memberCount;}
 else if(catalogless(image,bytes,available,occupied))result.organizationId=std::string(DiskContentIds::OrganizationCataloglessBootImage);
 // Preserve the C# association for every recognized organization.
 if(result.organizationId)result.compressionIds.emplace_back(DiskContentIds::CompressionAtnImploder);
 result.hasValidAmigaBootLoader=validBoot(image,bytes,available);checkCancellation(token);return result;
}
}
