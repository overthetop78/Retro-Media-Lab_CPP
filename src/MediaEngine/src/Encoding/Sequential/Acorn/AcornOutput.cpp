#include "AcornOutput.h"
#include "../../../Formats/Tape/Acorn/Constants.h"
#include "../../../Formats/Tape/Uef/Constants.h"
#include "../../../Formats/Tape/Common/Pcm.h"
#include "../../../Decoding/Sequential/Fsk/Serial.h"
#include "../../../Sources/RandomAccessSources.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <bit>
#include <cmath>
#include <limits>
namespace RetroMediaLab::MediaEngine::AcornTape {
namespace {
using namespace Constants;using namespace Uef::Constants;
SequentialTime time(double seconds){const auto ticks=seconds*TicksPerSecond;if(!std::isfinite(ticks)||ticks<0||ticks>=double(std::numeric_limits<std::int64_t>::max()))throw MediaError(DiagnosticCode::AcornTapeSignalLimit);return SequentialTime(static_cast<std::int64_t>(ticks));}
std::map<std::string,std::string> context(std::string_view chunk){return {{std::string(ChunkIdMetadataKey),std::string(chunk)},{std::string(BaseFrequencyMetadataKey),std::to_string(DefaultBaseFrequency)},{std::string(BaudRateMetadataKey),std::to_string(DefaultBaudRate)},{std::string(PhaseMetadataKey),std::to_string(InitialPhase)}};}
}
std::shared_ptr<const SequentialImage> encodeUef(const std::vector<std::vector<std::uint8_t>>& blocks,std::stop_token token){
    std::vector<SequentialSegment> segments;SequentialTime elapsed{};std::int64_t length{};
    const auto add=[&](SequentialSegmentKind kind,std::span<const std::uint8_t> bytes,SequentialTime duration,std::string_view id,bool data){
        SequentialSegmentFields value;value.position=segments.size();value.kind=kind;value.start=elapsed;value.duration=duration;value.dataRange=DataRange(0,bytes.size(),DataRangeKind::Stored,memorySource(bytes));value.metadata=context(id);
        if(data){value.length=bytes.size();value.direction=SequentialTravelDirection::Forward;value.metadata[std::string(ReconstructionMetadataKey)]=TrueValue;}
        if(duration.count()>std::numeric_limits<std::int64_t>::max()-elapsed.count())throw MediaError(DiagnosticCode::AcornTapeSignalLimit);elapsed+=duration;segments.emplace_back(std::move(value));
    };
    for(std::size_t index=0;index<blocks.size();++index){
        checkCancellation(token);const auto& bytes=blocks[index];const auto seconds=index==0?InitialCarrierSeconds:InterBlockCarrierSeconds;
        std::vector<std::uint8_t> carrier(WordBytes);qToLittleEndian(static_cast<std::uint16_t>(TapePcm::roundEven(seconds*WordBytes*DefaultBaseFrequency)),carrier.data());add(SequentialSegmentKind::Carrier,carrier,time(seconds),CarrierChunk,false);
        add(SequentialSegmentKind::DataBlock,bytes,time(bytes.size()*(1+AcornDataBitsPerFrame+AcornStopBitsPerFrame)/double(DefaultBaudRate)),ImplicitChunk,true);length+=bytes.size();
        std::vector<std::uint8_t> gap(FloatBytes);qToLittleEndian(std::bit_cast<std::uint32_t>(static_cast<float>(InterBlockGapSeconds)),gap.data());add(SequentialSegmentKind::Silence,gap,time(InterBlockGapSeconds),GapChunk,false);
    }
    return std::make_shared<const SequentialImage>(length,elapsed,std::move(segments));
}
std::shared_ptr<const SequentialImage> encodeWave(const std::vector<std::vector<std::uint8_t>>& blocks,int rate,std::stop_token token){
    std::vector<std::int16_t> samples;auto level=PcmAmplitude;
    for(std::size_t index=0;index<blocks.size();++index){
        checkCancellation(token);const auto carrier=index==0?InitialCarrierSeconds:InterBlockCarrierSeconds;
        const auto seconds=(index?InterBlockGapSeconds:0)+carrier+blocks[index].size()*(1+AcornDataBitsPerFrame+AcornStopBitsPerFrame)/double(DefaultBaudRate);
        if(rate<=0||!std::isfinite(seconds)||seconds*rate>MaximumSignalSamples-samples.size())throw MediaError(DiagnosticCode::AcornTapeSignalLimit);
        if(index)TapePcm::appendSilence(samples,rate,InterBlockGapSeconds);
        Fsk::appendCarrier(samples,level,rate,DefaultBaudRate,AcornZeroFrequency,AcornOneFrequency,carrier,token);
        for(auto byte:blocks[index])Fsk::appendFrame(samples,level,rate,DefaultBaudRate,AcornZeroFrequency,AcornOneFrequency,AcornDataBitsPerFrame,AcornStopBitsPerFrame,byte,token);
    }
    SourceOptions options;options.primaryPath=EncodedWave;
    const auto document=TapePcm::signalDocument(std::make_shared<MediaSource>(std::move(options)),std::string(WaveFormat),samples,rate,token);const auto& image=dynamic_cast<const SequentialImage&>(document->representation());
    auto value=image.segments()->front().fields();value.length=samples.size();value.start=SequentialTime{};value.duration=image.duration();value.direction=SequentialTravelDirection::Forward;
    value.metadata[std::string(ChannelKey)]=ChannelZero;value.metadata[std::string(InterleavedKey)]=TrueValue;value.metadata[std::string(ReconstructionMetadataKey)]=TrueValue;
    return std::make_shared<const SequentialImage>(image.logicalLength(),image.duration(),std::vector<SequentialSegment>{SequentialSegment(std::move(value))});
}
}
