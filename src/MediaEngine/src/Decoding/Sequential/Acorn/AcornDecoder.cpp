#include "AcornDecoder.h"
#include "../Compatibility.h"
#include "../Fsk/Serial.h"
#include "../../../Formats/Tape/Acorn/Blocks.h"
#include "../../../Formats/Tape/Acorn/Constants.h"
#include "../../../Formats/Tape/Uef/Constants.h"
#include "../../../Formats/Tape/Common/Signal.h"
#include "../../../Images/RecognitionContext.h"
#include "../../../Models/Blocks/Internal.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaEngine/DiagnosticKeys.h>
#include <QString>
#include <algorithm>
#include <charconv>
#include <set>
#include <tuple>
namespace RetroMediaLab::MediaEngine {
namespace {
using namespace AcornTape::Constants;using namespace Uef::Constants;
using Segments=std::vector<SequentialSegment>;
const Segments& segments(const SequentialImage& image){static const Segments empty;return image.segments()?*image.segments():empty;}
std::string_view chunkId(const SequentialSegment& segment){const auto found=segment.fields().metadata.find(std::string(ChunkIdMetadataKey));return found==segment.fields().metadata.end()?std::string_view{}:found->second;}
SequentialDecodeResult result(const SequentialImage& image,std::vector<SequentialDecodedBlock> blocks,const std::set<std::int64_t>& consumed,std::string_view emptyMessage){
    Segments retained;for(const auto& segment:segments(image))if(!consumed.contains(segment.fields().position))retained.push_back(segment);
    const auto valid=std::count_if(blocks.begin(),blocks.end(),[](const auto& block){return block.integrityValid()==true;});
    const auto confidence=blocks.empty()?0:double(valid)/blocks.size();std::vector<MediaFileSystems::MessageRecipe> messages;
    if(blocks.empty())messages.push_back({std::string(emptyMessage),{},{}});
    return {std::string(DecoderId),confidence,std::move(blocks),std::move(retained),{},std::move(messages)};
}
SequentialDecodeResult wave(const MediaDocument& document,const SequentialImage& image,std::stop_token token){
    int channels{},rate{},bits{},align{};
    if(!TapePcm::tryProfile(document.metadata(),channels,rate,bits,align))return result(image,{}, {},DiagnosticKeys::AcornTapePcmProfile);
    const auto native=TapePcm::readChannels(document,token);
    const auto fast=Fsk::decode(native.channels.front(),native.rate,DefaultBaudRate,AcornZeroFrequency,AcornOneFrequency,AcornDataBitsPerFrame,AcornStopBitsPerFrame,MinimumSamplesPerBit,token);
    const auto slow=Fsk::decode(native.channels.front(),native.rate,SlowBaud,AcornZeroFrequency,AcornOneFrequency,AcornDataBitsPerFrame,AcornStopBitsPerFrame,MinimumSamplesPerBit,token);
    std::vector<std::int64_t> positions;std::set<std::int64_t> seen;
    for(const auto& segment:segments(image)){const auto& value=segment.fields();if(value.kind==SequentialSegmentKind::Samples&&(!value.channelNumber||*value.channelNumber==0)&&seen.insert(value.position).second)positions.push_back(value.position);}
    auto fastBlocks=AcornTape::decodeBlocks(fast.bytes(),positions,WaveSource,0,fast.positions(),token);
    auto slowBlocks=AcornTape::decodeBlocks(slow.bytes(),positions,WaveSource,0,slow.positions(),token);
    std::vector<SequentialDecodedBlock> valid;
    for(const auto* values:{&fastBlocks,&slowBlocks})for(const auto& block:*values)if(block.integrityValid()==true)valid.push_back(block);
    std::stable_sort(valid.begin(),valid.end(),[](const auto& a,const auto& b){return std::stoi(a.metadata().at(std::string(SamplePositionKey)))<std::stoi(b.metadata().at(std::string(SamplePositionKey)));});
    std::vector<SequentialDecodedBlock> blocks;
    if(valid.empty())blocks=fastBlocks.size()>=slowBlocks.size()?std::move(fastBlocks):std::move(slowBlocks);
    else{
        std::set<std::tuple<std::string,std::string,std::string>> emitted;
        for(const auto& block:valid){const auto& metadata=block.metadata();if(emitted.emplace(metadata.at(std::string(SamplePositionKey)),metadata.at(std::string(FileNameMetadataKey)),metadata.at(std::string(BlockNumberMetadataKey))).second)blocks.push_back(block.atPosition(blocks.size()));}
    }
    std::set<std::int64_t> consumed;if(!blocks.empty())consumed.insert(positions.begin(),positions.end());
    return result(image,std::move(blocks),consumed,DiagnosticKeys::AcornTapeNoPcmBlocks);
}
SequentialDecodeResult uef(const MediaDocument& document,const SequentialImage& image,std::stop_token token){
    std::vector<SequentialDecodedBlock> blocks;std::set<std::int64_t> consumed;std::vector<const SequentialSegment*> sorted;
    for(const auto& segment:segments(image))sorted.push_back(&segment);
    std::stable_sort(sorted.begin(),sorted.end(),[](const auto* a,const auto* b){return a->fields().position<b->fields().position;});
    for(const auto* segment:sorted){
        checkCancellation(token);if(segment->fields().kind!=SequentialSegmentKind::DataBlock)continue;
        const auto idText=QString::fromUtf8(chunkId(*segment)).trimmed().toStdString();unsigned id{};
        const auto parsed=std::from_chars(idText.data(),idText.data()+idText.size(),id,RadixHex);
        if(parsed.ec!=std::errc{}||parsed.ptr!=idText.data()+idText.size()||id!=ImplicitData)continue;
        const auto& range=segment->fields().dataRange;const auto bytes=readStoredRange(range?&*range:nullptr,token);if(!bytes)continue;
        auto decoded=AcornTape::decodeBlocks(*bytes,{segment->fields().position},ImplicitSource,blocks.size(),{},token);
        if(decoded.empty())continue;blocks.insert(blocks.end(),std::make_move_iterator(decoded.begin()),std::make_move_iterator(decoded.end()));consumed.insert(segment->fields().position);
    }
    const bool synthesize=std::any_of(segments(image).begin(),segments(image).end(),[](const auto& segment){const auto id=chunkId(segment);return id==ExplicitChunk||id==DefinedChunk||id==AtomChunk||id==CyclesChunk;});
    if(synthesize){auto signal=TapeSignal::fromUef(image,token);const auto pcm=TapePcm::signalDocument(document.mediaSource(),std::string(WaveFormat),signal.signal,token);return wave(*pcm,dynamic_cast<const SequentialImage&>(pcm->representation()),token);}
    return result(image,std::move(blocks),consumed,DiagnosticKeys::AcornTapeNoUefBlocks);
}
}
std::string_view AcornTapeDecoder::id()const noexcept{return DecoderId;}
std::span<const std::string> AcornTapeDecoder::formatIds()const{static const std::vector<std::string> ids{std::string(FormatId),std::string(WaveFormat),std::string(CswFormat),std::string(RawFormat),std::string(VocFormat)};return ids;}
std::span<const std::string> AcornTapeDecoder::machineIds()const{static const std::vector<std::string> ids{std::string(MachineId)};return ids;}
bool AcornTapeDecoder::canDecode(const MediaDocument& document,std::optional<std::string> machine)const{return canDecodeTape(document,machine,formatIds(),machineIds());}
std::future<SequentialDecodeResult> AcornTapeDecoder::decodeAsync(const MediaDocument& document,std::optional<std::string> machine,std::stop_token token)const{
    return std::async(std::launch::deferred,[&document,machine=std::move(machine),token]{checkCancellation(token);if(!AcornTapeDecoder{}.canDecode(document,machine))throw MediaError(DiagnosticCode::AcornTapeIncompatible);const auto& image=dynamic_cast<const SequentialImage&>(document.representation());return sameIdentifier(document.formatId(),FormatId)&&!document.metadata().contains(std::string(TapePcm::Constants::RateKey))?uef(document,image,token):wave(document,image,token);});
}
}
