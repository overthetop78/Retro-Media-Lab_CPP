#include "Reader.h"
#include "Constants.h"
#include "../../../Sources/RandomAccessSources.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaEngine/DiagnosticKeys.h>
#include <QtEndian>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::VocTape {
namespace {
using namespace Constants;
std::uint32_t length24(std::span<const std::uint8_t> bytes,std::size_t& position) {
    if(position>bytes.size()||LengthBytes>bytes.size()-position)throw MediaError(DiagnosticCode::VocData);
    std::uint32_t length=0;for(int i=0;i<LengthBytes;++i)length|=std::uint32_t(bytes[position++])<<(i*ByteBits);return length;
}
void require(std::span<const std::uint8_t> bytes,std::size_t position,std::size_t count) {
    if(position>bytes.size()||count>bytes.size()-position)throw MediaError(DiagnosticCode::VocData);
}
}
const ReaderCapabilities& Reader::capabilities()const {
    static const ReaderCapabilities value{{std::string(FormatId)},{std::string(Extension)},{{Signature.begin(),Signature.end()}},{},{MediaKind::Tape},{RepresentationKind::Sequential}};return value;
}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token] {
        checkCancellation(token);const auto requested=context.requestedFormatId();if((requested&&!sameIdentifier(*requested,FormatId))||context.length()<HeaderSize)return false;
        const auto header=context.readHeaderAsync(int(Signature.size()),token).get();return std::equal(Signature.begin(),Signature.end(),header.begin());
    });
}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token] {
        const auto bytes=context.readBytesAsync(token).get();
        if(bytes.size()<HeaderSize||!std::equal(Signature.begin(),Signature.end(),bytes.begin()))throw MediaError(DiagnosticCode::VocData);
        std::size_t position=qFromLittleEndian<std::uint16_t>(bytes.data()+DataOffset);
        if(position<HeaderSize||position>=bytes.size())throw MediaError(DiagnosticCode::VocData);
        std::optional<std::uint8_t> timeConstant;std::vector<std::uint8_t> samples;bool terminated=false;
        const auto rate=[&](std::uint8_t current){if(!current||(timeConstant&&*timeConstant!=current))throw MediaError(DiagnosticCode::VocProfile);timeConstant=current;};
        const auto append=[&](std::size_t count){require(bytes,position,count);if(count>std::size_t(TapePcm::Constants::MaximumBuffer)/sizeof(std::int16_t)-samples.size())throw MediaError(DiagnosticCode::VocData);samples.insert(samples.end(),bytes.begin()+position,bytes.begin()+position+count);position+=count;};
        while(position<bytes.size()) {
            checkCancellation(token);const auto id=bytes[position++];if(id==Terminator){terminated=true;break;}
            switch(id) {
            case Sound:{const auto length=length24(bytes,position);require(bytes,position,length);if(length<SoundFields)throw MediaError(DiagnosticCode::VocData);const auto current=bytes[position++];if(bytes[position++]!=0)throw MediaError(DiagnosticCode::VocProfile);rate(current);append(length-SoundFields);break;}
            case Continuation:{const auto length=length24(bytes,position);append(length);break;}
            // Legacy C# / Caprice32 layout: these fixed-size records have no 24-bit length field.
            case Silence:{require(bytes,position,SilenceBytes);const auto count=std::uint32_t(qFromLittleEndian<std::uint16_t>(bytes.data()+position))+1;rate(bytes[position+SoundFields]);position+=SilenceBytes;if(count>std::size_t(TapePcm::Constants::MaximumBuffer)/sizeof(std::int16_t)-samples.size())throw MediaError(DiagnosticCode::VocData);samples.insert(samples.end(),count,SampleBias);break;}
            case Marker:require(bytes,position,MarkerBytes);position+=MarkerBytes;break;
            case Text:{const auto length=length24(bytes,position);require(bytes,position,length);position+=length;break;}
            default:throw MediaError(DiagnosticCode::VocProfile);
            }
        }
        if(!timeConstant||samples.empty())throw MediaError(DiagnosticCode::VocData);
        const auto sampleRate=int(TapePcm::roundEven(double(RateNumerator)/(RateBase-*timeConstant)));
        std::vector<std::uint8_t> pcm(samples.size()*sizeof(std::int16_t));
        for(std::size_t i=0;i<samples.size();++i){checkCancellation(token);qToLittleEndian(std::int16_t((int(samples[i])-SampleBias)*(1<<ByteBits)),pcm.data()+i*sizeof(std::int16_t));}
        const auto duration=SequentialTime(std::int64_t(double(samples.size())/sampleRate*SequentialTicksPerSecond));
        SequentialSegmentFields value;value.kind=SequentialSegmentKind::Samples;value.length=std::int64_t(samples.size());value.start=SequentialTime{};value.duration=duration;value.channelNumber=0;value.direction=SequentialTravelDirection::Forward;value.dataRange=DataRange(0,pcm.size(),DataRangeKind::Stored,memorySource(pcm));
        value.metadata={{std::string(ChannelKey),std::to_string(0)},{std::string(InterleavedKey),std::string(TapePcm::Constants::TrueValue)},{std::string(TapePcm::Constants::AlignKey),std::to_string(sizeof(std::int16_t))}};
        TapePcm::Parameters metadata{{std::string(TapePcm::Constants::ChannelsKey),std::to_string(1)},{std::string(TapePcm::Constants::RateKey),std::to_string(sampleRate)},{std::string(TapePcm::Constants::BitsKey),std::to_string(SampleBits)},{std::string(TapePcm::Constants::AlignKey),std::to_string(sizeof(std::int16_t))},{std::string(TimeConstantKey),std::to_string(*timeConstant)}};
        std::vector<MediaFileSystems::MessageRecipe> messages;if(!terminated)messages.push_back({std::string(DiagnosticKeys::VocTerminator),{}, {}});
        return std::make_shared<const MediaDocument>(context.source(),std::string(FormatId),MediaKind::Tape,std::make_shared<SequentialImage>(pcm.size(),duration,std::vector<SequentialSegment>{SequentialSegment(std::move(value))}),std::vector<MediaFileSystems::VolumeDescriptor>{},std::move(messages),std::move(metadata));
    });
}
}
