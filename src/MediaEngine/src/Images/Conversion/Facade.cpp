#include <RetroMediaLab/MediaEngine/MediaConversion.h>
#include <RetroMediaLab/MediaEngine/SequentialConversion.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include "../../Composition/Runtime.h"
namespace RetroMediaLab::MediaEngine {
std::vector<MediaConversionDestination> availableConversionDestinations(std::shared_ptr<const MediaDocument> source){if(!source)throw MediaError(DiagnosticCode::ConversionRequest);return RuntimeDetails::defaultServices()->conversion.availableDestinations(*source);}
std::future<MediaConversionResult> convertMediaAsync(MediaConversionRequest request,std::stop_token token){return RuntimeDetails::defaultServices()->conversion.convertAsync(std::move(request),token);}
std::vector<MediaConversionDestination> availableOpticalConversionDestinations(std::shared_ptr<const MediaDocument> source){if(!source)throw MediaError(DiagnosticCode::ConversionRequest);return RuntimeDetails::defaultServices()->optical.availableDestinations(*source);}
std::future<std::vector<std::filesystem::path>> convertOpticalMediaAsync(std::filesystem::path source,std::filesystem::path output,std::string target,MediaConversionProgressCallback progress,std::stop_token token){return RuntimeDetails::defaultServices()->optical.convertAsync(std::move(source),std::move(output),std::move(target),std::move(progress),token);}
namespace {
std::vector<MessageInfo> diagnosticMessages(std::span<const std::string> values){std::vector<MessageInfo> result;for(const auto& value:values)result.push_back({value,{},{}});return result;}
std::vector<MessageInfo> lossMessages(std::span<const MediaFileSystems::MessageRecipe> values){std::vector<MessageInfo> result;for(const auto& value:values)result.push_back({value.resourceKey,value.arguments,value.translationDomain});return result;}
}
std::future<std::vector<MediaConversionDestination>> availableSequentialConversionDestinationsAsync(std::shared_ptr<const MediaDocument> source,std::optional<std::string> machine,std::map<std::string,std::string> parameters,std::stop_token token){
 return std::async(std::launch::async,[services=RuntimeDetails::defaultServices(),source=std::move(source),machine=std::move(machine),parameters=std::move(parameters),token]{
  std::vector<MediaConversionDestination> result;for(const auto& value:services->sequential.availableDestinations(source,machine,parameters,token))result.push_back({value.formatId,value.extension,value.writerId,value.producesMultipleFiles});return result;
 });
}
std::future<SequentialConversionPlan> planSequentialConversionAsync(std::shared_ptr<const MediaDocument> source,std::string target,std::optional<std::string> machine,std::map<std::string,std::string> parameters,std::stop_token token){
 return std::async(std::launch::async,[services=RuntimeDetails::defaultServices(),source=std::move(source),target=std::move(target),machine=std::move(machine),parameters=std::move(parameters),token]{
  const auto plan=services->sequential.plan(source,target,machine,parameters,token);return SequentialConversionPlan{plan.targetDocument(),diagnosticMessages(plan.diagnostics()),lossMessages(plan.losses())};
 });
}
std::future<MediaConversionResult> convertSequentialMediaAsync(std::filesystem::path source,std::filesystem::path output,std::string target,std::optional<std::string> machine,std::map<std::string,std::string> parameters,bool acceptLosses,std::stop_token token){
 return std::async(std::launch::async,[services=RuntimeDetails::defaultServices(),source=std::move(source),output=std::move(output),target=std::move(target),machine=std::move(machine),parameters=std::move(parameters),acceptLosses,token]{
  auto result=services->sequential.convert(source,output,target,machine,parameters,acceptLosses,token);return MediaConversionResult{std::move(result.producedFiles),diagnosticMessages(result.diagnostics),lossMessages(result.losses)};
 });
}
}
