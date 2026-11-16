#include "Expansion.h"
#include "../../../Formats/Tape/Tzx/Constants.h"
#include "../../../Formats/Tape/Tzx/Metadata.h"
#include "../../../Images/RecognitionContext.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaEngine/DiagnosticKeys.h>
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaEngine {
SequentialSignalDecodeResult::SequentialSignalDecodeResult(std::shared_ptr<const SequentialImage> timeline,std::vector<SequentialSegment> source,std::vector<MediaFileSystems::MessageRecipe> messages)
    :timeline_(std::move(timeline)),source_(std::move(source)),messages_(std::move(messages)){if(!timeline_)throw MediaError(DiagnosticCode::TzxSignalDocument);}

std::future<SequentialSignalDecodeResult> TzxSignalDecoder::decodeAsync(std::shared_ptr<const MediaDocument> document,std::stop_token token)const{return std::async(std::launch::deferred,[document=std::move(document),token]{
    using namespace Tzx::Constants;checkCancellation(token);const auto* image=document?dynamic_cast<const SequentialImage*>(&document->representation()):nullptr;
    if(!document||!sameIdentifier(document->formatId(),FormatId)||!image||!image->segments())throw MediaError(DiagnosticCode::TzxSignalDocument);
    std::vector<const SequentialSegmentFields*> sorted;for(const auto& segment:*image->segments())sorted.push_back(&segment.fields());
    std::stable_sort(sorted.begin(),sorted.end(),[](const auto* a,const auto* b){return a->position<b->position;});TzxSignal::Expansion expansion(token);std::vector<MediaFileSystems::MessageRecipe> messages;
    for(const auto* source:sorted){
        checkCancellation(token);std::uint8_t id{};
        if(!Tzx::tryBlockId(*source,id)){messages.push_back({std::string(DiagnosticKeys::TzxSignalId),{std::to_string(source->position)},{}});continue;}
        const auto* range=source->dataRange?&*source->dataRange:nullptr;
        if(range&&(!range->source()||range->length()>std::numeric_limits<std::int32_t>::max()))throw MediaError(DiagnosticCode::TzxSignalPayload);
        const bool supported=id==StandardSpeedData||id==TurboSpeedData||id==PureTone||id==PulseSequence||id==PureData||id==DirectRecording||id==Pause;
        if(!supported){messages.push_back({std::string(DiagnosticKeys::TzxSignalUnsupported),{Tzx::blockIdText(id),std::to_string(source->position)},{}});continue;}
        std::vector<std::uint8_t> payload(range?static_cast<std::size_t>(range->length()):0);if(range)range->source()->readExactly(range->sourceOffset(),payload,token);
        expansion.block(id,*source,payload);
    }
    return SequentialSignalDecodeResult(expansion.finish(),*image->segments(),std::move(messages));
});}
}
