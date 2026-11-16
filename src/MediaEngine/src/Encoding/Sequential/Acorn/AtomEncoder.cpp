#include "AtomEncoder.h"
#include "../../../Formats/Tape/AtomTap/Reader.h"
#include "../../../Formats/Tape/AtomTap/Codec.h"
#include "../../../Formats/Tape/AtomTap/Constants.h"
#include "../../../Formats/Tape/Uef/Constants.h"
#include "../../../Formats/Tape/Common/Pcm.h"
#include "../../../Decoding/Sequential/Fsk/Serial.h"
#include "../../../Models/Blocks/Internal.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <algorithm>
#include <bit>
#include <cmath>
namespace RetroMediaLab::MediaEngine {
namespace {
using namespace AtomTap::Constants;
std::shared_ptr<const MediaSource> source(std::string_view path){SourceOptions options;options.primaryPath=path;return std::make_shared<MediaSource>(std::move(options));}
std::shared_ptr<const SequentialImage> wave(const SequentialImage& image,int rate,std::stop_token token){
    std::vector<std::int16_t> samples;std::int16_t level=-Uef::Constants::PcmAmplitude;
    if(image.segments())for(const auto& segment:*image.segments()){
        checkCancellation(token);const auto& value=segment.fields();if(value.kind==SequentialSegmentKind::TapeMark)continue;
        const auto data=readStoredRange(value.dataRange?&*value.dataRange:nullptr,token).value_or(std::vector<std::uint8_t>{});
        double seconds{};
        if(value.kind==SequentialSegmentKind::Carrier){if(data.size()!=WordBytes)throw MediaError(DiagnosticCode::AtomTapData);seconds=qFromLittleEndian<std::uint16_t>(data.data())/(double(WordBytes)*Uef::Constants::DefaultBaseFrequency);}
        else if(value.kind==SequentialSegmentKind::Silence){if(data.size()!=FloatBytes)throw MediaError(DiagnosticCode::AtomTapData);seconds=std::bit_cast<float>(qFromLittleEndian<std::uint32_t>(data.data()));}
        else seconds=(data.size()>SerialPrefix?data.size()-SerialPrefix:0)*(1+DataBits+StopBits)/double(Baud);
        if(!std::isfinite(seconds)||seconds<0||seconds*rate>MaximumPcmSamples-samples.size())throw MediaError(DiagnosticCode::AtomTapePcmLimit);
        if(value.kind==SequentialSegmentKind::Carrier)Fsk::appendCarrier(samples,level,rate,Baud,Uef::Constants::AcornZeroFrequency,Uef::Constants::AcornOneFrequency,seconds,token);
        else if(value.kind==SequentialSegmentKind::Silence)TapePcm::appendSilence(samples,rate,seconds);
        else for(std::size_t at=SerialPrefix;at<data.size();++at)Fsk::appendFrame(samples,level,rate,Baud,Uef::Constants::AcornZeroFrequency,Uef::Constants::AcornOneFrequency,DataBits,StopBits,data[at],token);
    }
    const auto document=TapePcm::signalDocument(source(EncodedWave),std::string(WaveFormat),samples,rate,token);
    const auto& result=dynamic_cast<const SequentialImage&>(document->representation());
    auto value=result.segments()->front().fields();value.metadata[std::string(Uef::Constants::ReconstructionMetadataKey)]=Uef::Constants::TrueValue;
    value.metadata[std::string(TapePcm::Constants::RateKey)]=std::to_string(rate);value.metadata[std::string(TapePcm::Constants::BitsKey)]=std::to_string(TapePcm::Constants::DefaultBits);value.metadata[std::string(TapePcm::Constants::AlignKey)]=std::to_string(WordBytes);
    return std::make_shared<const SequentialImage>(result.logicalLength(),result.duration(),std::vector<SequentialSegment>{SequentialSegment(std::move(value))});
}
}
std::string_view AtomTapeEncoder::id()const noexcept{return DecoderId;}
std::span<const std::string> AtomTapeEncoder::formatIds()const{static const std::vector<std::string> ids{std::string(FormatId),std::string(UefFormat),std::string(WaveFormat)};return ids;}
std::span<const std::string> AtomTapeEncoder::machineIds()const{static const std::vector<std::string> ids{std::string(MachineId)};return ids;}
bool AtomTapeEncoder::canEncode(const SequentialEncodeRequest& request)const{
    if(!std::any_of(formatIds().begin(),formatIds().end(),[&](const auto& id){return sameIdentifier(request.targetFormatId(),id);})||!sameIdentifier(request.machineId(),MachineId)||!request.retainedSegments().empty())return false;
    int rate{};if(sameIdentifier(request.targetFormatId(),WaveFormat)&&!TapePcm::trySampleRate(request.parameters(),rate))return false;
    return AtomTap::canEncodeFiles(request.blocks());
}
std::future<std::shared_ptr<const SequentialImage>> AtomTapeEncoder::encodeAsync(SequentialEncodeRequest request,std::stop_token token)const{
    return std::async(std::launch::deferred,[request=std::move(request),token]{
        checkCancellation(token);if(!AtomTapeEncoder{}.canEncode(request))throw MediaError(DiagnosticCode::AtomTapFiles);
        const auto bytes=AtomTap::encode(request.blocks(),token);const auto document=AtomTap::Reader::readBytes(source(EncodedTap),bytes,token);
        const auto image=std::dynamic_pointer_cast<const SequentialImage>(document->representationOwner());
        if(!sameIdentifier(request.targetFormatId(),WaveFormat))return image;
        int rate{};TapePcm::trySampleRate(request.parameters(),rate);return wave(*image,rate,token);
    });
}
}
