#include "Signal.h"
#include "SignalConstants.h"
#include "../Uef/Constants.h"
#include "../../../Models/Blocks/Internal.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <QString>
#include <bit>
#include <charconv>
#include <cmath>
#include <limits>
namespace RetroMediaLab::MediaEngine::TapeSignal {
namespace {
using namespace TapeSignalConstants;
std::string text(const SequentialSegmentFields& value,std::string_view key){
    const auto found=value.metadata.find(std::string(key));
    if(found==value.metadata.end())throw MediaError(DiagnosticCode::UefSignalData);
    return found->second;
}
int integer(const SequentialSegmentFields& value,std::string_view key){
    bool valid=false;const auto result=QString::fromUtf8(text(value,key)).toInt(&valid);
    if(!valid)throw MediaError(DiagnosticCode::UefSignalData);return result;
}
class Synthesizer {
public:
    explicit Synthesizer(std::stop_token stop):token(stop){}
    std::vector<std::int16_t> samples;
    void pulse(std::int16_t value,double duration){
        checkCancellation(token);
        if(!std::isfinite(duration)||duration<0)throw MediaError(DiagnosticCode::UefSignalPulse);
        const auto total=remainder+duration;
        const auto rounded=TapePcm::roundEven(total);
        if(!std::isfinite(rounded)||rounded<0||rounded>MaximumSamples-samples.size())throw MediaError(DiagnosticCode::UefSignalLimit);
        remainder=total-rounded;samples.insert(samples.end(),static_cast<std::size_t>(rounded),value);
    }
    void carrier(unsigned cycles){for(unsigned half=0;half<cycles*PairWidth;++half){pulse(level,SampleRate/(FastHalves*frequency));level=static_cast<std::int16_t>(-level);}}
    void bit(bool value){
        if(baud!=SlowBaud&&baud!=FastBaud)throw MediaError(DiagnosticCode::UefSignalBaud,{std::to_string(baud)});
        const auto halves=(baud==SlowBaud?SlowFactor:1)*(value?FastHalves:SlowHalves);
        for(int half=0;half<halves;++half){pulse(level,SampleRate/(PairWidth*frequency*(value?PairWidth:1)));level=static_cast<std::int16_t>(-level);}
    }
    void frame(std::uint8_t value){bit(false);for(int at=0;at<ByteBits;++at)bit((value&(1<<at))!=0);bit(true);}
    std::int16_t level=-Uef::Constants::PcmAmplitude;
    double frequency=Uef::Constants::DefaultBaseFrequency;
    int baud=Uef::Constants::DefaultBaudRate;
private:
    double remainder{};
    std::stop_token token;
};
}
UefResult fromUef(const SequentialImage& image,std::stop_token token){
    checkCancellation(token);
    using namespace TapeSignalConstants;using namespace Uef::Constants;
    Synthesizer synth(token);std::map<std::int64_t,int> positions;std::vector<std::int64_t> order;
    if(image.segments())for(const auto& segment:*image.segments()){
        checkCancellation(token);const auto& value=segment.fields();const auto idText=QString::fromUtf8(text(value,ChunkIdMetadataKey)).trimmed().toStdString();unsigned id{};
        const auto parsed=std::from_chars(idText.data(),idText.data()+idText.size(),id,HexBase);
        if(parsed.ec!=std::errc{}||parsed.ptr!=idText.data()+idText.size()||id>std::numeric_limits<std::uint16_t>::max())throw MediaError(DiagnosticCode::UefSignalData);
        if(id<FirstSignalChunk||id==PositionMarker||id==TapeSetInformation||id==TapeSide)continue;
        const auto data=readStoredRange(value.dataRange?&*value.dataRange:nullptr,token).value_or(std::vector<std::uint8_t>{});
        bool valid=false;synth.frequency=QString::fromUtf8(text(value,BaseFrequencyMetadataKey)).toDouble(&valid);if(!valid)throw MediaError(DiagnosticCode::UefSignalData);synth.baud=integer(value,BaudRateMetadataKey);
        const auto mark=[&]{if(!positions.contains(value.position))order.push_back(value.position);positions[value.position]=static_cast<int>(synth.samples.size());};
        switch(id){
        case ImplicitData:
            mark();for(auto byte:data)synth.frame(byte);break;
        case ExplicitData:{
            if(data.empty()||data[0]<ByteBits||data[0]>ExplicitMaximum)throw MediaError(DiagnosticCode::UefSignalData);
            if(data.size()*ByteBits<data[0])break;
            const auto bits=data.size()*ByteBits-data[0];
            for(std::size_t at=0;at<bits;++at)synth.bit((data[1+at/ByteBits]&(1<<(at&BitMask)))!=0);break;
        }
        case DefinedData:{
            mark();if(data.size()<SerialPrefix||data[0]<1||data[0]>ByteBits||(data[1]!=NoParity&&data[1]!=EvenParity&&data[1]!=OddParity))throw MediaError(DiagnosticCode::UefSignalData);
            const auto stops=std::bit_cast<std::int8_t>(data[PairWidth]);
            for(std::size_t at=SerialPrefix;at<data.size();++at){
                synth.bit(false);bool parity=false;
                for(int bit=0;bit<data[0];++bit){const bool current=(data[at]&(1<<bit))!=0;synth.bit(current);parity=parity!=current;}
                if(data[1]!=NoParity)synth.bit(parity!=(data[1]==OddParity));
                for(int stop=0;stop<std::abs(int(stops));++stop)synth.bit(true);
                if(stops<0)synth.carrier(1);
            }break;
        }
        case ImplicitAtomData:
            for(std::size_t at=0;at<data.size();++at){synth.frame(data[at]);if(at+1<data.size())synth.carrier(1);}break;
        case CarrierTone:
            if(data.size()!=PairWidth)throw MediaError(DiagnosticCode::UefSignalData);synth.carrier(qFromLittleEndian<std::uint16_t>(data.data()));break;
        case CarrierToneWithDummy:
            if(data.size()!=FloatWidth)throw MediaError(DiagnosticCode::UefSignalData);synth.carrier(qFromLittleEndian<std::uint16_t>(data.data()));synth.frame(DummyByte);synth.carrier(qFromLittleEndian<std::uint16_t>(data.data()+PairWidth));break;
        case IntegerGap:{
            if(data.size()!=PairWidth)throw MediaError(DiagnosticCode::UefSignalData);const auto count=qFromLittleEndian<std::uint16_t>(data.data());if(count)synth.pulse(0,SampleRate/(PairWidth*count*synth.frequency));break;
        }
        case FloatingGap:
            if(data.size()!=FloatWidth)throw MediaError(DiagnosticCode::UefSignalData);synth.pulse(0,SampleRate*double(std::bit_cast<float>(qFromLittleEndian<std::uint32_t>(data.data()))));break;
        case SecurityCycles:{
            if(data.size()<SecurityHeader)throw MediaError(DiagnosticCode::UefSignalData);
            const auto cycles=std::uint32_t(data[0])|(std::uint32_t(data[1])<<ByteBits)|(std::uint32_t(data[PairWidth])<<SignalBits);
            if(data.size()<SecurityHeader+(cycles+BitMask)/ByteBits)throw MediaError(DiagnosticCode::UefSignalData);
            const auto phase=integer(value,PhaseMetadataKey);const auto positive=phase<HalfPhase?PcmAmplitude:static_cast<std::int16_t>(-PcmAmplitude);
            for(std::uint32_t at=0;at<cycles;++at){const bool fast=(data[SecurityHeader+at/ByteBits]&(MostSignificantBit>>(at&BitMask)))!=0;const auto half=SampleRate/(PairWidth*synth.frequency*(fast?PairWidth:1));if(!(at==0&&data[SecurityFirstFlag]==PartialCycle))synth.pulse(positive,half);if(!(at==cycles-1&&data[SecurityLastFlag]==PartialCycle))synth.pulse(static_cast<std::int16_t>(-positive),half);}break;
        }
        case BaseFrequency:case BaudRate:break;
        case PhaseChange:
            if(data.size()!=PairWidth)throw MediaError(DiagnosticCode::UefSignalData);synth.level=qFromLittleEndian<std::uint16_t>(data.data())%FullPhase<HalfPhase?PcmAmplitude:static_cast<std::int16_t>(-PcmAmplitude);break;
        default:throw MediaError(DiagnosticCode::UefSignalUnsupported,{idText});
        }
    }
    std::vector<int> blockPositions;for(const auto position:order)blockPositions.push_back(positions.at(position));
    return {MediaTapeSignal(std::move(synth.samples),SampleRate,std::move(blockPositions)),std::move(positions)};
}
}
