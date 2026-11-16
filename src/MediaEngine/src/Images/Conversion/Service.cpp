#include "Service.h"
#include "../../Formats/Scp/Constants.h"
#include "../../Sources/DriveOperation.h"
#include "../../Primitives/Identifiers.h"
#include "Sequential/Constants.h"
#include <RetroMediaLab/MediaEngine/ConversionConstants.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaEngine/DiagnosticKeys.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace {
std::filesystem::path utf8Path(std::string_view value){return std::filesystem::path(std::u8string(value.begin(),value.end()));}
std::string utf8(const std::filesystem::path& path){const auto value=path.u8string();return {value.begin(),value.end()};}
MessageInfo message(const MediaFileSystems::MessageRecipe& value){return {value.resourceKey,value.arguments,value.translationDomain};}
void appendUnique(std::vector<MessageInfo>& values,MessageInfo value){if(std::none_of(values.begin(),values.end(),[&](const auto& existing){return existing.resourceKey==value.resourceKey&&existing.arguments==value.arguments&&existing.translationDomain==value.translationDomain;}))values.push_back(std::move(value));}
}
MediaConversionRequest::MediaConversionRequest(std::shared_ptr<const MediaDocument> document,std::filesystem::path path,std::string target,std::map<std::string,std::string> parameters,MediaConversionProgressCallback callback)
 :source(std::move(document)),output(std::move(path)),targetFormatId(std::move(target)),options(std::move(parameters)),progress(std::move(callback)){
 if(!source||Primitives::isBlankIdentifier(utf8(output))||Primitives::isBlankIdentifier(targetFormatId))throw MediaError(DiagnosticCode::ConversionRequest);
}
std::future<std::shared_ptr<const MediaFileSystems::SectorImage>> MediaConversionService::readSectorSource(const std::filesystem::path& path,const SectorRead& scp,const SectorRead& image){return Primitives::asciiIdentifierEqualsIgnoreCase(utf8(path.extension()),Scp::Constants::Extension)?scp():image();}
MediaConversionService::MediaConversionService(RepresentationConverterRegistry converters,WriterRegistry writers,MediaImageWritingService writing):converters_(std::move(converters)),writers_(std::move(writers)),writing_(std::move(writing)){}
std::vector<MediaConversionDestination> MediaConversionService::availableDestinations(const MediaDocument& document)const{
 std::vector<MediaConversionDestination> result;
 for(const auto& writer:writers_.writers())for(const auto& format:writer->capabilities().formatIds)for(const auto& extension:writer->capabilities().producedFileExtensions){
  if(!writer->supportsTarget(format,extension))continue;
  bool accepted=writer->canWrite(document,format,extension);
  if(!accepted)for(const auto kind:writer->capabilities().representationKinds)if(converters_.resolve(document,format,kind)){accepted=true;break;}
  MediaConversionDestination destination{format,extension,writer->capabilities().id,writer->capabilities().producesMultipleFiles};
  if(accepted&&std::find(result.begin(),result.end(),destination)==result.end())result.push_back(std::move(destination));
 }
 std::stable_sort(result.begin(),result.end(),[](const auto& a,const auto& b){const auto format=QString::compare(Primitives::utf8Identifier(a.formatId),Primitives::utf8Identifier(b.formatId),Qt::CaseInsensitive);return format?format<0:QString::compare(Primitives::utf8Identifier(a.extension),Primitives::utf8Identifier(b.extension),Qt::CaseInsensitive)<0;});return result;
}
std::future<MediaConversionResult> MediaConversionService::convertAsync(MediaConversionRequest request,std::stop_token token)const{
 return std::async(std::launch::async,[service=*this,request=std::move(request),token]{
  checkCancellation(token);if(!request.source||request.output.empty()||Primitives::isBlankIdentifier(request.targetFormatId))throw MediaError(DiagnosticCode::ConversionRequest);
  DriveOperationScope activity(request.source->mediaSource()->options().drive.get());
  if(request.progress)request.progress({{std::string(DiagnosticKeys::ConversionReading),{utf8(utf8Path(request.source->mediaSource()->primaryPath()).filename())},{}},ConversionPlanningConstants::StartPercent,request.source->mediaKind()});
  const auto extension=utf8(request.output.extension());if(Primitives::isBlankIdentifier(extension))throw MediaError(DiagnosticCode::ConversionOutputExtension);
  auto document=request.source;
  if(request.targetFormatId==SequentialConversionConstants::RawFormat&&!request.options.empty()){
   auto options=document->mediaSource()->options();options.parameters=request.options;
   document=std::make_shared<MediaDocument>(std::make_shared<MediaSource>(std::move(options)),document->formatId(),document->mediaKind(),document->representationOwner(),std::vector<MediaFileSystems::VolumeDescriptor>(document->volumes().begin(),document->volumes().end()),RepresentationMessages(document->messages().begin(),document->messages().end()),document->metadata(),std::vector<std::string>(document->diagnostics().begin(),document->diagnostics().end()));
  }
  MediaConversionResult result;for(const auto& value:document->diagnostics())appendUnique(result.diagnostics,{value,{},{}});
  if(!service.writers_.canWrite(*document,request.targetFormatId,extension)){
   std::vector<RepresentationKind> kinds;
   for(const auto& writer:service.writers_.findTargetCandidates(request.targetFormatId,extension))for(const auto kind:writer->capabilities().representationKinds)if(std::find(kinds.begin(),kinds.end(),kind)==kinds.end())kinds.push_back(kind);
   if(kinds.empty())throw MediaError(DiagnosticCode::ConversionWriterMissing,{request.targetFormatId,extension});
   std::optional<RepresentationConversionResult> conversion;
   RepresentationProgress progress;if(request.progress)progress=[callback=request.progress](const auto& value){callback({message(value.message),value.percent,value.mediaKind});};
   for(const auto kind:kinds){
    checkCancellation(token);const auto converter=service.converters_.resolve(*document,request.targetFormatId,kind);if(!converter)continue;
    auto candidate=converter->convert(document,request.targetFormatId,kind,request.options,progress,token);
    if(service.writers_.canWrite(*candidate.document(),request.targetFormatId,extension)){conversion=std::move(candidate);break;}
   }
   if(!conversion)throw MediaError(DiagnosticCode::ConversionRepresentationMissing,{request.targetFormatId,extension});
   document=conversion->document();for(const auto& value:conversion->diagnostics())appendUnique(result.diagnostics,message(value));
   for(const auto& value:document->diagnostics())appendUnique(result.diagnostics,{value,{},{}});
   for(const auto& value:conversion->losses())appendUnique(result.losses,message(value));
  }
  result.producedFiles=service.writing_.writeAsync(document,request.output,request.targetFormatId,token).get();
  if(request.progress)request.progress({{std::string(DiagnosticKeys::ConversionComplete),{request.targetFormatId},{}},ConversionPlanningConstants::CompletePercent,document->mediaKind()});
  activity.finish();return result;
 });
}
}
