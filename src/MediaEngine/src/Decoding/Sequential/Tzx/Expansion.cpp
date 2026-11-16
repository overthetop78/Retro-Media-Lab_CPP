#include "Expansion.h"
#include "../../../Formats/Tape/Tzx/Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <bit>
#include <limits>
namespace RetroMediaLab::MediaEngine::TzxSignal {
namespace {
using namespace Tzx::Constants;
using Metadata=std::map<std::string,std::string>;
void require(std::span<const std::uint8_t> bytes,std::size_t count){if(bytes.size()<count)throw MediaError(DiagnosticCode::TzxBlock);}
std::uint16_t word(std::span<const std::uint8_t> bytes,int offset=0){require(bytes,offset+WordWidth);return qFromLittleEndian<std::uint16_t>(bytes.data()+offset);}
int triple(std::span<const std::uint8_t> bytes,int offset){require(bytes,offset+TripleWidth);return bytes[offset]|(bytes[offset+1]<<ByteBits)|(bytes[offset+WordWidth]<<(WordWidth*ByteBits));}
SequentialTime duration(std::int64_t states){return SequentialTime(static_cast<std::int64_t>((states/double(TStatesPerSecond))*SequentialTicksPerSecond));}
Metadata metadata(std::int64_t position,std::string_view role){return {{std::string(SourcePositionKey),std::to_string(position)},{std::string(RoleKey),std::string(role)}};}
DataRange slice(const SequentialSegmentFields& source,int offset,int length){
    if(!source.dataRange||offset<0||length<=0||offset>source.dataRange->length()||length>source.dataRange->length()-offset)throw MediaError(DiagnosticCode::TzxSignalPayload);
    const auto& range=*source.dataRange;
    if(range.address()>std::numeric_limits<std::int64_t>::max()-offset||range.sourceOffset()>std::numeric_limits<std::int64_t>::max()-offset)throw MediaError(DiagnosticCode::TzxSignalLimit);
    return DataRange(range.address()+offset,length,range.kind(),range.source(),range.sourceOffset()+offset);
}
std::int64_t bits(int length,std::uint8_t used){if(!used||used>FullLastByteBitCount)throw MediaError(DiagnosticCode::TzxSignalBits);return std::int64_t(length-1)*ByteBits+used;}
}
void Expansion::append(SequentialSegmentFields value){
    checkCancellation(token_);const auto ticks=value.duration?value.duration->count():0;
    if(ticks<0||elapsed_.count()>std::numeric_limits<std::int64_t>::max()-ticks||value.length.value_or(0)>std::numeric_limits<std::int64_t>::max()-length_)throw MediaError(DiagnosticCode::TzxSignalLimit);
    value.position=static_cast<std::int64_t>(segments_.size());if(value.duration)value.start=elapsed_;
    segments_.emplace_back(std::move(value));elapsed_+=SequentialTime(ticks);length_+=segments_.back().fields().length.value_or(0);
}
void Expansion::pulse(std::uint16_t states,int count,std::int64_t source,std::string_view role){
    if(!states||!count)return;SequentialSegmentFields value;value.kind=SequentialSegmentKind::Pulse;value.length=count;value.duration=duration(std::int64_t(states)*count);value.direction=SequentialTravelDirection::Forward;
    value.metadata=metadata(source,role);value.metadata[std::string(PulseStatesKey)]=std::to_string(states);value.metadata[std::string(PulseCountKey)]=std::to_string(count);append(std::move(value));
}
void Expansion::pause(std::uint16_t milliseconds,std::int64_t source){
    SequentialSegmentFields value;value.kind=milliseconds?SequentialSegmentKind::Silence:SequentialSegmentKind::TapeMark;value.metadata=metadata(source,milliseconds?PauseRole:StopRole);
    if(milliseconds){value.duration=SequentialTime(std::int64_t(milliseconds)*(SequentialTicksPerSecond/MillisecondsPerSecond));value.direction=SequentialTravelDirection::Forward;}append(std::move(value));
}
void Expansion::data(const SequentialSegmentFields& source,std::span<const std::uint8_t> payload,int offset,int length,std::uint8_t used,std::uint16_t zero,std::uint16_t one){
    if(!length)return;bits(length,used);std::int64_t states=0;
    for(int index=0;index<length;++index){
        checkCancellation(token_);const auto validBits=index+1==length?used:FullLastByteBitCount;
        const auto ones=std::popcount(static_cast<unsigned>(payload[offset+index]>>(ByteBits-validBits)));
        states+=PulsePair*(std::int64_t(ones)*one+std::int64_t(validBits-ones)*zero);
    }
    if(!states)throw MediaError(DiagnosticCode::TzxSignalPayload);
    SequentialSegmentFields value;value.kind=SequentialSegmentKind::DataBlock;value.length=length;value.duration=duration(states);value.direction=SequentialTravelDirection::Forward;value.dataRange=slice(source,offset,length);value.metadata=metadata(source.position,DataRole);append(std::move(value));
}
void Expansion::block(std::uint8_t id,const SequentialSegmentFields& source,std::span<const std::uint8_t> payload){
    checkCancellation(token_);
    switch(id){
    case StandardSpeedData:{
        require(payload,StandardHeaderSize);const auto length=word(payload,StandardLengthOffset);require(payload,StandardHeaderSize+length);
        const auto pilots=length&&payload[StandardHeaderSize]<HeaderFlagBoundary?StandardHeaderPilotPulseCount:StandardDataPilotPulseCount;
        pulse(StandardPilotPulseTStates,pilots,source.position,PilotRole);pulse(StandardSyncFirstPulseTStates,1,source.position,SyncRole);pulse(StandardSyncSecondPulseTStates,1,source.position,SyncRole);
        data(source,payload,StandardHeaderSize,length,FullLastByteBitCount,StandardZeroPulseTStates,StandardOnePulseTStates);pause(word(payload),source.position);break;
    }
    case TurboSpeedData:{
        require(payload,TurboHeaderSize);const auto length=triple(payload,TurboLengthOffset);require(payload,TurboHeaderSize+length);
        pulse(word(payload),word(payload,TurboPilotCountOffset),source.position,PilotRole);pulse(word(payload,TurboSyncFirstOffset),1,source.position,SyncRole);pulse(word(payload,TurboSyncSecondOffset),1,source.position,SyncRole);
        data(source,payload,TurboHeaderSize,length,payload[TurboUsedBitsOffset],word(payload,TurboZeroOffset),word(payload,TurboOneOffset));pause(word(payload,TurboPauseOffset),source.position);break;
    }
    case PureTone:require(payload,DwordWidth);pulse(word(payload),word(payload,WordWidth),source.position,ToneRole);break;
    case PulseSequence:
        require(payload,ByteWidth);require(payload,ByteWidth+payload.front()*WordWidth);
        for(int index=0;index<payload.front();++index)pulse(word(payload,ByteWidth+index*WordWidth),1,source.position,SequenceRole);break;
    case PureData:{
        require(payload,PureHeaderSize);const auto length=triple(payload,PureLengthOffset);require(payload,PureHeaderSize+length);
        data(source,payload,PureHeaderSize,length,payload[PureUsedBitsOffset],word(payload),word(payload,PureOneOffset));pause(word(payload,PurePauseOffset),source.position);break;
    }
    case DirectRecording:{
        require(payload,DirectHeaderSize);const auto length=triple(payload,DirectLengthOffset);require(payload,DirectHeaderSize+length);
        if(length){
            const auto count=bits(length,payload[DirectUsedBitsOffset]);const auto states=word(payload);if(!states)throw MediaError(DiagnosticCode::TzxSignalPayload);
            SequentialSegmentFields value;value.kind=SequentialSegmentKind::Samples;value.length=count;value.duration=duration(states*count);value.direction=SequentialTravelDirection::Forward;value.dataRange=slice(source,DirectHeaderSize,length);value.metadata=metadata(source.position,DirectRole);append(std::move(value));
        }
        pause(word(payload,DirectPauseOffset),source.position);break;
    }
    case Pause:require(payload,WordWidth);pause(word(payload),source.position);break;
    default:break;
    }
}
std::shared_ptr<const SequentialImage> Expansion::finish(){checkCancellation(token_);return std::make_shared<const SequentialImage>(length_,elapsed_.count()?std::optional(elapsed_):std::nullopt,std::move(segments_));}
}
