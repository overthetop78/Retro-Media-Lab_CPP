#include "Projection.h"
#include "ProjectionConstants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <limits>
#include <set>
#include <QString>
namespace RetroMediaLab::MediaEngine::DiskProjection {
DiskContainerInfo container(const DiskExploration& disk){
 DiskContainerInfo result;
 if(disk.capture){
  const auto& header=disk.capture->header;std::set<std::int32_t> heads;for(const auto& track:disk.capture->tracks)heads.insert(track.head);
  result.signature=Constants::Signature;result.diskType=std::to_string(header.diskType);result.resolutionNanoseconds=header.resolutionNanoseconds();result.revolutionCount=header.revolutions;result.firstTrack=header.startTrack;result.lastTrack=header.endTrack;result.trackCount=disk.capture->tracks.size();result.headCount=std::uint32_t(heads.size());result.checksumPresent=true;result.declaredChecksum=QString::number(header.checksum,Constants::HexBase).rightJustified(Constants::ChecksumWidth,QLatin1Char(Constants::ChecksumPadding)).toUpper().toStdString();result.checksumValid=disk.capture->checksumValid;
  result.formatProperties={{std::string(Constants::Flags),std::to_string(header.flags)},{std::string(Constants::BitCellEncoding),std::to_string(header.bitCellEncoding)},{std::string(Constants::Heads),std::to_string(header.heads)}};
 }else{
  const auto& geometry=disk.image->geometry();result.trackCount=std::uint64_t(geometry.cylinders)*geometry.heads;result.headCount=geometry.heads;const auto extension=disk.sourcePath.extension().u8string();result.formatProperties.emplace(Constants::Extension,std::string(extension.begin(),extension.end()));
 }return result;
}
std::vector<DiskTrackInfo> tracks(const DiskExploration& disk,const Cache& cache){
 std::vector<DiskTrackInfo> result;
 if(disk.capture){
  for(const auto& source:disk.capture->tracks){
   DiskTrackInfo track;track.sourceNumber=source.trackNumber;track.cylinder=source.cylinder;track.head=source.head;std::int64_t start=0;
   for(std::size_t index=0;index<source.revolutions.size();++index){
    const auto& revolution=source.revolutions[index];const auto resolution=disk.capture->header.resolutionNanoseconds();const auto duration=std::int64_t(revolution.flux.indexTimeTicks)*resolution;
    if(start>std::numeric_limits<std::int64_t>::max()-duration)throw MediaError(DiagnosticCode::ScpOverflow);
    DiskRevolutionInfo info;info.number=std::int32_t(index);info.resolutionNanoseconds=resolution;info.startNanoseconds=start;info.durationNanoseconds=duration;info.declaredFluxCount=revolution.declaredFluxCount;info.origin=revolution.origin==Scp::RevolutionOrigin::Captured?DiskRevolutionOrigin::Captured:DiskRevolutionOrigin::Synthetic;info.fluxIntervals=std::shared_ptr<const std::vector<std::uint32_t>>(disk.capture,&revolution.flux.intervals);track.revolutions.push_back(std::move(info));start+=duration;
   }result.push_back(std::move(track));
  }return result;
 }
 const auto& geometry=disk.image->geometry();const auto& available=cache.images.at(disk.image.get());std::map<std::pair<std::int64_t,std::int64_t>,std::vector<Block>> grouped;
 for(const auto number:available.order){const auto& block=available.values.at(number);grouped[{block->address.cylinder,block->address.head}].push_back(block);}
 for(std::uint32_t cylinder=0;cylinder<geometry.cylinders;++cylinder)for(std::uint32_t head=0;head<geometry.heads;++head){
  DiskTrackInfo track;track.cylinder=std::int32_t(cylinder);track.head=std::int32_t(head);auto& blocks=grouped[{cylinder,head}];std::stable_sort(blocks.begin(),blocks.end(),[](const auto& a,const auto& b){return a->address.number<b->address.number;});
  for(const auto& block:blocks)track.sourceSectors.push_back({block->address.number,block->data.size(),DiskBytes(block,&block->data)});result.push_back(std::move(track));
 }return result;
}
}
