#include "Optical.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/ConversionConstants.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaEngine/DiagnosticKeys.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace {
std::string utf8(const std::filesystem::path& path){const auto value=path.u8string();return {value.begin(),value.end()};}
}
OpticalImageConversionService::OpticalImageConversionService(MediaImageReadingService reading,WriterRegistry writers,MediaImageWritingService writing)
 :reading_(std::move(reading)),writers_(std::move(writers)),writing_(std::move(writing)){}
std::vector<MediaConversionDestination> OpticalImageConversionService::availableDestinations(const MediaDocument& document)const{
 std::vector<MediaConversionDestination> result;if(document.mediaKind()!=MediaKind::Optical)return result;
 for(const auto& writer:writers_.writers())for(const auto& format:writer->capabilities().formatIds)for(const auto& extension:writer->capabilities().producedFileExtensions){
  if(!writer->canWrite(document,format,extension))continue;
  MediaConversionDestination value{format,extension,writer->capabilities().id,writer->capabilities().producesMultipleFiles};
  if(std::find(result.begin(),result.end(),value)==result.end())result.push_back(std::move(value));
 }
 std::stable_sort(result.begin(),result.end(),[](const auto& a,const auto& b){const auto format=QString::compare(Primitives::utf8Identifier(a.formatId),Primitives::utf8Identifier(b.formatId),Qt::CaseInsensitive);return format?format<0:QString::compare(Primitives::utf8Identifier(a.extension),Primitives::utf8Identifier(b.extension),Qt::CaseInsensitive)<0;});return result;
}
std::future<std::vector<std::filesystem::path>> OpticalImageConversionService::convertAsync(std::filesystem::path source,std::filesystem::path output,std::string target,MediaConversionProgressCallback progress,std::stop_token token)const{
 return std::async(std::launch::async,[service=*this,source=std::move(source),output=std::move(output),target=std::move(target),progress=std::move(progress),token]{
  checkCancellation(token);
  if(Primitives::isBlankIdentifier(utf8(source))||Primitives::isBlankIdentifier(utf8(output))||Primitives::isBlankIdentifier(target))throw MediaError(DiagnosticCode::ConversionRequest);
  SourceOptions options;options.primaryPath=utf8(source);
  const auto document=service.reading_.readAsync(std::make_shared<MediaSource>(std::move(options)),token).get();
  if(document->mediaKind()!=MediaKind::Optical)throw MediaError(DiagnosticCode::ConversionRequest);
  const auto extension=utf8(output.extension());
  if(!service.writers_.resolve(*document,target,extension))throw MediaError(DiagnosticCode::ConversionRepresentationMissing,{target,extension});
  auto produced=service.writing_.writeAsync(document,output,target,token).get();
  if(progress)progress({{std::string(DiagnosticKeys::ConversionComplete),{target},{}},ConversionPlanningConstants::CompletePercent,document->mediaKind()});
  return produced;
 });
}
}
