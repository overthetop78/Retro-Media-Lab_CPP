#include "Encoder.h"
#include "../Compatibility.h"
#include "../../../Formats/Tape/MsxCas/Constants.h"
#include "../../../Formats/Tape/Common/Pcm.h"
#include "../../../Decoding/Sequential/Fsk/Serial.h"
#include "../../../Sources/RandomAccessSources.h"
#include "../../../Images/RecognitionContext.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <limits>
#include <cmath>
namespace RetroMediaLab::MediaEngine {
namespace {
using namespace MsxCas::Constants;
std::shared_ptr<const SequentialImage> cas(std::span<const SequentialDecodedBlock> blocks,std::stop_token token){
    std::vector<SequentialSegment> segments;std::int64_t length{};
    for(const auto& block:blocks){
        checkCancellation(token);SequentialSegmentFields separator;separator.position=segments.size();separator.kind=SequentialSegmentKind::TapeMark;separator.length=SeparatorLength;
        separator.metadata={{std::string(SeparatorMetadataKey),std::string(TrueValue)},{std::string(ReconstructionMetadataKey),std::string(TrueValue)}};segments.emplace_back(std::move(separator));
        SequentialSegmentFields data;data.position=segments.size();data.kind=SequentialSegmentKind::DataBlock;data.length=block.data().size();data.direction=SequentialTravelDirection::Forward;
        data.dataRange=DataRange(0,block.data().size(),DataRangeKind::Stored,memorySource(block.data()));data.metadata={{std::string(TimingMetadataKey),std::string(NoTiming)},{std::string(ReconstructionMetadataKey),std::string(TrueValue)}};
        if(const auto found=block.metadata().find(std::string(FileTypeMetadataKey));found!=block.metadata().end())data.metadata[std::string(FileTypeMetadataKey)]=found->second;
        if(length>std::numeric_limits<std::int64_t>::max()-SeparatorLength||block.data().size()>static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()-length-SeparatorLength))throw MediaError(DiagnosticCode::MsxTapeSignalLimit);
        length+=SeparatorLength+block.data().size();segments.emplace_back(std::move(data));
    }
    return std::make_shared<const SequentialImage>(length,std::nullopt,std::move(segments));
}
std::shared_ptr<const SequentialImage> wave(std::span<const SequentialDecodedBlock> blocks,int rate,std::stop_token token){
    std::vector<std::int16_t> samples;auto level=PcmAmplitude;
    for(std::size_t index=0;index<blocks.size();++index){
        checkCancellation(token);const auto& block=blocks[index];const bool longHeader=index==0||block.metadata().contains(std::string(FileTypeMetadataKey));
        const auto silence=longHeader?LongSilenceSeconds:ShortSilenceSeconds,carrier=longHeader?LongCarrierSeconds:ShortCarrierSeconds;
        const auto seconds=silence+carrier+block.data().size()*SerialFrameBitCount/double(StandardBaudRate);
        if(rate<=0||!std::isfinite(seconds)||seconds*rate>MaximumPcmSamples-samples.size())throw MediaError(DiagnosticCode::MsxTapeSignalLimit);
        TapePcm::appendSilence(samples,rate,silence);Fsk::appendCarrier(samples,level,rate,StandardBaudRate,ZeroFrequency,OneFrequency,carrier,token);
        for(auto value:block.data())Fsk::appendFrame(samples,level,rate,StandardBaudRate,ZeroFrequency,OneFrequency,DataBitsPerFrame,StopBitsPerFrame,value,token);
    }
    SourceOptions options;options.primaryPath=EncodedWave;const auto document=TapePcm::signalDocument(std::make_shared<MediaSource>(std::move(options)),std::string(WaveFormat),samples,rate,token);
    const auto& image=dynamic_cast<const SequentialImage&>(document->representation());auto value=image.segments()->front().fields();value.length=samples.size();value.start=SequentialTime{};value.duration=image.duration();value.direction=SequentialTravelDirection::Forward;
    value.metadata[std::string(ChannelKey)]=ChannelZero;value.metadata[std::string(InterleavedKey)]=TrueValue;value.metadata[std::string(ReconstructionMetadataKey)]=TrueValue;
    return std::make_shared<const SequentialImage>(image.logicalLength(),image.duration(),std::vector<SequentialSegment>{SequentialSegment(std::move(value))});
}
}
std::string_view MsxTapeEncoder::id()const noexcept{return EncoderId;}
std::span<const std::string> MsxTapeEncoder::formatIds()const{static const std::vector<std::string> ids{std::string(FormatId),std::string(WaveFormat)};return ids;}
std::span<const std::string> MsxTapeEncoder::machineIds()const{static const std::vector<std::string> ids{std::string(MachineId)};return ids;}
bool MsxTapeEncoder::canEncode(const SequentialEncodeRequest& request)const{return canEncodeValidatedBytes(request,formatIds(),machineIds());}
std::future<std::shared_ptr<const SequentialImage>> MsxTapeEncoder::encodeAsync(SequentialEncodeRequest request,std::stop_token token)const{return std::async(std::launch::deferred,[request=std::move(request),token]{
    checkCancellation(token);if(!MsxTapeEncoder{}.canEncode(request))throw MediaError(DiagnosticCode::MsxTapeEncode);
    if(!sameIdentifier(request.targetFormatId(),WaveFormat))return cas(request.blocks(),token);
    int rate{};TapePcm::trySampleRate(request.parameters(),rate);return wave(request.blocks(),rate,token);
});}
}
