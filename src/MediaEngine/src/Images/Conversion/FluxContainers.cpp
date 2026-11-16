#include "FluxContainers.h"
#include "../WriterRegistry.h"
#include "../../Formats/Hfe/Constants.h"
#include "../../Formats/Scp/ProtectedImage.h"
#include "../../Formats/Scp/Adapter.h"
#include "../../Formats/Scp/Writer.h"
#include "../../Formats/Scp/DiskTypeConstants.h"
#include "../../Models/Flux/Conversion.h"
#include "../../Sources/AtomicImageWriter.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/FluxConversion.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QFile>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace {
std::string utf8(const std::filesystem::path& path){const auto value=path.u8string();return {value.begin(),value.end()};}
std::string extension(const std::filesystem::path& path){return Primitives::utf8Identifier(utf8(path.extension())).toLower().toStdString();}
std::vector<std::uint8_t> bytes(const std::filesystem::path& path,std::stop_token token){
 checkCancellation(token);QFile file(QString::fromStdU16String(path.u16string()));if(!file.open(QIODevice::ReadOnly))throw MediaError(DiagnosticCode::AtomicOpen,{utf8(path)});
 const auto length=file.size();if(length<0||std::uint64_t(length)>Scp::Constants::MaximumLength)throw MediaError(DiagnosticCode::ScpOverflow);
 std::vector<std::uint8_t> result(static_cast<std::size_t>(length));
 for(std::size_t offset=0;offset<result.size();){checkCancellation(token);const auto count=std::min(result.size()-offset,std::size_t(Scp::Constants::ChunkBytes));const auto read=file.read(reinterpret_cast<char*>(result.data()+offset),qint64(count));if(read<=0)throw MediaError(DiagnosticCode::AtomicOpen,{utf8(path)});offset+=std::size_t(read);}return result;
}
void copy(const std::filesystem::path& output,const std::vector<std::uint8_t>& data,std::stop_token token){writeAtomicImage(output,[&](const ImageOutput& write,std::stop_token stop){checkCancellation(stop);write(data);},token);}
Scp::Image toScp(const Hfe::Image& source,std::stop_token token){
 using namespace Scp::Constants;Scp::Image result;
 for(const auto& track:source.tracks){
  checkCancellation(token);const auto duration=std::uint64_t(track.bits.size())*track.bitCellTicks;if(duration>MaximumInterval)throw MediaError(DiagnosticCode::ScpOverflow);
  auto intervals=Flux::toIntervals(track.bits,track.bitCellTicks);if(intervals.size()>MaximumInterval)throw MediaError(DiagnosticCode::ScpOverflow);
  Scp::Revolution revolution;revolution.flux={std::uint32_t(duration),std::move(intervals)};revolution.declaredFluxCount=std::uint32_t(revolution.flux.intervals.size());
  result.tracks.push_back({Scp::trackNumber(track.cylinder,track.head),track.cylinder,track.head,{std::move(revolution)}});
 }
 if(result.tracks.empty())throw MediaError(DiagnosticCode::HfeTracks);
 std::sort(result.tracks.begin(),result.tracks.end(),[](const auto& a,const auto& b){return a.trackNumber<b.trackNumber;});
 result.header={CaptureVersion,Scp::DiskTypes::Other720,CaptureRevolutions,result.tracks.front().trackNumber,result.tracks.back().trackNumber,std::uint8_t(IndexAlignedFlag|WritableFlag|ThirdPartyFlag),DefaultWidth,source.heads==1?Side0:BothHeads,CaptureResolution,MissingChecksum};
 result.checksumValid=true;result.fileSize=InitialFileSize;return result;
}
}
bool canConvertFluxContainer(const std::filesystem::path& source,std::string_view format,std::string_view targetExtension){
 const auto input=extension(source);const auto target=normalizeExtension(targetExtension);
 if(input==Scp::Constants::Extension)return target==Scp::Constants::Extension||target==Hfe::Constants::Extension;
 if(input!=Hfe::Constants::Extension)return false;
 return (target==Scp::Constants::Extension&&Primitives::ordinalIdentifierEqualsIgnoreCase(format,Scp::Constants::FormatId))||(target==Hfe::Constants::Extension&&Primitives::ordinalIdentifierEqualsIgnoreCase(format,Hfe::Constants::FormatId));
}
std::future<void> convertFluxContainerAsync(std::filesystem::path source,std::filesystem::path output,std::stop_token token){
 return std::async(std::launch::async,[source=std::move(source),output=std::move(output),token]{
  checkCancellation(token);const auto input=extension(source),target=extension(output);
  if((input!=Scp::Constants::Extension&&input!=Hfe::Constants::Extension)||(target!=Scp::Constants::Extension&&target!=Hfe::Constants::Extension))throw MediaError(DiagnosticCode::ConversionIncompatibleExtension,{target,input});
  const auto data=bytes(source,token);
  if(input==target){
   if(input==Scp::Constants::Extension){const auto expected=Scp::read(data,token);copy(output,data,token);const auto copied=bytes(output,token);FluxContainerConversion::validateParity(expected,Scp::read(copied,token),token);if(data!=copied)throw MediaError(DiagnosticCode::FluxContainerParity);}
   else {const auto expected=Hfe::read(data,token);copy(output,data,token);const auto copied=bytes(output,token);FluxContainerConversion::validateParity(expected,Hfe::read(copied,token),token);if(data!=copied)throw MediaError(DiagnosticCode::FluxContainerParity);}return;
  }
  if(input==Hfe::Constants::Extension){const auto expected=Hfe::read(data,token);Scp::write(toScp(expected,token),output,token);FluxContainerConversion::validateParity(expected,Scp::read(bytes(output,token),token),token);return;}
  const auto captured=Scp::read(data,token);if(captured.header.revolutions!=Scp::Constants::CaptureRevolutions||std::any_of(captured.tracks.begin(),captured.tracks.end(),[](const auto& t){return t.revolutions.size()!=Scp::Constants::CaptureRevolutions;}))throw MediaError(DiagnosticCode::HfeRevolutions);
  const auto expected=Hfe::fromProtected(Scp::protectedImage(captured),Scp::metadata(captured));Hfe::write(expected,output,token);FluxContainerConversion::validateParity(expected,Hfe::read(bytes(output,token),token),token);
 });
}
}
