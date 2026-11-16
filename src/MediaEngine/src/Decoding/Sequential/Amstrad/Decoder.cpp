#include "Decoder.h"
#include "Pulses.h"
#include "../Compatibility.h"
#include "../../../Formats/Tape/Amstrad/Constants.h"
#include "../../../Formats/Tape/Amstrad/Records.h"
#include "../../../Formats/Tape/Tzx/Constants.h"
#include "../../../Formats/Tape/Tzx/Metadata.h"
#include "../../../Formats/Tape/Common/Pcm.h"
#include "../../../Images/RecognitionContext.h"
#include "../../../Models/Blocks/Internal.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaEngine/DiagnosticKeys.h>
#include <QString>
#include <QtEndian>
#include <set>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace {
using namespace AmstradTape::Constants;
using Segments=std::vector<SequentialSegment>;
const Segments& segments(const SequentialImage& image){static const Segments empty;return image.segments()?*image.segments():empty;}
struct Header {int number,length;bool valid;std::vector<std::int64_t> sources;std::map<std::string,std::string> metadata;};
Header header(const AmstradTape::Record& record){
    const auto& data=record.data;auto end=FileNameLength;while(end>0&&(data[end-1]==Null||data[end-1]==Space))--end;
    const auto name=QString::fromLatin1(reinterpret_cast<const char*>(data.data()+FileNameOffset),end).toUtf8().toStdString();const auto word=[&](int offset){return qFromLittleEndian<std::uint16_t>(data.data()+offset);};
    return {data[BlockNumberOffset],word(DataLengthOffset),record.valid,record.sources,{{std::string(FileNameKey),name},{std::string(BlockNumberKey),std::to_string(data[BlockNumberOffset])},{std::string(LastBlockKey),std::string(data[LastBlockOffset]?TrueValue:FalseValue)},{std::string(FirstBlockKey),std::string(data[FirstBlockOffset]?TrueValue:FalseValue)},{std::string(FileTypeKey),std::to_string(data[FileTypeOffset])},{std::string(DataLengthKey),std::to_string(word(DataLengthOffset))},{std::string(DataLocationKey),std::to_string(word(DataLocationOffset))},{std::string(LogicalLengthKey),std::to_string(word(LogicalLengthOffset))},{std::string(EntryAddressKey),std::to_string(word(EntryAddressOffset))}}};
}
SequentialDecodeResult result(const SequentialImage& image,std::vector<AmstradTape::Record> records,std::stop_token token){
    std::vector<SequentialDecodedBlock> blocks;std::set<std::int64_t> consumed;std::vector<MediaFileSystems::MessageRecipe> messages;std::optional<Header> pending;
    for(const auto& record:records){
        checkCancellation(token);consumed.insert(record.sources.begin(),record.sources.end());
        if(record.sync==HeaderSync){if(record.data.size()<HeaderLength){messages.push_back({std::string(DiagnosticKeys::CpcTapeShortHeader),{},{}});pending.reset();continue;}pending=header(record);continue;}
        if(record.sync!=DataSync||!pending){messages.push_back({std::string(DiagnosticKeys::CpcTapeOrphanData),{},{}});continue;}
        const auto length=std::min<std::size_t>(pending->length,record.data.size());auto metadata=pending->metadata;metadata[std::string(SourceKindKey)]=SourceKind;metadata[std::string(HeaderIntegrityKey)]=pending->valid?TrueValue:FalseValue;metadata[std::string(DataIntegrityKey)]=record.valid?TrueValue:FalseValue;
        auto sources=pending->sources;for(const auto position:record.sources)if(std::find(sources.begin(),sources.end(),position)==sources.end())sources.push_back(position);
        blocks.emplace_back(blocks.size(),std::vector<std::uint8_t>(record.data.begin(),record.data.begin()+length),std::move(sources),pending->valid&&record.valid&&length==pending->length,std::move(metadata));
        if(length!=pending->length)messages.push_back({std::string(DiagnosticKeys::CpcTapeShortBlock),{std::to_string(pending->number),std::to_string(length),std::to_string(pending->length)},{}});pending.reset();
    }
    if(pending)messages.push_back({std::string(DiagnosticKeys::CpcTapeOrphanHeader),{},{}});if(blocks.empty())messages.push_back({std::string(DiagnosticKeys::CpcTapeNoBlocks),{},{}});
    std::vector<SequentialSegment> undecoded;for(const auto& segment:segments(image))if(!consumed.contains(segment.fields().position))undecoded.push_back(segment);const auto valid=std::count_if(blocks.begin(),blocks.end(),[](const auto& block){return block.integrityValid()==true;});const auto confidence=blocks.empty()?0:valid/double(blocks.size());
    return {std::string(DecoderId),confidence,std::move(blocks),std::move(undecoded),{},std::move(messages)};
}
SequentialDecodeResult decode(const MediaDocument& document,const SequentialImage& image,std::stop_token token){
    std::vector<AmstradTape::Record> records;const bool structured=sameIdentifier(document.formatId(),Tzx::Constants::FormatId);int channels{},rate{},bits{},align{};
    if(!structured&&!TapePcm::tryProfile(document.metadata(),channels,rate,bits,align))return result(image,{},token);
    for(const auto& segment:segments(image)){
        checkCancellation(token);const auto& value=segment.fields();if(!structured&&(value.kind!=SequentialSegmentKind::Samples||value.channelNumber!=0))continue;
        const auto payload=readStoredRange(value.dataRange?&*value.dataRange:nullptr,token);if(!payload)continue;
        if(structured){int offset{},length{};if(Tzx::tryDataRange(value,*payload,offset,length))if(auto record=AmstradTape::decodeRecord(std::span(*payload).subspan(offset,length),value.position,token))records.push_back(std::move(*record));}
        else{const auto samples=TapePcm::extractChannel(*payload,channels,bits,align,0);for(const auto& encoded:AmstradTape::detect(samples,rate,token))if(auto record=AmstradTape::decodeRecord(encoded,value.position,token))records.push_back(std::move(*record));}
    }
    return result(image,std::move(records),token);
}
}
std::string_view AmstradCpcTapeDecoder::id()const noexcept{return DecoderId;}
std::span<const std::string> AmstradCpcTapeDecoder::formatIds()const{static const std::vector<std::string> ids{std::string(Tzx::Constants::FormatId),std::string(WaveFormat),std::string(CswFormat),std::string(RawFormat),std::string(AudioFormat),std::string(VocFormat)};return ids;}
std::span<const std::string> AmstradCpcTapeDecoder::machineIds()const{static const std::vector<std::string> ids{std::string(MachineId)};return ids;}
bool AmstradCpcTapeDecoder::canDecode(const MediaDocument& document,std::optional<std::string> machine)const{return canDecodeTape(document,machine,formatIds(),machineIds());}
std::future<SequentialDecodeResult> AmstradCpcTapeDecoder::decodeAsync(const MediaDocument& document,std::optional<std::string> machine,std::stop_token token)const{return std::async(std::launch::deferred,[&document,machine=std::move(machine),token]{
    checkCancellation(token);if(!AmstradCpcTapeDecoder{}.canDecode(document,machine))throw MediaError(DiagnosticCode::CpcTapeIncompatible);return decode(document,dynamic_cast<const SequentialImage&>(document.representation()),token);
});}
}
