#include "Atx.h"
#include "Constants.h"
#include "../Atari8Bit/Constants.h"
#include "../../Models/Sectors/SectorImages.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <map>
#include <set>
namespace RetroMediaLab::MediaEngine::Atx {
namespace {
using namespace Constants;
std::uint16_t word(MediaFileSystems::Bytes b,std::size_t i){return std::uint16_t(b[i]|(std::uint16_t(b[i+1])<<BitsPerByte));}
std::uint32_t dword(MediaFileSystems::Bytes b,std::size_t i){std::uint32_t value=0;for(std::size_t byte=0;byte<DwordBytes;++byte)value|=std::uint32_t(b[i+byte])<<(byte*BitsPerByte);return value;}
void range(MediaFileSystems::Bytes b,std::size_t offset,std::size_t length,std::string_view field){if(offset>b.size()||length>b.size()-offset)throw MediaError(DiagnosticCode::AtxTruncated,{std::string(field)});}
std::size_t checkedOffset(std::uint64_t value,std::string_view field){if(value>MaxOffset)throw MediaError(DiagnosticCode::AtxLargeOffset,{std::string(field)});return std::size_t(value);}
std::size_t offset(MediaFileSystems::Bytes b,std::size_t at,std::string_view field){range(b,at,DwordBytes,field);return checkedOffset(dword(b,at),field);}
struct Candidate {std::uint8_t status;std::uint16_t position;std::vector<std::uint8_t> data;};
}
std::shared_ptr<const MediaFileSystems::SectorImage> read(MediaFileSystems::Bytes bytes,std::stop_token token){using namespace Constants;if(bytes.size()<FileHeader||dword(bytes,0)!=Signature)throw MediaError(DiagnosticCode::AtxHeader);auto trackOffset=offset(bytes,TrackDataOffset,TrackDataField);if(trackOffset<FileHeader||trackOffset>bytes.size()-TrackHeader)throw MediaError(DiagnosticCode::AtxTrackOffset);std::map<std::pair<std::uint32_t,std::uint32_t>,std::vector<Candidate>> candidates;std::set<std::size_t> visited;
 while(trackOffset>0){checkCancellation(token);if(!visited.insert(trackOffset).second)throw MediaError(DiagnosticCode::AtxTrackCycle);range(bytes,trackOffset,TrackHeader,HeaderField);const auto relative=dword(bytes,trackOffset+NextOffset);const auto end=relative==0?bytes.size():checkedOffset(std::uint64_t(trackOffset)+checkedOffset(relative,NextField),NextField);if(end<=trackOffset||end>bytes.size())throw MediaError(DiagnosticCode::AtxTrackBounds);const auto type=word(bytes,trackOffset+TypeOffset);const auto number=bytes[trackOffset+NumberOffset];const auto count=word(bytes,trackOffset+CountOffset);if(number>=Tracks)throw MediaError(DiagnosticCode::AtxTrackNumber,{std::to_string(number)});if(count>MaxRecords)throw MediaError(DiagnosticCode::AtxSectorCount,{std::to_string(number)});
  if(type==0&&count>0){const auto list=checkedOffset(std::uint64_t(trackOffset)+offset(bytes,trackOffset+RelativeDataOffset,ListField),ListField);const auto length=ListHeader+count*SectorHeader;range(bytes,list,length,ListField);if(list+length>end)throw MediaError(DiagnosticCode::AtxListBounds,{std::to_string(number)});
   for(std::uint32_t index=0;index<count;++index){const auto header=list+ListHeader+index*SectorHeader;const auto sector=bytes[header+SectorNumberOffset];if(sector<Atari8Bit::Constants::FirstSector||sector>MaxSectors)throw MediaError(DiagnosticCode::AtxSectorNumber,{std::to_string(number),std::to_string(sector)});const auto status=bytes[header+StatusOffset];const auto position=word(bytes,header+PositionOffset);if(status&MissingData)continue;const auto data=checkedOffset(std::uint64_t(trackOffset)+offset(bytes,header+DataOffset,DataField),DataField);range(bytes,data,SectorSize,DataField);if(data+SectorSize>end)throw MediaError(DiagnosticCode::AtxDataBounds,{std::to_string(number),std::to_string(sector)});candidates[{number,sector}].push_back({status,position,{bytes.begin()+std::ptrdiff_t(data),bytes.begin()+std::ptrdiff_t(data+SectorSize)}});}}
  if(relative==0||end==bytes.size())break;trackOffset=end;
 }
 if(candidates.empty())throw MediaError(DiagnosticCode::AtxNoSectors);const auto maximum=std::max_element(candidates.begin(),candidates.end(),[](const auto&a,const auto&b){return a.first.second<b.first.second;});const auto sectors=std::max(StandardSectors,maximum->first.second);std::vector<MediaFileSystems::SectorBlock> blocks;for(const auto&[address,list]:candidates){const auto selected=std::min_element(list.begin(),list.end(),[](const auto&a,const auto&b){return std::pair{a.status!=0,a.position}<std::pair{b.status!=0,b.position};});MediaFileSystems::SectorBlock b;b.logicalBlock=address.first*sectors+address.second-Atari8Bit::Constants::FirstSector;b.address={address.first,Atari8Bit::Constants::FirstHead,address.second};b.data=selected->data;b.integrityValid=selected->status==0;b.diagnosticCode=selected->status;blocks.push_back(std::move(b));}MediaFileSystems::SectorImageOptions options;options.capacity=std::uint64_t(Tracks)*sectors*SectorSize;options.logicalBlockCount=Tracks*sectors;return sparseSectorImage({std::string(MediaFileSystems::Formats::AtariAtx),SectorSize,Tracks,Atari8Bit::Constants::Heads,sectors},std::move(blocks),options);
}
}
