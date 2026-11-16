#include "AtomDecoder.h"
#include "../Compatibility.h"
#include "../../../Images/RecognitionContext.h"
#include "../Fsk/Serial.h"
#include "../../../Formats/Tape/AtomTap/Codec.h"
#include "../../../Formats/Tape/AtomTap/Constants.h"
#include "../../../Formats/Tape/Uef/Constants.h"
#include "../../../Formats/Tape/Common/Signal.h"
#include "../../../Models/Blocks/Internal.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaEngine/DiagnosticKeys.h>
#include <algorithm>
#include <set>
namespace RetroMediaLab::MediaEngine {
namespace {
using namespace AtomTap::Constants;
using Segments=std::vector<SequentialSegment>;
const Segments& segments(const SequentialImage& image){static const Segments empty;return image.segments()?*image.segments():empty;}
std::string_view chunkId(const SequentialSegment& segment){const auto& metadata=segment.fields().metadata;const auto found=metadata.find(std::string(Uef::Constants::ChunkIdMetadataKey));return found==metadata.end()?std::string_view{}:found->second;}
bool directSerial(const SequentialImage& image){
    return std::all_of(segments(image).begin(),segments(image).end(),[](const auto& segment){
        const auto kind=segment.fields().kind;const auto id=chunkId(segment);
        return kind==SequentialSegmentKind::Carrier||kind==SequentialSegmentKind::Silence
            ||(kind==SequentialSegmentKind::DataBlock&&(id==ImplicitChunk||id==DefinedChunk||id==ImplicitAtomChunk))
            ||(kind==SequentialSegmentKind::TapeMark&&(id==BaudChunk||id==PhaseChunk||id==FrequencyChunk));
    });
}
SequentialDecodeResult failed(const SequentialImage& image,std::string_view key){return {std::string(DecoderId),0,{},segments(image),{},{{std::string(key),{},{}}}};}
SequentialDecodeResult fromSource(const SequentialImage& image,SequentialDecodeResult result){
    std::vector<std::int64_t> positions;std::set<std::int64_t> seen;
    for(const auto& segment:segments(image))if(seen.insert(segment.fields().position).second)positions.push_back(segment.fields().position);
    std::vector<SequentialDecodedBlock> blocks;for(const auto& block:result.blocks())blocks.push_back(block.withSourcePositions(positions));
    const bool retain=blocks.empty()||!result.undecodedSegments().empty()||!result.diagnostics().empty()||!result.messages().empty();
    return {std::string(DecoderId),result.confidence(),std::move(blocks),retain?segments(image):Segments{},
        {result.diagnostics().begin(),result.diagnostics().end()},{result.messages().begin(),result.messages().end()}};
}
SequentialDecodeResult decode(const MediaDocument& document,const SequentialImage& image,std::stop_token token){
    if(sameIdentifier(document.formatId(),FormatId))return AtomTap::decode(image,token);
    const MediaDocument* pcm=&document;std::shared_ptr<const MediaDocument> owner;
    int channels{},rate{},bits{},align{};
    if(sameIdentifier(document.formatId(),UefFormat)&&!TapePcm::tryProfile(document.metadata(),channels,rate,bits,align)){
        if(directSerial(image)){
            std::vector<std::uint8_t> values;
            for(const auto& segment:segments(image)){
                checkCancellation(token);const auto& value=segment.fields();if(value.kind!=SequentialSegmentKind::DataBlock)continue;
                const auto data=readStoredRange(value.dataRange?&*value.dataRange:nullptr,token).value_or(std::vector<std::uint8_t>{});
                const auto id=chunkId(segment);std::size_t offset{};
                if(id==ImplicitChunk||id==ImplicitAtomChunk)offset=0;
                else if(id==DefinedChunk&&data.size()>=SerialPrefix&&data[0]==DataBits&&data[1]==NoParity&&(data[StopOffset]==StopBits||data[StopOffset]==AlternateStopBits))offset=SerialPrefix;
                else return failed(image,DiagnosticKeys::AtomTapeFraming);
                if(data.size()-offset>MaximumBuffer-values.size())throw MediaError(DiagnosticCode::AtomTapLimit);
                values.insert(values.end(),data.begin()+offset,data.end());
            }
            return fromSource(image,AtomTap::decodeSerial(values,token));
        }
        auto signal=TapeSignal::fromUef(image,token);
        owner=TapePcm::signalDocument(document.mediaSource(),std::string(WaveFormat),signal.signal,token);pcm=owner.get();
    }
    if(!TapePcm::tryProfile(pcm->metadata(),channels,rate,bits,align))return failed(image,DiagnosticKeys::AtomTapePcmRequired);
    const auto native=TapePcm::readChannels(*pcm,token);
    const auto serial=Fsk::decode(native.channels.front(),native.rate,Baud,Uef::Constants::AcornZeroFrequency,Uef::Constants::AcornOneFrequency,DataBits,StopBits,Uef::Constants::MinimumSamplesPerBit,token);
    return fromSource(image,AtomTap::decodeSerial(serial.bytes(),token));
}
}
std::string_view AtomTapeDecoder::id()const noexcept{return DecoderId;}
std::span<const std::string> AtomTapeDecoder::formatIds()const{static const std::vector<std::string> ids{std::string(FormatId),std::string(UefFormat),std::string(WaveFormat),std::string(CswFormat),std::string(RawFormat),std::string(VocFormat)};return ids;}
std::span<const std::string> AtomTapeDecoder::machineIds()const{static const std::vector<std::string> ids{std::string(MachineId)};return ids;}
bool AtomTapeDecoder::canDecode(const MediaDocument& document,std::optional<std::string> machine)const{return canDecodeTape(document,machine,formatIds(),machineIds());}
std::future<SequentialDecodeResult> AtomTapeDecoder::decodeAsync(const MediaDocument& document,std::optional<std::string> machine,std::stop_token token)const{
    return std::async(std::launch::deferred,[&document,machine=std::move(machine),token]{checkCancellation(token);if(!AtomTapeDecoder{}.canDecode(document,machine))throw MediaError(DiagnosticCode::AtomTapeIncompatible);return decode(document,dynamic_cast<const SequentialImage&>(document.representation()),token);});
}
}
