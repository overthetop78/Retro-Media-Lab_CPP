#include "Service.h"
#include "Constants.h"
#include "../../../Formats/Tape/Common/Pcm.h"
#include "../../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaEngine/DiagnosticKeys.h>
#include <algorithm>

namespace RetroMediaLab::MediaEngine {
namespace {
using namespace SequentialConversionConstants;
bool equal(std::string_view a,std::string_view b){return Primitives::ordinalIdentifierEqualsIgnoreCase(a,b);}
void uniqueDiagnostics(std::vector<std::string>& values){std::vector<std::string> result;for(auto& value:values)if(std::find(result.begin(),result.end(),value)==result.end())result.push_back(std::move(value));values=std::move(result);}
}
SequentialMediaConversionPlan::SequentialMediaConversionPlan(std::shared_ptr<const MediaDocument> target,std::vector<std::string> diagnostics,ConversionMessages losses)
    :target_(std::move(target)),diagnostics_(std::move(diagnostics)),losses_(std::move(losses)){
    if(!target_||std::any_of(diagnostics_.begin(),diagnostics_.end(),[](const auto& value){return Primitives::isBlankIdentifier(value);})||std::any_of(losses_.begin(),losses_.end(),[](const auto& value){return Primitives::isBlankIdentifier(value.resourceKey);}))throw MediaError(DiagnosticCode::SequentialConversionPlan);
}
std::optional<std::string> SequentialMediaConversionService::resolveMachine(std::shared_ptr<const MediaDocument> source,std::optional<std::string> requested,std::stop_token token)const {
    if(requested&&!Primitives::isBlankIdentifier(*requested))return requested;
    std::vector<std::string> machines;
    for(const auto& decoder:codecs_.decoders().findCompatible(*source,{},token))for(const auto& machine:decoder->machineIds())
        if(std::none_of(machines.begin(),machines.end(),[&](const auto& existing){return equal(existing,machine);}))machines.push_back(machine);
    if(machines.size()==1)return machines.front();
    std::optional<std::string> best;std::size_t maximum=0;bool tied=false;
    for(const auto& machine:machines){
        checkCancellation(token);const auto decoded=codecs_.decoders().decodeAsync(source,machine,token).get();
        const auto valid=static_cast<std::size_t>(std::count_if(decoded.blocks().begin(),decoded.blocks().end(),[](const auto& block){return block.integrityValid()==true;}));
        if(valid>maximum){maximum=valid;best=machine;tied=false;}else if(valid>0&&valid==maximum)tied=true;
    }
    return tied?std::nullopt:best;
}
std::optional<SequentialEncodeRequest> SequentialMediaConversionService::selectRequest(const SequentialDecodeResult& decoded,std::string target,std::string machine,const std::map<std::string,std::string>& parameters,std::stop_token token)const {
    std::vector<SequentialDecodedBlock> blocks(decoded.blocks().begin(),decoded.blocks().end());
    SequentialEncodeRequest preserving(target,machine,blocks,{decoded.undecodedSegments().begin(),decoded.undecodedSegments().end()},parameters);
    if(codecs_.encoders().findCompatible(preserving,token).size()==1)return preserving;
    SequentialEncodeRequest lossy(std::move(target),std::move(machine),std::move(blocks),{},parameters);
    if(codecs_.encoders().findCompatible(lossy,token).size()==1)return lossy;
    return {};
}
SequentialMediaConversionPlan SequentialMediaConversionService::plan(std::shared_ptr<const MediaDocument> source,std::string target,std::optional<std::string> machine,std::map<std::string,std::string> parameters,std::stop_token token)const {
    checkCancellation(token);
    if(!source||Primitives::isBlankIdentifier(target))throw MediaError(DiagnosticCode::SequentialConversionPlan);
    if(source->mediaKind()!=MediaKind::Tape||!dynamic_cast<const SequentialImage*>(&source->representation()))throw MediaError(DiagnosticCode::SequentialConversionSource);
    const auto resolved=resolveMachine(source,std::move(machine),token);
    if(!resolved)throw MediaError(DiagnosticCode::SequentialConversionMachine);
    const auto decoded=codecs_.decoders().decodeAsync(source,*resolved,token).get();
    parameters.try_emplace(std::string(TapePcm::Constants::RateKey),std::to_string(TapePcm::Constants::DefaultRate));
    const auto request=selectRequest(decoded,target,*resolved,parameters,token);
    if(!request)throw MediaError(DiagnosticCode::SequentialConversionEncoder,{source->formatId(),target,*resolved});
    const auto encoded=codecs_.encoders().encodeAsync(*request,token).get();
    ConversionMessages losses;
    if(!decoded.undecodedSegments().empty()&&request->retainedSegments().empty())losses.push_back({std::string(DiagnosticKeys::SequentialConversionSegments),{std::to_string(decoded.undecodedSegments().size())},{}});
    bool reconstructed=false,dataBlocks=false;
    auto metadata=parameters;
    if(encoded->segments())for(const auto& segment:*encoded->segments()){
        const auto& fields=segment.fields();const auto found=fields.metadata.find(std::string(ReconstructedKey));
        reconstructed|=found!=fields.metadata.end()&&equal(found->second,TrueValue);
        dataBlocks|=fields.kind==SequentialSegmentKind::DataBlock;
    }
    int channels{},rate{},bits{},align{};
    if(reconstructed||(target==UefFormat&&dataBlocks&&TapePcm::tryProfile(source->metadata(),channels,rate,bits,align)))losses.push_back({std::string(DiagnosticKeys::SequentialConversionTiming),{},{}});
    if(target==AtomFormat&&source->formatId()!=AtomFormat)losses.push_back({std::string(DiagnosticKeys::SequentialConversionAtom),{},{}});
    if(encoded->segments())for(const auto& segment:*encoded->segments())if(segment.fields().kind==SequentialSegmentKind::Samples){
        metadata[std::string(TapePcm::Constants::ChannelsKey)]=MonoValue;
        for(const auto key:{TapePcm::Constants::RateKey,TapePcm::Constants::BitsKey,TapePcm::Constants::AlignKey}){
            const auto found=segment.fields().metadata.find(std::string(key));if(found!=segment.fields().metadata.end())metadata[found->first]=found->second;
        }break;
    }
    auto options=source->mediaSource()->options();options.parameters=parameters;
    std::vector<std::string> diagnostics(decoded.diagnostics().begin(),decoded.diagnostics().end());uniqueDiagnostics(diagnostics);
    auto document=std::make_shared<MediaDocument>(std::make_shared<MediaSource>(std::move(options)),std::move(target),MediaKind::Tape,encoded,std::vector<MediaFileSystems::VolumeDescriptor>{},ConversionMessages(decoded.messages().begin(),decoded.messages().end()),std::move(metadata),diagnostics);
    return {std::move(document),std::move(diagnostics),std::move(losses)};
}
}
