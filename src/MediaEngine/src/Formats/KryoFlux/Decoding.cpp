#include "Internal.h"
#include <cmath>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::KryoFlux {
DecodedTrack readTrack(std::span<const std::uint8_t> data,std::stop_token token){
    using namespace Constants;checkCancellation(token);
    DecodedTrack result;std::vector<std::uint32_t> indexes;double clock=DefaultSampleClock;readOutOfBand(data,indexes,result.metadata,clock,token);
    if(clock<=0||!std::isfinite(clock))throw MediaError(DiagnosticCode::KryoClock);
    std::vector<std::vector<std::uint32_t>> segments(1);std::uint64_t overflow{};std::uint32_t streamPosition{};std::size_t indexNumber{};
    auto addFlux=[&](std::uint64_t ticks){
        if(!ticks)throw MediaError(DiagnosticCode::KryoInterval);
        const auto ns=std::round(double(ticks)*NanosecondsPerSecond/clock);
        if(!std::isfinite(ns)||ns<1||ns>double(MaximumDuration))throw MediaError(DiagnosticCode::KryoTiming);
        segments.back().push_back(std::uint32_t(ns));overflow=0;
    };
    for(std::size_t offset=0;offset<data.size();){checkCancellation(token);
        while(indexNumber<indexes.size()&&streamPosition>=indexes[indexNumber]){segments.emplace_back();++indexNumber;}
        const auto code=data[offset];
        if(code<=MaximumShortOpcode){require(data,offset,ShortBytes);addFlux(overflow+(std::uint32_t(code)<<ByteBits)+data[offset+1]);offset+=ShortBytes;streamPosition+=std::uint32_t(ShortBytes);}
        else if(code==Nop1||code==Nop2||code==Nop3){const auto count=opcodeBytes(code);require(data,offset,count);offset+=count;streamPosition+=std::uint32_t(count);}
        else if(code==Overflow16){overflow+=OverflowIncrement;++streamPosition;++offset;}
        else if(code==Flux3){require(data,offset,LongBytes);addFlux(overflow+(std::uint32_t(data[offset+1])<<ByteBits)+data[offset+LongLowByteOffset]);offset+=LongBytes;streamPosition+=std::uint32_t(LongBytes);}
        else if(code==OutOfBand){
            if(trailingFill(data,offset,token))break;
            require(data,offset,OutOfBandHeader);const auto type=data[offset+TypeOffset];const auto size=u16(data,offset+SizeOffset);require(data,offset+OutOfBandHeader,size);
            if((type==StreamInfo||type==StreamEnd)&&(size<IndexBytes||u32(data,offset+OutOfBandHeader)!=streamPosition))throw MediaError(DiagnosticCode::KryoPosition);
            offset+=OutOfBandHeader+size;if(type==EndOfFile)break;
        }else{addFlux(overflow+code);++streamPosition;++offset;}
    }
    const std::size_t begin=indexes.size()>1?1:0,end=indexes.size()>1?std::min(segments.size(),indexes.size()):segments.size();
    for(auto i=begin;i<end;++i){checkCancellation(token);auto& intervals=segments[i];if(intervals.empty())continue;std::uint64_t duration{};for(auto interval:intervals){duration+=interval;if(duration>MaximumDuration)throw MediaError(DiagnosticCode::KryoDuration);}
        result.revolutions.emplace_back(ResolutionNanoseconds,Flux::Revolution{std::uint32_t(duration),std::move(intervals)});
    }
    if(result.revolutions.empty())throw MediaError(DiagnosticCode::KryoNoRevolutions);return result;
}
}
