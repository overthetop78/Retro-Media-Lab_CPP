#include "Decoder.h"
#include "../Compatibility.h"
#include "../Tzx/SignalDecoder.h"
#include "../Fsk/Serial.h"
#include "../../../Formats/Tape/MsxCas/Constants.h"
#include "../../../Formats/Tape/Tzx/Constants.h"
#include "../../../Formats/Tape/Common/Pcm.h"
#include "../../../Models/Blocks/Internal.h"
#include "../../../Images/RecognitionContext.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaEngine/DiagnosticKeys.h>
#include <algorithm>
#include <set>
#include <charconv>
namespace RetroMediaLab::MediaEngine {
namespace {
using namespace MsxCas::Constants;
using Segments=std::vector<SequentialSegment>;
const Segments& segments(const SequentialImage& image){static const Segments empty;return image.segments()?*image.segments():empty;}
std::optional<std::string> type(std::span<const std::uint8_t> data){
    if(data.size()<FileTypeMarkerLength||!std::all_of(data.begin(),data.begin()+FileTypeMarkerLength,[&](auto value){return value==data.front();}))return {};
    switch(data.front()){case BinaryFileMarker:return std::string(BinaryType);case TokenizedBasicFileMarker:return std::string(BasicType);case AsciiFileMarker:return std::string(AsciiType);default:return {};}
}
SequentialDecodedBlock create(std::int64_t position,std::vector<std::uint8_t> data,std::vector<std::int64_t> sources,std::string_view kind,const std::optional<std::string>& fileType,std::optional<bool> integrity){
    std::map<std::string,std::string> metadata{{std::string(SourceKindMetadataKey),std::string(kind)}};
    if(integrity)metadata[std::string(SerialFramesValidMetadataKey)]=*integrity?TrueValue:FalseValue;if(fileType)metadata[std::string(FileTypeMetadataKey)]=*fileType;
    return {position,std::move(data),std::move(sources),integrity,std::move(metadata)};
}
SequentialDecodeResult result(const SequentialImage& image,std::vector<SequentialDecodedBlock> blocks,const std::set<std::int64_t>& consumed,std::string_view empty,std::vector<MediaFileSystems::MessageRecipe> messages={}){
    std::vector<SequentialSegment> undecoded;for(const auto& segment:segments(image))if(!consumed.contains(segment.fields().position))undecoded.push_back(segment);
    const auto valid=std::count_if(blocks.begin(),blocks.end(),[](const auto& block){return block.integrityValid()!=false;});const auto confidence=blocks.empty()?0:valid/double(blocks.size());
    if(blocks.empty())messages.insert(messages.begin(),{std::string(empty),{},{}});return {std::string(DecoderId),confidence,std::move(blocks),std::move(undecoded),{},std::move(messages)};
}
SequentialDecodeResult cas(const SequentialImage& image,std::stop_token token){
    std::vector<const SequentialSegmentFields*> sorted;for(const auto& segment:segments(image))sorted.push_back(&segment.fields());std::stable_sort(sorted.begin(),sorted.end(),[](const auto* a,const auto* b){return a->position<b->position;});
    std::vector<SequentialDecodedBlock> blocks;std::set<std::int64_t> consumed;std::optional<std::string> currentType;
    for(const auto* value:sorted){
        checkCancellation(token);if(value->kind==SequentialSegmentKind::TapeMark&&value->metadata.contains(std::string(SeparatorMetadataKey))){consumed.insert(value->position);continue;}
        if(value->kind!=SequentialSegmentKind::DataBlock)continue;auto data=readStoredRange(value->dataRange?&*value->dataRange:nullptr,token);if(!data)continue;
        const auto found=value->metadata.find(std::string(FileTypeMetadataKey));if(found!=value->metadata.end())currentType=found->second;if(!currentType)continue;
        blocks.push_back(create(blocks.size(),std::move(*data),{value->position},CasSource,currentType,true));consumed.insert(value->position);
    }
    return result(image,std::move(blocks),consumed,DiagnosticKeys::MsxTapeNoCas);
}
std::int64_t sourcePosition(const SequentialSegmentFields& value){
    const auto found=value.metadata.find(std::string(Tzx::Constants::SourcePositionKey));if(found==value.metadata.end())return value.position;
    const auto& text=found->second;std::int64_t position{};const auto parsed=std::from_chars(text.data(),text.data()+text.size(),position,Tzx::Constants::DecimalRadix);
    return parsed.ec==std::errc{}&&parsed.ptr==text.data()+text.size()&&position>=0?position:value.position;
}
SequentialDecodeResult tsx(const MediaDocument& document,const SequentialImage& image,std::stop_token token){
    const auto signal=TzxSignalDecoder{}.decodeAsync(std::make_shared<MediaDocument>(document),token).get();std::vector<SequentialDecodedBlock> blocks;std::set<std::int64_t> consumed;
    for(const auto& segment:segments(signal.timeline())){
        checkCancellation(token);const auto& value=segment.fields();if(value.kind!=SequentialSegmentKind::DataBlock)continue;
        auto data=readStoredRange(value.dataRange?&*value.dataRange:nullptr,token);if(!data)continue;const auto fileType=type(*data);const auto position=sourcePosition(value);
        blocks.push_back(create(blocks.size(),std::move(*data),{position},TsxSource,fileType,fileType?std::optional(true):std::nullopt));consumed.insert(position);
    }
    return result(image,std::move(blocks),consumed,DiagnosticKeys::MsxTapeNoTsx,{signal.messages().begin(),signal.messages().end()});
}
SequentialDecodeResult wave(const MediaDocument& document,const SequentialImage& image,std::stop_token token){
    int channels{},rate{},bits{},align{};if(!TapePcm::tryProfile(document.metadata(),channels,rate,bits,align))return {std::string(DecoderId),0,{},segments(image),{},{{std::string(DiagnosticKeys::MsxTapePcm),{},{}}}};
    std::vector<const SequentialSegmentFields*> sorted;for(const auto& segment:segments(image))if(segment.fields().kind==SequentialSegmentKind::Samples&&segment.fields().channelNumber==0)sorted.push_back(&segment.fields());
    std::stable_sort(sorted.begin(),sorted.end(),[](const auto* a,const auto* b){return a->position<b->position;});std::vector<std::uint8_t> bytes;std::set<std::int64_t> consumed;std::int64_t valid=0,tested=0;
    for(const auto* value:sorted){
        checkCancellation(token);const auto data=readStoredRange(value->dataRange?&*value->dataRange:nullptr,token);if(!data)continue;
        const auto samples=TapePcm::extractChannel(*data,channels,bits,align,0);const auto decoded=Fsk::decode(samples,rate,StandardBaudRate,ZeroFrequency,OneFrequency,DataBitsPerFrame,StopBitsPerFrame,MinimumSamplesPerBit,token);
        if(decoded.bytes().size()>static_cast<std::size_t>(TapePcm::Constants::MaximumBuffer)-bytes.size())throw MediaError(DiagnosticCode::MsxTapeDecodeLimit);
        bytes.insert(bytes.end(),decoded.bytes().begin(),decoded.bytes().end());valid+=decoded.validFrames();tested+=decoded.testedFrames();if(!decoded.bytes().empty())consumed.insert(value->position);
    }
    std::vector<SequentialDecodedBlock> blocks;if(!bytes.empty()){const auto fileType=type(bytes);blocks.push_back(create(0,std::move(bytes),{consumed.begin(),consumed.end()},FskSource,fileType,tested>0&&valid==tested));}
    std::vector<SequentialSegment> undecoded;for(const auto& segment:segments(image))if(!consumed.contains(segment.fields().position))undecoded.push_back(segment);
    std::vector<MediaFileSystems::MessageRecipe> messages;if(blocks.empty())messages.push_back({std::string(DiagnosticKeys::MsxTapeNoFsk),{},{}});
    return {std::string(DecoderId),tested?valid/double(tested):0,std::move(blocks),std::move(undecoded),{},std::move(messages)};
}
}
std::string_view MsxTapeDecoder::id()const noexcept{return DecoderId;}
std::span<const std::string> MsxTapeDecoder::formatIds()const{static const std::vector<std::string> ids{std::string(FormatId),std::string(Tzx::Constants::FormatId),std::string(WaveFormat),std::string(CswFormat),std::string(RawFormat)};return ids;}
std::span<const std::string> MsxTapeDecoder::machineIds()const{static const std::vector<std::string> ids{std::string(MachineId)};return ids;}
bool MsxTapeDecoder::canDecode(const MediaDocument& document,std::optional<std::string> machine)const{
    if(!canDecodeTape(document,machine,formatIds(),machineIds()))return false;if(!sameIdentifier(document.formatId(),Tzx::Constants::FormatId)||machine)return true;
    const auto found=document.metadata().find(std::string(Tzx::Constants::ContextKey));return found!=document.metadata().end()&&sameIdentifier(found->second,Tzx::Constants::TsxExtension);
}
std::future<SequentialDecodeResult> MsxTapeDecoder::decodeAsync(const MediaDocument& document,std::optional<std::string> machine,std::stop_token token)const{return std::async(std::launch::deferred,[&document,machine=std::move(machine),token]{
    checkCancellation(token);if(!MsxTapeDecoder{}.canDecode(document,machine))throw MediaError(DiagnosticCode::MsxTapeIncompatible);const auto& image=dynamic_cast<const SequentialImage&>(document.representation());
    if(sameIdentifier(document.formatId(),FormatId))return cas(image,token);if(sameIdentifier(document.formatId(),Tzx::Constants::FormatId))return tsx(document,image,token);return wave(document,image,token);
});}
}
