#include "Decoder.h"
#include "../Compatibility.h"
#include "../../../Formats/Tape/Common/PulseIntervals.h"
#include "../../../Formats/Tape/CommodoreTap/Protocol.h"
#include "../../../Formats/Tape/CommodoreTap/Profile.h"
#include "../../../Formats/Tape/CommodoreTap/Constants.h"
#include "../../../Formats/Tape/Common/Pcm.h"
#include "../../../Models/Blocks/Internal.h"
#include "../../../Images/RecognitionContext.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaEngine/DiagnosticKeys.h>
#include <algorithm>
#include <set>
#include <cmath>
namespace RetroMediaLab::MediaEngine {
namespace {
using namespace CommodoreTap::Constants;
using Segments=std::vector<SequentialSegment>;
const Segments& segments(const SequentialImage& image){static const Segments empty;return image.segments()?*image.segments():empty;}
void wavePulses(std::span<const std::int32_t> samples,int rate,std::int64_t source,std::vector<CommodoreTape::Pulse>& pulses,std::stop_token token){
    TapePcm::pulseIntervals(samples,rate,[&](auto interval){
        if(interval.silence){pulses.push_back({0,source});return;}
        const auto cycles=TapePcm::roundEven(interval.samples*double(DefaultC64PalClockRate)/rate);
        if(cycles>std::numeric_limits<int>::max())throw MediaError(DiagnosticCode::CommodoreTapeLimit);if(cycles>0)pulses.push_back({static_cast<int>(cycles),source});
    },token);
}
SequentialDecodeResult decode(const MediaDocument& document,const SequentialImage& image,std::stop_token token){
    const bool pcm=!sameIdentifier(document.formatId(),FormatId);int channels{},rate{},bits{},align{};
    if(pcm&&!TapePcm::tryProfile(document.metadata(),channels,rate,bits,align))return {std::string(DecoderId),0,{},segments(image),{},{{std::string(DiagnosticKeys::CommodoreTapePcm),{},{}}}};
    int clock{};if(!CommodoreTap::integer(document.metadata(),ClockRateMetadataKey,clock)||clock<=0)clock=DefaultC64PalClockRate;
    std::vector<const SequentialSegmentFields*> sorted;for(const auto& segment:segments(image))sorted.push_back(&segment.fields());std::stable_sort(sorted.begin(),sorted.end(),[](const auto* a,const auto* b){return a->position<b->position;});std::vector<CommodoreTape::Pulse> pulses;
    for(const auto* value:sorted){
        checkCancellation(token);
        if(pcm){if(value->kind!=SequentialSegmentKind::Samples||value->channelNumber!=0)continue;const auto data=readStoredRange(value->dataRange?&*value->dataRange:nullptr,token);if(!data)continue;const auto samples=TapePcm::extractChannel(*data,channels,bits,align,0);wavePulses(samples,rate,value->position,pulses,token);continue;}
        if(value->kind!=SequentialSegmentKind::Pulse)continue;int cycles{};
        if(!CommodoreTap::integer(value->metadata,CycleCountMetadataKey,cycles)||cycles<=0){if(!value->duration)continue;const auto rounded=TapePcm::roundEven(value->duration->count()*double(clock)/SequentialTicksPerSecond);if(!std::isfinite(rounded)||rounded>std::numeric_limits<int>::max())throw MediaError(DiagnosticCode::CommodoreTapeLimit);cycles=static_cast<int>(rounded);}
        pulses.push_back({cycles,value->position});
    }
    auto decoded=CommodoreTape::decode(pulses,token);std::vector<SequentialDecodedBlock> blocks;std::set<std::int64_t> consumed;
    for(auto& block:decoded){consumed.insert(block.sources.begin(),block.sources.end());const bool valid=block.parityValid;blocks.emplace_back(blocks.size(),std::move(block.data),std::move(block.sources),valid,std::map<std::string,std::string>{{std::string(SourceKindMetadataKey),std::string(pcm?WaveSource:TapSource)},{std::string(ParityValidMetadataKey),std::string(valid?TrueValue:FalseValue)},{std::string(ChecksumPresentMetadataKey),std::string(FalseValue)}});}
    std::vector<SequentialSegment> undecoded;for(const auto& segment:segments(image))if(!consumed.contains(segment.fields().position))undecoded.push_back(segment);
    const auto valid=std::count_if(blocks.begin(),blocks.end(),[](const auto& block){return block.integrityValid()==true;});const auto confidence=blocks.empty()?0:valid/double(blocks.size());std::vector<MediaFileSystems::MessageRecipe> messages;if(blocks.empty())messages.push_back({std::string(DiagnosticKeys::CommodoreTapeNoFrames),{},{}});
    return {std::string(DecoderId),confidence,std::move(blocks),std::move(undecoded),{},std::move(messages)};
}
}
std::string_view CommodoreTapeDecoder::id()const noexcept{return DecoderId;}
std::span<const std::string> CommodoreTapeDecoder::formatIds()const{static const std::vector<std::string> ids{std::string(FormatId),std::string(WaveFormat),std::string(CswFormat),std::string(RawFormat)};return ids;}
std::span<const std::string> CommodoreTapeDecoder::machineIds()const{static const std::vector<std::string> ids{std::string(MachineId)};return ids;}
bool CommodoreTapeDecoder::canDecode(const MediaDocument& document,std::optional<std::string> machine)const{return canDecodeTape(document,machine,formatIds(),machineIds());}
std::future<SequentialDecodeResult> CommodoreTapeDecoder::decodeAsync(const MediaDocument& document,std::optional<std::string> machine,std::stop_token token)const{return std::async(std::launch::deferred,[&document,machine=std::move(machine),token]{
    checkCancellation(token);if(!CommodoreTapeDecoder{}.canDecode(document,machine))throw MediaError(DiagnosticCode::CommodoreTapeIncompatible);return decode(document,dynamic_cast<const SequentialImage&>(document.representation()),token);
});}
}
