#include "Encoder.h"
#include "../Tzx/SignalEncoder.h"
#include "../../../Formats/Tape/SpectrumTap/Constants.h"
#include "../../../Formats/Tape/Tzx/Metadata.h"
#include "../../../Formats/Tape/Common/Pcm.h"
#include "../../../Sources/RandomAccessSources.h"
#include "../../../Images/RecognitionContext.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <cmath>
#include <limits>
namespace RetroMediaLab::MediaEngine {
namespace {
using namespace SpectrumTap::Constants;
bool valid(const SequentialDecodedBlock& block){
    if(block.integrityValid()!=true||block.data().size()<MinimumBlockLength||block.data().size()>MaximumBlockLength)return false;
    std::uint8_t checksum=0;for(auto byte:block.data())checksum^=byte;return checksum==0;
}
std::shared_ptr<const SequentialImage> tap(std::span<const SequentialDecodedBlock> blocks,std::stop_token token){
    std::vector<SequentialSegment> output;std::int64_t length=0;
    for(const auto& block:blocks){
        checkCancellation(token);const auto data=block.data();SequentialSegmentFields value;value.position=output.size();value.kind=SequentialSegmentKind::DataBlock;value.length=data.size();value.dataRange=DataRange(0,data.size(),DataRangeKind::Stored,memorySource(data));
        value.metadata={{std::string(BlockMetadataKey),std::string(TrueValue)},{std::string(ChecksumKey),std::string(TrueValue)},{std::string(FlagKey),Tzx::blockIdText(data.front())},{std::string(HeaderKey),std::string(data.size()==HeaderBlockLength&&data.front()==HeaderFlag?TrueValue:FalseValue)}};
        if(data.size()>static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()-length))throw MediaError(DiagnosticCode::SpectrumTapeSignalLimit);length+=data.size();output.emplace_back(std::move(value));
    }
    return std::make_shared<const SequentialImage>(length,std::nullopt,std::move(output));
}
std::shared_ptr<const SequentialImage> wave(const SequentialEncodeRequest& request,std::stop_token token){
    int rate{};TapePcm::trySampleRate(request.parameters(),rate);std::vector<std::int16_t> samples;auto level=PcmAmplitude;
    const auto append=[&](std::uint16_t states,int count){
        checkCancellation(token);const auto size=std::max(double(1),TapePcm::roundEven(double(states)*rate/Tzx::Constants::TStatesPerSecond));
        if(!std::isfinite(size)||size*count>MaximumPcmSamples-samples.size())throw MediaError(DiagnosticCode::SpectrumTapeSignalLimit);
        TapePcm::appendPulses(samples,level,states,count,rate,Tzx::Constants::TStatesPerSecond);
    };
    for(const auto& block:request.blocks()){
        checkCancellation(token);const auto data=block.data();append(PilotPulseTStates,data.front()==HeaderFlag?HeaderPilotPulseCount:DataPilotPulseCount);append(SyncFirstPulseTStates,1);append(SyncSecondPulseTStates,1);
        for(auto value:data)for(int bit=LastBit;bit>=0;--bit)append((value&(1<<bit))?OnePulseTStates:ZeroPulseTStates,PulsePair);
        if(static_cast<std::size_t>(rate)>MaximumPcmSamples-samples.size())throw MediaError(DiagnosticCode::SpectrumTapeSignalLimit);TapePcm::appendSilence(samples,rate,1);
    }
    SourceOptions options;options.primaryPath=EncodedWave;const auto document=TapePcm::signalDocument(std::make_shared<MediaSource>(std::move(options)),std::string(WaveFormat),samples,rate,token);
    const auto& image=dynamic_cast<const SequentialImage&>(document->representation());auto value=image.segments()->front().fields();value.length=samples.size();value.start=SequentialTime{};value.duration=image.duration();value.direction=SequentialTravelDirection::Forward;value.metadata[std::string(ChannelKey)]=ChannelZero;value.metadata[std::string(InterleavedKey)]=TrueValue;
    return std::make_shared<const SequentialImage>(image.logicalLength(),image.duration(),std::vector<SequentialSegment>{SequentialSegment(std::move(value))});
}
}
std::string_view SpectrumTapeEncoder::id()const noexcept{return EncoderId;}
std::span<const std::string> SpectrumTapeEncoder::formatIds()const{static const std::vector<std::string> ids{std::string(FormatId),std::string(Tzx::Constants::FormatId),std::string(WaveFormat)};return ids;}
std::span<const std::string> SpectrumTapeEncoder::machineIds()const{static const std::vector<std::string> ids{std::string(MachineId)};return ids;}
bool SpectrumTapeEncoder::canEncode(const SequentialEncodeRequest& request)const{
    int rate{};return std::any_of(formatIds().begin(),formatIds().end(),[&](const auto& id){return sameIdentifier(request.targetFormatId(),id);})&&sameIdentifier(request.machineId(),MachineId)
        &&!request.blocks().empty()&&request.retainedSegments().empty()&&std::all_of(request.blocks().begin(),request.blocks().end(),valid)&&(!sameIdentifier(request.targetFormatId(),WaveFormat)||TapePcm::trySampleRate(request.parameters(),rate));
}
std::future<std::shared_ptr<const SequentialImage>> SpectrumTapeEncoder::encodeAsync(SequentialEncodeRequest request,std::stop_token token)const{return std::async(std::launch::deferred,[request=std::move(request),token]{
    checkCancellation(token);if(!SpectrumTapeEncoder{}.canEncode(request))throw MediaError(DiagnosticCode::SpectrumTapeEncode);
    if(sameIdentifier(request.targetFormatId(),Tzx::Constants::FormatId))return TzxSignalEncoder{}.encodeAsync(std::move(request),token).get();
    return sameIdentifier(request.targetFormatId(),WaveFormat)?wave(request,token):tap(request.blocks(),token);
});}
}
