#include "Blocks.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <array>
#include <bit>
namespace RetroMediaLab::MediaEngine::Tzx {
namespace {
using namespace Constants;
std::uint32_t number(const RecognitionContext& context,std::int64_t offset,int width,std::stop_token token){
    checkCancellation(token);if(offset<0||offset>context.length()||width>context.length()-offset)throw MediaError(DiagnosticCode::TzxBlock);
    std::array<std::uint8_t,DwordWidth> bytes{};context.dataSource()->readExactly(offset,std::span(bytes).first(width),token);std::uint32_t value=0;
    for(int index=0;index<width;++index)value|=std::uint32_t(bytes[index])<<(index*ByteBits);return value;
}
void validate(const std::vector<BlockInfo>& blocks){
    std::size_t loops=0;bool hasCall=false;
    for(std::size_t index=0;index<blocks.size();++index){
        const auto& block=blocks[index];const auto target=[&](std::int16_t relative){const auto destination=static_cast<std::int64_t>(index)+relative;if(destination<0||destination>=static_cast<std::int64_t>(blocks.size()))throw MediaError(DiagnosticCode::TzxTarget,{std::to_string(index),std::to_string(destination)});};
        if(block.jump)target(*block.jump);if(block.id==CallSequence){hasCall=true;for(const auto call:block.calls)target(call);}
        if(block.id==LoopStart)++loops;else if(block.id==LoopEnd){if(!loops)throw MediaError(DiagnosticCode::TzxControlFlow);--loops;}
        else if(block.id==Return&&!hasCall)throw MediaError(DiagnosticCode::TzxControlFlow);
    }
    if(loops)throw MediaError(DiagnosticCode::TzxControlFlow);
}
}
std::vector<BlockInfo> readBlocks(const RecognitionContext& context,std::stop_token token){
    std::vector<BlockInfo> blocks;std::int64_t cursor=HeaderSize;
    while(cursor<context.length()){
        checkCancellation(token);BlockInfo block{};block.id=static_cast<std::uint8_t>(number(context,cursor++,ByteWidth,token));block.offset=cursor;
        const auto at=[&](int offset,int width){return number(context,cursor+offset,width,token);};
        switch(block.id){
        case StandardSpeedData:block.length=DwordWidth+at(WordWidth,WordWidth);break;
        case TurboSpeedData:block.length=TurboHeaderSize+std::int64_t(at(TurboLengthOffset,TripleWidth));break;
        case PureTone:block.length=DwordWidth;break;
        case PulseSequence:block.length=ByteWidth+at(0,ByteWidth)*WordWidth;break;
        case PureData:block.length=PureHeaderSize+std::int64_t(at(PureLengthOffset,TripleWidth));break;
        case DirectRecording:block.length=DirectHeaderSize+std::int64_t(at(DirectLengthOffset,TripleWidth));break;
        case CswRecording:case GeneralizedData:case StopIf48K:case SetSignalLevel:block.length=DwordWidth+std::int64_t(at(0,DwordWidth));break;
        case Pause:case LoopStart:block.length=WordWidth;break;
        case GroupStart:case TextDescription:block.length=ByteWidth+at(0,ByteWidth);break;
        case GroupEnd:case LoopEnd:case Return:break;
        case Jump:block.length=WordWidth;block.jump=std::bit_cast<std::int16_t>(static_cast<std::uint16_t>(at(0,WordWidth)));break;
        case CallSequence:{
            const auto count=at(0,WordWidth);block.length=WordWidth+count*WordWidth;
            if(block.length>context.length()-cursor)throw MediaError(DiagnosticCode::TzxBlock);
            block.calls.reserve(count);for(std::uint32_t index=0;index<count;++index)block.calls.push_back(std::bit_cast<std::int16_t>(static_cast<std::uint16_t>(at(WordWidth+index*WordWidth,WordWidth))));break;
        }
        case Select:case ArchiveInfo:block.length=WordWidth+at(0,WordWidth);break;
        case Message:block.length=WordWidth+at(ByteWidth,ByteWidth);break;
        case HardwareType:block.length=ByteWidth+at(0,ByteWidth)*TripleWidth;break;
        case CustomInfo:block.length=CustomHeaderSize+std::int64_t(at(CustomLengthOffset,DwordWidth));break;
        case Glue:block.length=GlueLength;break;
        default:block.length=DwordWidth+std::int64_t(at(0,DwordWidth));break;
        }
        if(block.length>context.length()-cursor)throw MediaError(DiagnosticCode::TzxBlock);cursor+=block.length;blocks.push_back(std::move(block));
    }
    validate(blocks);return blocks;
}
SequentialSegmentKind segmentKind(std::uint8_t id){
    switch(id){
    case StandardSpeedData:case TurboSpeedData:case PureData:case DirectRecording:case CswRecording:case GeneralizedData:return SequentialSegmentKind::DataBlock;
    case PureTone:case PulseSequence:return SequentialSegmentKind::Pulse;
    case Pause:return SequentialSegmentKind::Silence;
    case GroupStart:case GroupEnd:case Glue:return SequentialSegmentKind::TapeMark;
    default:return SequentialSegmentKind::Unknown;
    }
}
}
