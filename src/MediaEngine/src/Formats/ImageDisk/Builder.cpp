#include "ImageDisk.h"
#include "Constants.h"
#include "../Epson/Geometry.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <map>
#include <tuple>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::ImageDisk {
Image buildEpson(std::shared_ptr<const MediaFileSystems::SectorImage> source,Mode mode){if(!source)throw MediaError(DiagnosticCode::ImdEpsonGeometry);const auto& geometry=Epson::resolve(source->geometry().formatId);if(source->geometry().cylinders!=std::uint32_t(geometry.cylinders)||source->geometry().heads!=std::uint32_t(geometry.heads))throw MediaError(DiagnosticCode::ImdEpsonGeometry);using Address=std::tuple<std::int64_t,std::int64_t,std::int64_t>;std::map<Address,std::shared_ptr<const MediaFileSystems::SectorBlock>> blocks;source->visitAvailableBlocks([&](auto logical){const auto b=source->block(logical);if(!blocks.emplace(Address{b->address.cylinder,b->address.head,b->address.number},b).second)throw MediaError(DiagnosticCode::ImdEpsonDuplicate,{std::to_string(b->address.cylinder),std::to_string(b->address.head),std::to_string(b->address.number)});return true;});Image result;result.comment=Constants::DefaultComment;result.sectorImage=source;
 for(std::int32_t cylinder=0;cylinder<geometry.cylinders;++cylinder)for(std::int32_t head=0;head<geometry.heads;++head){const auto layout=geometry.track(cylinder,head);if(!layout.count)continue;Track track;track.mode=mode;track.cylinder=std::uint8_t(cylinder);track.head=std::uint8_t(head);for(std::int32_t index=0;index<layout.count;++index){Sector s;s.cylinder=track.cylinder;s.head=track.head;s.number=std::uint8_t(layout.firstSector+index);s.size=layout.sectorSize;const auto found=blocks.find({cylinder,head,layout.firstSector+index});if(found==blocks.end()){s.recordType=RecordType::Unavailable;s.data.resize(std::size_t(s.size));}else{const auto&b=*found->second;if(b.data.size()!=std::size_t(s.size))throw MediaError(DiagnosticCode::ImdEpsonSize,{std::to_string(cylinder),std::to_string(head),std::to_string(layout.firstSector+index)});const bool packed=std::all_of(b.data.begin(),b.data.end(),[&](auto value){return value==b.data.front();});s.recordType=recordType(packed,b.integrityValid!=false,b.deletedDataMark==true);s.data=b.data;}track.sectors.push_back(std::move(s));}result.tracks.push_back(std::move(track));}return result;
}
}
