#include "Decoder.h"
#include "Pulses.h"
#include "../Compatibility.h"
#include "../../../Formats/Tape/SpectrumTap/Constants.h"
#include "../../../Formats/Tape/Tzx/Metadata.h"
#include "../../../Formats/Tape/Common/Pcm.h"
#include "../../../Models/Blocks/Internal.h"
#include "../../../Images/RecognitionContext.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaEngine/DiagnosticKeys.h>
#include <QtEndian>
#include <algorithm>
#include <set>
namespace RetroMediaLab::MediaEngine {
namespace {
using namespace SpectrumTap::Constants;
using Segments=std::vector<SequentialSegment>;
const Segments& segments(const SequentialImage& image){static const Segments empty;return image.segments()?*image.segments():empty;}
SequentialDecodedBlock create(std::int64_t position,std::vector<std::uint8_t> bytes,std::int64_t source,std::string format){
    std::uint8_t checksum=0;for(auto byte:bytes)checksum^=byte;
    const auto flag=Tzx::blockIdText(bytes.front());const auto header=bytes.size()==HeaderBlockLength&&bytes.front()==HeaderFlag;
    return {position,std::move(bytes),{source},checksum==0,{{std::string(SourceFormatKey),std::move(format)},{std::string(FlagKey),flag},{std::string(HeaderKey),std::string(header?TrueValue:FalseValue)}}};
}
SequentialDecodeResult decode(const MediaDocument& document,const SequentialImage& image,std::stop_token token){
    const bool pcm=!sameIdentifier(document.formatId(),FormatId)&&!sameIdentifier(document.formatId(),Tzx::Constants::FormatId);
    int channels{},rate{},bits{},align{};if(pcm&&!TapePcm::tryProfile(document.metadata(),channels,rate,bits,align))return {std::string(DecoderId),0,{},segments(image),{},{{std::string(DiagnosticKeys::SpectrumTapePcm),{},{}}}};
    std::vector<SequentialDecodedBlock> blocks;std::set<std::int64_t> consumed;
    for(const auto& segment:segments(image)){
        checkCancellation(token);const auto& value=segment.fields();const auto* range=value.dataRange?&*value.dataRange:nullptr;
        if(pcm){
            if(value.kind!=SequentialSegmentKind::Samples||value.channelNumber!=0)continue;
            const auto data=readStoredRange(range,token);if(!data)continue;
            const auto samples=TapePcm::extractChannel(*data,channels,bits,align,0);auto found=SpectrumTape::detect(samples,rate,token);
            if(!found.empty())consumed.insert(value.position);
            for(auto& bytes:found)blocks.push_back(create(blocks.size(),std::move(bytes),value.position,std::string(WaveSource)));continue;
        }
        std::optional<std::vector<std::uint8_t>> data;
        if(sameIdentifier(document.formatId(),FormatId)&&value.metadata.contains(std::string(BlockMetadataKey)))data=readStoredRange(range,token);
        else if(sameIdentifier(document.formatId(),Tzx::Constants::FormatId)){
            const auto payload=readStoredRange(range,token);int offset{},length{};if(payload&&Tzx::tryDataRange(value,*payload,offset,length)&&length>=MinimumBlockLength)data=std::vector<std::uint8_t>(payload->begin()+offset,payload->begin()+offset+length);
        }
        if(!data||data->size()<MinimumBlockLength)continue;
        blocks.push_back(create(blocks.size(),std::move(*data),value.position,document.formatId()));consumed.insert(value.position);
    }
    std::vector<SequentialSegment> undecoded;for(const auto& segment:segments(image))if(!consumed.contains(segment.fields().position))undecoded.push_back(segment);
    const auto valid=std::count_if(blocks.begin(),blocks.end(),[](const auto& block){return block.integrityValid()==true;});const auto confidence=blocks.empty()?0:valid/double(blocks.size());std::vector<MediaFileSystems::MessageRecipe> messages;
    if(blocks.empty())messages.push_back({std::string(pcm?DiagnosticKeys::SpectrumTapeNoPulses:DiagnosticKeys::SpectrumTapeNoBlocks),{},{}});
    return {std::string(DecoderId),confidence,std::move(blocks),std::move(undecoded),{},std::move(messages)};
}
}
std::string_view SpectrumTapeDecoder::id()const noexcept{return DecoderId;}
std::span<const std::string> SpectrumTapeDecoder::formatIds()const{static const std::vector<std::string> ids{std::string(FormatId),std::string(Tzx::Constants::FormatId),std::string(WaveFormat),std::string(CswFormat),std::string(RawFormat)};return ids;}
std::span<const std::string> SpectrumTapeDecoder::machineIds()const{static const std::vector<std::string> ids{std::string(MachineId)};return ids;}
bool SpectrumTapeDecoder::canDecode(const MediaDocument& document,std::optional<std::string> machine)const{return canDecodeTape(document,machine,formatIds(),machineIds());}
std::future<SequentialDecodeResult> SpectrumTapeDecoder::decodeAsync(const MediaDocument& document,std::optional<std::string> machine,std::stop_token token)const{return std::async(std::launch::deferred,[&document,machine=std::move(machine),token]{
    checkCancellation(token);if(!SpectrumTapeDecoder{}.canDecode(document,machine))throw MediaError(DiagnosticCode::SpectrumTapeIncompatible);return decode(document,dynamic_cast<const SequentialImage&>(document.representation()),token);
});}
}
