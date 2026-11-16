#include "Parser.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaEngine/DiagnosticKeys.h>
#include <QString>
#include <QtEndian>
namespace RetroMediaLab::MediaEngine::SimhTape {
namespace {
using namespace Constants;
SequentialSegment marker(std::int64_t position,SequentialSegmentKind kind,std::uint32_t word,std::string_view object) {
    SequentialSegmentFields value;value.position=position;value.kind=kind;value.direction=SequentialTravelDirection::Forward;
    value.metadata={{std::string(ObjectKindKey),std::string(object)},{std::string(MarkerValueKey),QString::number(word,HexRadix).rightJustified(HexWidth,QLatin1Char(ZeroPad)).toStdString()}};
    return SequentialSegment(std::move(value));
}
}
ParseResult parse(const RecognitionContext& context,bool createSegments,std::stop_token token) {
    checkCancellation(token);if(context.length()<WordSize)throw MediaError(DiagnosticCode::SimhData);
    ParseResult result;std::int64_t offset=0,position=0;
    while(offset<context.length()) {
        checkCancellation(token);if(context.length()-offset<WordSize)throw MediaError(DiagnosticCode::SimhData);
        const auto bytes=context.readAsync(offset,WordSize,token).get();const auto word=qFromLittleEndian<std::uint32_t>(bytes.data());offset+=WordSize;
        if(word==TapeMark||word==EraseGap||word==EndOfMedium) {
            if(createSegments)result.segments.push_back(marker(position++,word==EraseGap?SequentialSegmentKind::Silence:SequentialSegmentKind::TapeMark,word,word==TapeMark?TapeMarkKind:word==EraseGap?EraseGapKind:EndOfMediumKind));
            if(word==EndOfMedium){if(createSegments&&offset<context.length())result.messages.push_back({std::string(DiagnosticKeys::SimhTrailing),{std::to_string(context.length()-offset)},{}});break;}
            continue;
        }
        if(word&ReservedLengthBits)throw MediaError(DiagnosticCode::SimhData);const auto length=word&RecordLengthMask;if(!length)throw MediaError(DiagnosticCode::SimhData);
        const auto padded=(length+RecordAlignment-1)&~std::uint32_t(RecordAlignment-1);
        if(std::int64_t(padded)+WordSize>context.length()-offset)throw MediaError(DiagnosticCode::SimhData);
        const auto start=offset;offset+=padded;const auto tail=context.readAsync(offset,WordSize,token).get();offset+=WordSize;
        if(qFromLittleEndian<std::uint32_t>(tail.data())!=word)throw MediaError(DiagnosticCode::SimhData);
        if(!createSegments)continue;
        const auto hasError=bool(word&ErrorFlag);SequentialSegmentFields value;value.position=position++;value.kind=SequentialSegmentKind::DataBlock;value.length=length;value.direction=SequentialTravelDirection::Forward;value.dataRange=DataRange(start,length,DataRangeKind::Stored,context.dataSource(),start);
        value.metadata={{std::string(ObjectKindKey),std::string(DataRecordKind)},{std::string(RecordErrorKey),std::string(hasError?True:False)}};
        result.segments.emplace_back(std::move(value));if(hasError)result.messages.push_back({std::string(DiagnosticKeys::SimhRecordError),{std::to_string(position-1)},{}});
    }
    return result;
}
}
