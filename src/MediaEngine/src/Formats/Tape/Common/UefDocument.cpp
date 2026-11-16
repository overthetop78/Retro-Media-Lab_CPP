#include "Signal.h"
#include "SignalConstants.h"
#include "../Uef/Constants.h"
#include "../../../Sources/RandomAccessSources.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <QString>
#include <algorithm>
#include <array>
#include <bit>
#include <charconv>
namespace RetroMediaLab::MediaEngine::TapeSignal {
namespace {
using namespace TapeSignalConstants;
int polarity(std::int16_t value){return (value>0)-(value<0);}
std::string number(double value){std::array<char,NumberBuffer> bytes{};const auto result=std::to_chars(bytes.data(),bytes.data()+bytes.size(),value);if(result.ec!=std::errc{})throw MediaError(DiagnosticCode::UefSignalData);return {bytes.data(),result.ptr};}
std::vector<std::uint8_t> floating(float value){std::vector<std::uint8_t> bytes(FloatWidth);qToLittleEndian(std::bit_cast<std::uint32_t>(value),bytes.data());return bytes;}
std::vector<std::uint8_t> cycles(std::size_t count){
    std::vector<std::uint8_t> bytes(SecurityHeader+(count+BitMask)/ByteBits);
    for(int i=0;i<CountBytes;++i)bytes[i]=static_cast<std::uint8_t>(count>>(i*ByteBits));
    bytes[SecurityFirstFlag]=bytes[SecurityLastFlag]=WholeCycle;return bytes;
}
}
std::shared_ptr<const MediaDocument> toUef(std::shared_ptr<const MediaSource> source,const MediaTapeSignal& signal,std::stop_token token){
    checkCancellation(token);
    using namespace TapeSignalConstants;
    namespace U=Uef::Constants;
    const auto samples=signal.samples();const auto rate=signal.sampleRate();
    std::vector<SequentialSegment> segments;double baseFrequency=U::DefaultBaseFrequency;
    const auto chunk=[&](std::uint16_t id,std::vector<std::uint8_t> bytes){
        SequentialSegmentFields value;value.position=segments.size();value.kind=SequentialSegmentKind::Pulse;
        value.dataRange=DataRange(0,bytes.size(),DataRangeKind::Stored,memorySource(bytes));
        value.metadata={{std::string(U::ChunkIdMetadataKey),QString::number(id,HexBase).rightJustified(ChunkIdDigits,DigitZero).toStdString()},
            {std::string(U::BaseFrequencyMetadataKey),number(baseFrequency)},
            {std::string(U::BaudRateMetadataKey),std::to_string(U::DefaultBaudRate)},
            {std::string(U::PhaseMetadataKey),std::to_string(0)}};
        segments.emplace_back(std::move(value));
    };
    const auto frequency=[&](float value){baseFrequency=value;chunk(U::BaseFrequency,floating(value));};
    chunk(U::PhaseChange,std::vector<std::uint8_t>(PairWidth));
    std::size_t offset{};
    while(offset<samples.size()){
        checkCancellation(token);const auto sign=polarity(samples[offset]);auto end=offset+1;
        while(end<samples.size()&&polarity(samples[end])==sign){checkCancellation(token);++end;}
        const auto duration=end-offset;
        if(sign>0&&rate%HighRate==0){
            std::vector<bool> bits;auto cursor=offset;
            while(cursor<samples.size()&&bits.size()<MaximumCycles){
                checkCancellation(token);auto positiveEnd=cursor;
                while(positiveEnd<samples.size()&&samples[positiveEnd]>0)++positiveEnd;
                const auto width=positiveEnd-cursor;
                if(width!=static_cast<std::size_t>(rate/LowRate)&&width!=static_cast<std::size_t>(rate/HighRate))break;
                auto negativeEnd=positiveEnd;while(negativeEnd<samples.size()&&samples[negativeEnd]<0)++negativeEnd;
                if(negativeEnd-positiveEnd!=width)break;
                bits.push_back(width==static_cast<std::size_t>(rate/HighRate));cursor=negativeEnd;
            }
            if(!bits.empty()){
                frequency(U::DefaultBaseFrequency);auto bytes=cycles(bits.size());
                for(std::size_t bit=0;bit<bits.size();++bit)if(bits[bit])bytes[SecurityHeader+bit/ByteBits]|=static_cast<std::uint8_t>(MostSignificantBit>>(bit&BitMask));
                chunk(U::SecurityCycles,std::move(bytes));offset=cursor;continue;
            }
        }
        if(sign==0)chunk(U::FloatingGap,floating(static_cast<float>(duration)/rate));
        else{
            frequency(static_cast<float>(rate)/(PairWidth*duration));std::size_t pairs{};
            if(sign>0){
                auto cursor=offset;
                while(pairs<MaximumCycles&&duration<=(samples.size()-cursor)/PairWidth){
                    checkCancellation(token);bool complete=true;
                    for(std::size_t sample=0;sample<PairWidth*duration;++sample)if(polarity(samples[cursor+sample])!=(sample<duration?1:-1)){complete=false;break;}
                    if(!complete)break;++pairs;cursor+=PairWidth*duration;
                }
                if(pairs)end=cursor;
            }
            if(!pairs){auto bytes=cycles(1);bytes[SecurityFirstFlag]=sign<0?PartialCycle:WholeCycle;bytes[SecurityLastFlag]=sign>0?PartialCycle:WholeCycle;chunk(U::SecurityCycles,std::move(bytes));}
            else chunk(U::SecurityCycles,cycles(pairs));
        }
        offset=end;
    }
    return std::make_shared<const MediaDocument>(std::move(source),std::string(U::FormatId),MediaKind::Tape,
        std::make_shared<SequentialImage>(std::nullopt,std::nullopt,std::move(segments)),
        std::vector<MediaFileSystems::VolumeDescriptor>{},std::vector<MediaFileSystems::MessageRecipe>{},
        std::map<std::string,std::string>{{std::string(U::GzipMetadataKey),std::string(U::TrueValue)}});
}
}
