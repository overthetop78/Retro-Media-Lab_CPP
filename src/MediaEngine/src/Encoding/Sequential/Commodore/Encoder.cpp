#include "Encoder.h"
#include "../Compatibility.h"
#include "../../../Formats/Tape/CommodoreTap/Protocol.h"
#include "../../../Formats/Tape/CommodoreTap/Constants.h"
#include "../../../Formats/Tape/Common/Pcm.h"
#include "../../../Sources/RandomAccessSources.h"
#include "../../../Images/RecognitionContext.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <cmath>
namespace RetroMediaLab::MediaEngine {
namespace {
using namespace CommodoreTap::Constants;
std::shared_ptr<const SequentialImage> tap(std::span<const int> pulses,std::stop_token token){
    std::vector<SequentialSegment> output;output.reserve(pulses.size());SequentialTime elapsed{};
    for(auto cycles:pulses){
        checkCancellation(token);const auto duration=SequentialTime(static_cast<std::int64_t>((cycles/double(DefaultC64PalClockRate))*SequentialTicksPerSecond));if(duration.count()>std::numeric_limits<std::int64_t>::max()-elapsed.count())throw MediaError(DiagnosticCode::CommodoreTapeLimit);
        SequentialSegmentFields value;value.position=output.size();value.kind=SequentialSegmentKind::Pulse;value.length=cycles;value.start=elapsed;value.duration=duration;value.direction=SequentialTravelDirection::Forward;
        value.metadata={{std::string(PulseMetadataKey),std::string(TrueValue)},{std::string(CycleCountMetadataKey),std::to_string(cycles)},{std::string(HalfWaveMetadataKey),std::string(FalseValue)}};output.emplace_back(std::move(value));elapsed+=duration;
    }
    return std::make_shared<const SequentialImage>(pulses.size(),elapsed,std::move(output));
}
std::shared_ptr<const SequentialImage> wave(std::span<const int> pulses,int rate,std::stop_token token){
    std::size_t count=0;for(auto pulse:pulses){checkCancellation(token);const auto rounded=std::max(double(1),TapePcm::roundEven(pulse*double(rate)/DefaultC64PalClockRate));if(!std::isfinite(rounded)||rounded>MaximumPcmSamples-count)throw MediaError(DiagnosticCode::CommodoreTapeLimit);count+=static_cast<std::size_t>(rounded);}
    std::vector<std::int16_t> samples;samples.reserve(count);auto level=PcmAmplitude;
    for(auto pulse:pulses){checkCancellation(token);const auto size=std::max(double(1),TapePcm::roundEven(pulse*double(rate)/DefaultC64PalClockRate));samples.insert(samples.end(),static_cast<std::size_t>(size),level);level=static_cast<std::int16_t>(-level);}
    SourceOptions options;options.primaryPath=EncodedWave;const auto document=TapePcm::signalDocument(std::make_shared<MediaSource>(std::move(options)),std::string(WaveFormat),samples,rate,token);const auto& image=dynamic_cast<const SequentialImage&>(document->representation());auto value=image.segments()->front().fields();value.length=samples.size();value.start=SequentialTime{};value.duration=image.duration();value.direction=SequentialTravelDirection::Forward;value.metadata[std::string(ChannelKey)]=ChannelZero;value.metadata[std::string(InterleavedKey)]=TrueValue;
    return std::make_shared<const SequentialImage>(image.logicalLength(),image.duration(),std::vector<SequentialSegment>{SequentialSegment(std::move(value))});
}
}
std::string_view CommodoreTapeEncoder::id()const noexcept{return EncoderId;}
std::span<const std::string> CommodoreTapeEncoder::formatIds()const{static const std::vector<std::string> ids{std::string(FormatId),std::string(WaveFormat)};return ids;}
std::span<const std::string> CommodoreTapeEncoder::machineIds()const{static const std::vector<std::string> ids{std::string(MachineId)};return ids;}
bool CommodoreTapeEncoder::canEncode(const SequentialEncodeRequest& request)const{return canEncodeValidatedBytes(request,formatIds(),machineIds());}
std::future<std::shared_ptr<const SequentialImage>> CommodoreTapeEncoder::encodeAsync(SequentialEncodeRequest request,std::stop_token token)const{return std::async(std::launch::deferred,[request=std::move(request),token]{
    checkCancellation(token);if(!CommodoreTapeEncoder{}.canEncode(request))throw MediaError(DiagnosticCode::CommodoreTapeEncode);
    std::vector<int> pulses;
    for(const auto& block:request.blocks()){checkCancellation(token);if(block.data().size()>(MaximumPulseCount-pulses.size())/FramePulseCount)throw MediaError(DiagnosticCode::CommodoreTapeLimit);auto encoded=CommodoreTape::encode(block.data(),token);pulses.insert(pulses.end(),encoded.begin(),encoded.end());}
    int rate{};if(sameIdentifier(request.targetFormatId(),WaveFormat)){TapePcm::trySampleRate(request.parameters(),rate);return wave(pulses,rate,token);}return tap(pulses,token);
});}
}
