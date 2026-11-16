#include "Service.h"
#include "Constants.h"
#include "../../../Formats/Tape/Common/PcmConstants.h"
#include "../../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QString>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
SequentialMediaConversionService::SequentialMediaConversionService(ReaderRegistry readers,SequentialMediaComposition codecs,WriterRegistry writers)
    :SequentialMediaConversionService(MediaImageReadingService(std::move(readers)),std::move(codecs),writers,MediaImageWritingService(writers)){}
SequentialMediaConversionService::SequentialMediaConversionService(MediaImageReadingService reading,SequentialMediaComposition codecs,WriterRegistry writers,MediaImageWritingService writing)
    :reading_(std::move(reading)),codecs_(std::move(codecs)),writers_(std::move(writers)),writing_(std::move(writing)){}
std::vector<SequentialConversionDestination> SequentialMediaConversionService::availableDestinations(std::shared_ptr<const MediaDocument> source,std::optional<std::string> machine,std::map<std::string,std::string> parameters,std::stop_token token)const {
    checkCancellation(token);if(!source)throw MediaError(DiagnosticCode::SequentialConversionPlan);
    if(source->mediaKind()!=MediaKind::Tape||!dynamic_cast<const SequentialImage*>(&source->representation()))return {};
    const auto resolved=resolveMachine(source,std::move(machine),token);if(!resolved)return {};
    const auto decoded=codecs_.decoders().decodeAsync(source,*resolved,token).get();
    parameters.try_emplace(std::string(TapePcm::Constants::RateKey),std::to_string(TapePcm::Constants::DefaultRate));
    std::vector<SequentialConversionDestination> result;
    for(const auto& writer:writers_.writers()){
        const auto& caps=writer->capabilities();
        for(const auto& format:caps.formatIds){
            checkCancellation(token);if(!selectRequest(decoded,format,*resolved,parameters,token))continue;
            for(const auto& extension:caps.producedFileExtensions){SequentialConversionDestination value{format,extension,caps.id,caps.producesMultipleFiles};if(std::find(result.begin(),result.end(),value)==result.end())result.push_back(std::move(value));}
        }
    }
    std::stable_sort(result.begin(),result.end(),[](const auto& a,const auto& b){const auto format=QString::compare(QString::fromStdString(a.formatId),QString::fromStdString(b.formatId),Qt::CaseInsensitive);return format?format<0:QString::compare(QString::fromStdString(a.extension),QString::fromStdString(b.extension),Qt::CaseInsensitive)<0;});
    return result;
}
SequentialConversionResult SequentialMediaConversionService::convert(const std::filesystem::path& sourcePath,const std::filesystem::path& output,std::string target,std::optional<std::string> machine,std::map<std::string,std::string> parameters,bool acceptLosses,std::stop_token token)const {
    using namespace SequentialConversionConstants;
    checkCancellation(token);if(sourcePath.empty()||output.empty())throw MediaError(DiagnosticCode::SequentialConversionPlan);
    SourceOptions options;const auto utf8=sourcePath.u8string();options.primaryPath.assign(utf8.begin(),utf8.end());options.parameters.emplace();
    for(const auto& [key,value]:parameters)if(key.starts_with(SourcePrefix))options.parameters->emplace(key.substr(SourcePrefix.size()),value);
    if(Primitives::ordinalIdentifierEqualsIgnoreCase(sourcePath.extension().string(),RawExtension))options.requestedFormatId=RawFormat;
    auto source=reading_.readAsync(std::make_shared<MediaSource>(std::move(options)),token).get();
    const auto planned=plan(std::move(source),std::move(target),std::move(machine),std::move(parameters),token);
    if(!acceptLosses&&!planned.losses().empty())throw MediaError(DiagnosticCode::SequentialConversionLosses);
    const auto extension=output.extension().u8string();const auto writer=writers_.resolve(*planned.targetDocument(),planned.targetDocument()->formatId(),std::string(extension.begin(),extension.end()));
    if(!writer)throw MediaError(DiagnosticCode::SequentialConversionWriter,{planned.targetDocument()->formatId(),std::string(extension.begin(),extension.end())});
    auto files=writing_.writeAsync(planned.targetDocument(),output,planned.targetDocument()->formatId(),token).get();
    return {std::move(files),{planned.diagnostics().begin(),planned.diagnostics().end()},{planned.losses().begin(),planned.losses().end()}};
}
}
