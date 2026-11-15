#include "Hfe.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <cmath>
namespace RetroMediaLab::MediaEngine::Hfe {
namespace {
using namespace Constants;
std::uint16_t word(MediaFileSystems::Bytes bytes,std::size_t offset){return std::uint16_t(bytes[offset]|(std::uint16_t(bytes[offset+1])<<BitsPerByte));}
std::uint8_t reverse(std::uint8_t value){std::uint8_t result=0;for(std::uint32_t i=0;i<BitsPerByte;++i)result|=std::uint8_t(((value>>i)&1)<<(BitsPerByte-1-i));return result;}
std::uint32_t rounded(double value){if(!std::isfinite(value)||value<0||std::nearbyint(value)>MaxTicks)throw MediaError(DiagnosticCode::HfeLimit);return std::uint32_t(std::nearbyint(value));}
Track decodeV3(std::int32_t cylinder,std::int32_t head,MediaFileSystems::Bytes packed,std::uint32_t nominal){
 Track t;t.cylinder=cylinder;t.head=head;t.bitCellTicks=nominal;double current=nominal,index=0,interval=0;std::size_t timingStart=0;std::uint32_t previous=0;std::optional<std::size_t> pending;std::uint32_t bitOffset=0;
 for(std::size_t offset=0;offset<packed.size();++offset){auto value=packed[offset];bool weak=false;if((value&OpcodeMask)==OpcodeMask){
  switch(value){case Nop:continue;case Index:pending=t.bits.size();continue;
   case Rate:{if(++offset>=packed.size())continue;const auto parameter=reverse(packed[offset]);if(!parameter)throw MediaError(DiagnosticCode::HfeRateOpcode,{std::to_string(cylinder),std::to_string(head)});if(t.bits.size()>timingStart)t.timing.push_back({std::int64_t(timingStart),std::int64_t(t.bits.size()-timingStart),current*TickNanoseconds});current=double(parameter)*NanosecondsPerMicrosecond/(ClockMegahertz*TickNanoseconds);timingStart=t.bits.size();continue;}
   case Skip:if(++offset>=packed.size())continue;bitOffset=reverse(packed[offset])&BitMask;continue;
   case Random:weak=true;value=RandomBits;break;default:continue;}}
  const auto first=t.bits.size();for(auto bit=bitOffset;bit<BitsPerByte;++bit){const bool set=(value&(1<<bit))!=0;t.bits.push_back(set);interval+=current;index+=current;if(!set)continue;const auto transition=rounded(index);t.revolution.intervals.push_back(transition-previous);previous=transition;interval=0;}
  if(weak&&t.bits.size()>first)t.features.push_back({Flux::TrackFeatureKind::WeakRegion,std::int64_t(first),std::int64_t(t.bits.size()-first),Decoding::message(WeakKey)});bitOffset=0;
 }
 if(t.bits.empty()||index==0)throw MediaError(DiagnosticCode::HfeEmptyTrack,{std::to_string(cylinder),std::to_string(head)});
 if(interval>0)t.revolution.intervals.push_back(rounded(index)-previous);if(t.bits.size()>timingStart)t.timing.push_back({std::int64_t(timingStart),std::int64_t(t.bits.size()-timingStart),current*TickNanoseconds});if(pending&&*pending<t.bits.size())t.features.push_back({Flux::TrackFeatureKind::IndexMark,std::int64_t(*pending),1,Decoding::message(IndexKey)});t.revolution.indexTimeTicks=rounded(index);return t;
}
}
Image read(MediaFileSystems::Bytes bytes,std::stop_token token){
 using namespace Constants;checkCancellation(token);if(bytes.size()<BlockSize)throw MediaError(DiagnosticCode::HfeHeader);const std::string_view signature(reinterpret_cast<const char*>(bytes.data()),SignatureLength);if(signature!=Signature&&signature!=Signature3)throw MediaError(DiagnosticCode::HfeHeader);const bool v3=signature==Signature3;
 Image image;image.revision=bytes[RevisionOffset];if(image.revision!=Revision)throw MediaError(DiagnosticCode::HfeRevision,{std::to_string(image.revision)});image.cylinders=bytes[CylinderOffset];image.heads=bytes[HeadOffset];image.encoding=bytes[EncodingOffset];image.bitRate=word(bytes,BitRateOffset);
 if(!image.cylinders||image.heads<1||image.heads>std::int32_t(MaxHeads)||!image.bitRate)throw MediaError(DiagnosticCode::HfeGeometry);
 const auto table=std::size_t(word(bytes,TableOffset))*BlockSize;if(table<BlockSize||table+image.cylinders*EntrySize>bytes.size())throw MediaError(DiagnosticCode::HfeTable);
 const auto ticks=NanosecondsPerSecond/(std::uint64_t(BitsPerDataBit)*TickNanoseconds*NanosecondsPerMicrosecond*image.bitRate);if(!ticks)throw MediaError(DiagnosticCode::HfeTiming);
 for(std::int32_t cylinder=0;cylinder<image.cylinders;++cylinder){checkCancellation(token);const auto entry=table+cylinder*EntrySize;const auto dataBlock=word(bytes,entry),length=word(bytes,entry+LengthOffset);if(length<MaxHeads)throw MediaError(DiagnosticCode::HfeTrackLength,{std::to_string(cylinder)});const auto sideLength=length/MaxHeads;
  for(std::int32_t head=0;head<image.heads;++head){std::vector<std::uint8_t> packed(sideLength);std::size_t remaining=sideLength,destination=0,block=dataBlock;while(remaining){const auto count=std::min(remaining,SideChunk);const auto source=block*BlockSize+head*SideChunk;if(source>bytes.size()||count>bytes.size()-source)throw MediaError(DiagnosticCode::HfeTrackData,{std::to_string(cylinder),std::to_string(head)});std::copy_n(bytes.begin()+std::ptrdiff_t(source),count,packed.begin()+std::ptrdiff_t(destination));remaining-=count;destination+=count;++block;}
   image.tracks.push_back(v3?decodeV3(cylinder,head,packed,std::uint32_t(ticks)):uniformTrack(cylinder,head,unpack(packed),std::uint32_t(ticks)));}}
 image.writeProtected=bytes[WriteAllowedOffset]==WriteForbidden;checkCancellation(token);return image;
}
}
