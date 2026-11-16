#include "Scp.h"
#include "../../Sources/RandomAccessSources.h"
#include <RetroMediaLab/MediaEngine/ScpCaptureInfo.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <set>
#include <limits>
namespace RetroMediaLab::MediaEngine {
std::future<ScpCaptureInfo> readScpCaptureInfoAsync(std::filesystem::path path,std::stop_token token){
 return std::async(std::launch::async,[path=std::move(path),token]{
  using namespace Scp::Constants;checkCancellation(token);const auto source=fileSource(path);
  if(source->length()<FullHeaderBytes)throw MediaError(DiagnosticCode::ScpTableSection,{std::to_string(0),std::to_string(FullHeaderBytes)});
  std::vector<std::uint8_t> table(FullHeaderBytes);source->readExactly(0,table,token);ScpCaptureInfo result;result.header=Scp::readHeader(table);result.fileSize=source->length();
  std::set<std::int32_t> cylinders,heads;
  for(std::uint32_t slot=result.header.startTrack;slot<=result.header.endTrack;++slot){
   checkCancellation(token);const auto offset=HeaderBytes+slot*FieldBytes;std::uint32_t trackOffset=0;
   for(std::uint32_t byte=0;byte<FieldBytes;++byte)trackOffset|=std::uint32_t(table[offset+byte])<<(byte*std::numeric_limits<std::uint8_t>::digits);
   if(!trackOffset)continue;++result.capturedTracks;const auto address=Scp::trackAddress(std::int32_t(slot));cylinders.insert(address.first);heads.insert(address.second);
  }
  std::vector<std::uint8_t> buffer(CaptureInfoBufferSize);std::uint32_t checksum=0;
  for(std::uint64_t offset=HeaderBytes;offset<source->length();){checkCancellation(token);const auto count=std::size_t(std::min(std::uint64_t(buffer.size()),source->length()-offset));const auto bytes=std::span(buffer).first(count);source->readExactly(offset,bytes,token);checksum=Scp::updateChecksum(checksum,bytes);offset+=count;}
  result.missingTracks=std::max(0,result.header.trackCount()-result.capturedTracks);result.cylinders=std::int32_t(cylinders.size());result.sides=std::int32_t(heads.size());result.checksumValid=Scp::isChecksumValid(result.header.checksum,result.header.flags,checksum);checkCancellation(token);return result;
 });
}
}
