#include "ImageDisk.h"
#include "Constants.h"
#include "../Epson/Geometry.h"
#include "../FormatIds.h"
#include "../../Models/Sectors/SectorImages.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <tuple>
namespace RetroMediaLab::MediaEngine::ImageDisk {
bool hasData(RecordType t) noexcept{return t!=RecordType::Unavailable;}
bool compressed(RecordType t) noexcept{return t==RecordType::Compressed||t==RecordType::CompressedDeleted||t==RecordType::CompressedError||t==RecordType::CompressedDeletedError;}
bool integrityValid(RecordType t) noexcept{return t==RecordType::Normal||t==RecordType::Compressed||t==RecordType::Deleted||t==RecordType::CompressedDeleted;}
bool deleted(RecordType t) noexcept{return t==RecordType::Deleted||t==RecordType::CompressedDeleted||t==RecordType::DeletedError||t==RecordType::CompressedDeletedError;}
RecordType recordType(bool packed,bool valid,bool removed) noexcept{if(removed)return valid?(packed?RecordType::CompressedDeleted:RecordType::Deleted):(packed?RecordType::CompressedDeletedError:RecordType::DeletedError);return valid?(packed?RecordType::Compressed:RecordType::Normal):(packed?RecordType::CompressedError:RecordType::NormalError);}
std::shared_ptr<const MediaFileSystems::SectorImage> buildImage(std::span<const Track> tracks){struct Item {const Track*track;const Sector*sector;std::size_t order;};std::vector<Item> ordered;for(const auto& t:tracks)for(std::size_t order=0;order<t.sectors.size();++order)ordered.push_back({&t,&t.sectors[order],order});std::stable_sort(ordered.begin(),ordered.end(),[](const auto&a,const auto&b){return std::tuple{a.track->cylinder,a.track->head,a.sector->number}<std::tuple{b.track->cylinder,b.track->head,b.sector->number};});if(ordered.empty())throw MediaError(DiagnosticCode::ImdNoSectors);
 std::vector<std::pair<std::int32_t,std::size_t>> sizes;std::uint32_t cylinders=0,heads=0,spt=0;for(const auto&t:tracks){cylinders=std::max(cylinders,std::uint32_t(t.cylinder)+1);heads=std::max(heads,std::uint32_t(t.head)+1);spt=std::max(spt,std::uint32_t(t.sectors.size()));}std::vector<MediaFileSystems::SectorBlock> blocks;std::vector<Epson::SectorDescriptor> descriptors;std::uint64_t capacity=0;bool addresses=true;
 for(std::size_t logical=0;logical<ordered.size();++logical){const auto& item=ordered[logical];const auto& s=*item.sector;auto found=std::find_if(sizes.begin(),sizes.end(),[&](const auto&p){return p.first==s.size;});if(found==sizes.end())sizes.emplace_back(s.size,1);else ++found->second;capacity+=std::uint64_t(s.size);descriptors.push_back({s.cylinder,s.head,s.number,s.size});addresses&=s.cylinder==item.track->cylinder&&s.head==item.track->head;if(!hasData(s.recordType))continue;MediaFileSystems::SectorBlock b;b.logicalBlock=logical;b.address={item.track->cylinder,item.track->head,s.number};b.data=s.data;b.integrityValid=integrityValid(s.recordType);b.diagnosticCode=std::uint8_t(s.recordType);b.recordedAddress=MediaFileSystems::SectorAddress{s.cylinder,s.head,s.number};b.recordedTrackOrder=std::int32_t(item.order);b.dataIntegrityValid=integrityValid(s.recordType);b.deletedDataMark=deleted(s.recordType);blocks.push_back(std::move(b));}
 const auto dominant=std::max_element(sizes.begin(),sizes.end(),[](const auto&a,const auto&b){return a.second<b.second;})->first;const auto detected=addresses?Epson::detect(descriptors):std::nullopt;MediaFileSystems::SectorImageOptions options;options.allowVariableBlockSize=sizes.size()>1;options.capacity=capacity;options.logicalBlockCount=ordered.size();return sparseSectorImage({detected.value_or(std::string(Formats::Imd)),std::uint32_t(dominant),cylinders,heads,spt},std::move(blocks),options);
}
}
