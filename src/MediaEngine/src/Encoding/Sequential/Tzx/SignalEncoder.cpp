#include "SignalEncoder.h"
#include "../../../Formats/Tape/Tzx/Metadata.h"
#include "../../../Formats/Tape/Tzx/Constants.h"
#include "../../../Formats/Tape/Common/Pcm.h"
#include "../../../Sources/RandomAccessSources.h"
#include "../../../Images/RecognitionContext.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <algorithm>
#include <charconv>
#include <limits>
namespace RetroMediaLab::MediaEngine {
namespace {
using namespace Tzx::Constants;
bool parameter(const SequentialSegmentFields& value,std::string_view key,std::uint16_t& number,bool optional=false){
    const auto found=value.metadata.find(std::string(key));if(found==value.metadata.end()){number=1;return optional;}unsigned parsed{};
    const auto& text=found->second;const auto result=std::from_chars(text.data(),text.data()+text.size(),parsed,DecimalRadix);
    if(result.ec!=std::errc{}||result.ptr!=text.data()+text.size()||!parsed||parsed>MaximumWord)return false;number=static_cast<std::uint16_t>(parsed);return true;
}
bool retain(const SequentialSegment& segment){
    const auto& value=segment.fields();
    if(value.metadata.contains(std::string(BlockIdMetadataKey))){
        std::uint8_t id{};if(!Tzx::tryBlockId(value,id))return false;
        return (!value.length&&!value.dataRange)||(value.dataRange&&value.dataRange->kind()==DataRangeKind::Stored&&value.dataRange->source()&&value.length==value.dataRange->length());
    }
    if(value.kind==SequentialSegmentKind::Pulse){std::uint16_t states{},count{};return parameter(value,PulseStatesKey,states)&&parameter(value,PulseCountKey,count,true);}
    return value.kind==SequentialSegmentKind::Silence&&value.duration&&value.duration->count()>=SequentialTicksPerSecond/MillisecondsPerSecond&&value.duration->count()<=std::int64_t(MaximumWord)*(SequentialTicksPerSecond/MillisecondsPerSecond);
}
SequentialSegment create(std::int64_t position,std::uint8_t id,std::vector<std::uint8_t> payload){
    SequentialSegmentFields value;value.position=position;value.kind=id==Pause?SequentialSegmentKind::Silence:id==PureTone||id==PulseSequence?SequentialSegmentKind::Pulse:SequentialSegmentKind::DataBlock;
    value.length=payload.size();value.dataRange=DataRange(0,payload.size(),DataRangeKind::Stored,memorySource(payload));
    value.metadata={{std::string(BlockIdMetadataKey),Tzx::blockIdText(id)},{std::string(PayloadKey),std::to_string(payload.size())},{std::string(KnownKey),std::string(TrueValue)}};return SequentialSegment(std::move(value));
}
}
std::string_view TzxSignalEncoder::id()const noexcept{return EncoderId;}
std::span<const std::string> TzxSignalEncoder::formatIds()const{static const std::vector<std::string> ids{std::string(FormatId)};return ids;}
std::span<const std::string> TzxSignalEncoder::machineIds()const{static const std::vector<std::string> ids{std::string(SpectrumMachine),std::string(MsxMachine)};return ids;}
bool TzxSignalEncoder::canEncode(const SequentialEncodeRequest& request)const{
    return sameIdentifier(request.targetFormatId(),FormatId)&&(sameIdentifier(request.machineId(),SpectrumMachine)||sameIdentifier(request.machineId(),MsxMachine))
        &&std::all_of(request.blocks().begin(),request.blocks().end(),[](const auto& block){return block.integrityValid()==true&&block.data().size()<=MaximumWord;})
        &&std::all_of(request.retainedSegments().begin(),request.retainedSegments().end(),retain);
}
std::future<std::shared_ptr<const SequentialImage>> TzxSignalEncoder::encodeAsync(SequentialEncodeRequest request,std::stop_token token)const{return std::async(std::launch::deferred,[request=std::move(request),token]{
    checkCancellation(token);if(!TzxSignalEncoder{}.canEncode(request))throw MediaError(DiagnosticCode::TzxSignalEncode);
    struct Item {std::int64_t position;const SequentialSegment* segment;const SequentialDecodedBlock* block;};std::vector<Item> ordered;
    for(const auto& segment:request.retainedSegments())ordered.push_back({segment.fields().position,&segment,nullptr});
    for(std::size_t index=0;index<request.blocks().size();++index){const auto& block=request.blocks()[index];const auto positions=block.sourcePositions();ordered.push_back({positions.empty()?std::numeric_limits<std::int64_t>::max()-static_cast<std::int64_t>(request.blocks().size()-index):*std::min_element(positions.begin(),positions.end()),nullptr,&block});}
    std::stable_sort(ordered.begin(),ordered.end(),[](const auto& a,const auto& b){return a.position!=b.position?a.position<b.position:bool(a.block)<bool(b.block);});
    std::vector<SequentialSegment> output;std::int64_t length=0;
    for(const auto& item:ordered){
        checkCancellation(token);
        if(item.block){
            const auto bytes=item.block->data();std::vector<std::uint8_t> payload(StandardHeaderSize+bytes.size());qToLittleEndian(StandardPauseMilliseconds,payload.data());qToLittleEndian(static_cast<std::uint16_t>(bytes.size()),payload.data()+StandardLengthOffset);std::copy(bytes.begin(),bytes.end(),payload.begin()+StandardHeaderSize);
            output.push_back(create(output.size(),StandardSpeedData,std::move(payload)));
        }else if(item.segment->fields().metadata.contains(std::string(BlockIdMetadataKey)))output.push_back(item.segment->atPosition(output.size()));
        else if(item.segment->fields().kind==SequentialSegmentKind::Pulse){
            std::uint16_t states{},count{};parameter(item.segment->fields(),PulseStatesKey,states);parameter(item.segment->fields(),PulseCountKey,count,true);
            std::vector<std::uint8_t> payload(DwordWidth);qToLittleEndian(states,payload.data());qToLittleEndian(count,payload.data()+WordWidth);output.push_back(create(output.size(),PureTone,std::move(payload)));
        }else{
            const auto milliseconds=TapePcm::roundEven(double(item.segment->fields().duration->count())/(SequentialTicksPerSecond/MillisecondsPerSecond));std::vector<std::uint8_t> payload(WordWidth);qToLittleEndian(static_cast<std::uint16_t>(milliseconds),payload.data());output.push_back(create(output.size(),Pause,std::move(payload)));
        }
        const auto size=output.back().fields().dataRange?output.back().fields().dataRange->length():0;if(size>std::numeric_limits<std::int64_t>::max()-length)throw MediaError(DiagnosticCode::TzxSignalLimit);length+=size;
    }
    return std::make_shared<const SequentialImage>(length,std::nullopt,std::move(output));
});}
}
