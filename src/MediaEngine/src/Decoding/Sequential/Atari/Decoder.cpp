#include "Decoder.h"
#include "../Compatibility.h"
#include "../Fsk/Serial.h"
#include "../../../Formats/Tape/AtariCas/Constants.h"
#include "../../../Formats/Tape/Common/Pcm.h"
#include "../../../Images/RecognitionContext.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaEngine/DiagnosticKeys.h>
#include <algorithm>
#include <set>
#include <cmath>
namespace RetroMediaLab::MediaEngine {
namespace {
using namespace AtariTapeConstants;
using Segments=std::vector<SequentialSegment>;
const Segments& segments(const SequentialImage& image){static const Segments empty;return image.segments()?*image.segments():empty;}
std::string join(const std::set<std::string>& values){std::string result;for(const auto& item:values){if(!result.empty())result+=ListSeparator;result+=item;}return result;}
SequentialDecodeResult failed(const SequentialImage& image,std::string_view key){return {std::string(DecoderId),0,{},segments(image),{},{{std::string(key),{}, {}}}};}
SequentialDecodedBlock fskBlock(std::int64_t position,std::vector<std::uint8_t> bytes,std::vector<std::int64_t> sources,std::string_view kind) {
    std::optional<bool> valid;if(bytes.size()>=WordBytes)valid=AtariTape::checksum(std::span(bytes).first(bytes.size()-1))==bytes.back();
    std::map<std::string,std::string> metadata{{std::string(SourceKindKey),std::string(kind)},{std::string(ChecksumPresentKey),std::string(valid.has_value()?True:False)}};
    return {position,std::move(bytes),std::move(sources),valid,std::move(metadata)};
}
SequentialDecodeResult decodeCas(const SequentialImage& image,std::stop_token token) {
    std::vector<SequentialDecodedBlock> blocks;std::set<std::int64_t> consumed;std::vector<std::uint8_t> data;std::vector<std::int64_t> positions;std::int64_t records=0,full=0,partial=0;bool integrity=true;std::set<std::string> baudRates;
    for(const auto& segment:segments(image))if(segment.fields().kind==SequentialSegmentKind::TapeMark||segment.fields().kind==SequentialSegmentKind::Carrier)consumed.insert(segment.fields().position);
    const auto flush=[&](bool endRecord) {
        if(positions.empty())return;std::map<std::string,std::string> metadata{{std::string(SourceKindKey),std::string(RecordsSource)},{std::string(LogicalOrdinalKey),std::to_string(blocks.size()+1)},{std::string(RecordCountKey),std::to_string(records)},{std::string(FullCountKey),std::to_string(full)},{std::string(PartialCountKey),std::to_string(partial)},{std::string(EndPresentKey),std::string(endRecord?True:False)},{std::string(BaudRatesKey),join(baudRates)},{std::string(ChecksumPresentKey),std::string(True)}};
        blocks.emplace_back(blocks.size(),std::move(data),std::move(positions),integrity,std::move(metadata));data.clear();positions.clear();records=full=partial=0;integrity=true;baudRates.clear();
    };
    for(const auto& segment:segments(image)) {
        checkCancellation(token);const auto& value=segment.fields();const auto id=value.metadata.find(std::string(ChunkKey));
        if(value.kind!=SequentialSegmentKind::DataBlock||id==value.metadata.end()||id->second!=DataChunk||!value.dataRange||!value.dataRange->source()||value.dataRange->length()>TapePcm::Constants::MaximumBuffer)continue;
        const auto bytes=readStoredRange(&*value.dataRange,token);const auto record=AtariTape::standardRecord(*bytes);
        if(!record){flush(false);if(AtariTape::extendedPadding(*bytes))consumed.insert(value.position);continue;}
        consumed.insert(value.position);positions.push_back(value.position);++records;integrity=integrity&&record->checksumValid;
        if(const auto baud=value.metadata.find(std::string(BaudKey));baud!=value.metadata.end())baudRates.insert(baud->second);
        if(record->type==FullRecord||record->type==PartialRecord){if(record->type==FullRecord)++full;else ++partial;if(record->data.size()>std::size_t(TapePcm::Constants::MaximumBuffer)-data.size())throw MediaError(DiagnosticCode::FskInvalid);data.insert(data.end(),record->data.begin(),record->data.end());}else flush(true);
    }
    flush(false);std::vector<const SequentialSegmentFields*> pulses;
    for(const auto& segment:segments(image)){const auto& value=segment.fields();if(value.kind==SequentialSegmentKind::Pulse&&value.duration&&value.metadata.contains(std::string(SignalKey)))pulses.push_back(&value);}
    std::stable_sort(pulses.begin(),pulses.end(),[](const auto* a,const auto* b){return a->position<b->position;});
    if(!pulses.empty()) {
        std::vector<std::uint8_t> bits;
        for(const auto* pulse:pulses){checkCancellation(token);const auto count=std::max(double(1),TapePcm::roundEven(double(pulse->duration->count())/SequentialTicksPerSecond*DefaultBaud));if(!std::isfinite(count)||count>TapePcm::Constants::MaximumBuffer-double(bits.size()))throw MediaError(DiagnosticCode::FskInvalid);const auto mark=sameIdentifier(pulse->metadata.at(std::string(SignalKey)),Mark);bits.insert(bits.end(),std::size_t(count),std::uint8_t(mark));}
        const auto decoded=Fsk::decodeFrames(bits,SerialBits,StopBits,token);
        if(!decoded.bytes().empty()){std::vector<std::int64_t> sources;for(const auto* pulse:pulses){sources.push_back(pulse->position);consumed.insert(pulse->position);}blocks.push_back(fskBlock(blocks.size(),{decoded.bytes().begin(),decoded.bytes().end()},std::move(sources),CasFskSource));for(const auto& segment:segments(image))if(const auto id=segment.fields().metadata.find(std::string(ChunkKey));id!=segment.fields().metadata.end()&&id->second==FskChunk)consumed.insert(segment.fields().position);}
    }
    Segments undecoded;for(const auto& segment:segments(image))if(!consumed.contains(segment.fields().position))undecoded.push_back(segment);
    const auto confidence=blocks.empty()?0:1;std::vector<MediaFileSystems::MessageRecipe> messages;if(blocks.empty())messages.push_back({std::string(DiagnosticKeys::AtariTapeNoData),{}, {}});
    return {std::string(DecoderId),double(confidence),std::move(blocks),std::move(undecoded),{},std::move(messages)};
}
SequentialDecodeResult decodeWave(const MediaDocument& document,const SequentialImage& image,std::stop_token token) {
    int channels=0,rate=0,precision=0,align=0;if(!TapePcm::tryProfile(document.metadata(),channels,rate,precision,align))return failed(image,DiagnosticKeys::TapePcmProfile);
    std::vector<const SequentialSegmentFields*> selected;for(const auto& segment:segments(image)){const auto& value=segment.fields();if(value.kind==SequentialSegmentKind::Samples&&value.channelNumber==0&&value.dataRange&&value.dataRange->source())selected.push_back(&value);}
    std::stable_sort(selected.begin(),selected.end(),[](const auto* a,const auto* b){return a->position<b->position;});if(selected.empty())return failed(image,DiagnosticKeys::TapePcmData);
    std::vector<std::uint8_t> bytes;std::vector<std::int64_t> sources;std::int64_t valid=0,tested=0;
    for(const auto* segment:selected){checkCancellation(token);if(segment->dataRange->length()>TapePcm::Constants::MaximumBuffer)continue;const auto pcm=readStoredRange(&*segment->dataRange,token);const auto samples=TapePcm::extractChannel(*pcm,channels,precision,align,0);const auto decoded=Fsk::decode(samples,rate,DefaultBaud,SpaceFrequency,MarkFrequency,SerialBits,StopBits,MinimumSamplesPerBit,token);if(decoded.bytes().size()>std::size_t(TapePcm::Constants::MaximumBuffer)-bytes.size())throw MediaError(DiagnosticCode::FskInvalid);bytes.insert(bytes.end(),decoded.bytes().begin(),decoded.bytes().end());valid+=decoded.validFrames();tested+=decoded.testedFrames();if(!decoded.bytes().empty())sources.push_back(segment->position);}
    std::set<std::int64_t> consumed(sources.begin(),sources.end());Segments undecoded;for(const auto& segment:segments(image))if(!consumed.contains(segment.fields().position))undecoded.push_back(segment);
    std::vector<SequentialDecodedBlock> blocks;std::vector<MediaFileSystems::MessageRecipe> messages;if(bytes.empty())messages.push_back({std::string(DiagnosticKeys::AtariTapeNoData),{}, {}});else blocks.push_back(fskBlock(0,std::move(bytes),std::move(sources),WaveFskSource));
    return {std::string(DecoderId),tested?double(valid)/tested:0,std::move(blocks),std::move(undecoded),{},std::move(messages)};
}
}
std::string_view AtariTapeDecoder::id()const noexcept{return DecoderId;}
std::span<const std::string> AtariTapeDecoder::formatIds()const {static const std::vector<std::string> ids{std::string(FormatId),std::string(WaveFormat),std::string(CswFormat),std::string(RawFormat)};return ids;}
std::span<const std::string> AtariTapeDecoder::machineIds()const {static const std::vector<std::string> ids{std::string(MachineId)};return ids;}
bool AtariTapeDecoder::canDecode(const MediaDocument& document,std::optional<std::string> machine)const{return canDecodeTape(document,machine,formatIds(),machineIds());}
std::future<SequentialDecodeResult> AtariTapeDecoder::decodeAsync(const MediaDocument& document,std::optional<std::string> machine,std::stop_token token)const {
    return std::async(std::launch::deferred,[&document,machine=std::move(machine),token]{checkCancellation(token);if(!AtariTapeDecoder{}.canDecode(document,machine))throw MediaError(DiagnosticCode::AtariTapeIncompatible);const auto& image=dynamic_cast<const SequentialImage&>(document.representation());return sameIdentifier(document.formatId(),FormatId)?decodeCas(image,token):decodeWave(document,image,token);});
}
}
