#include "Reader.h"
#include "Constants.h"
#include "Compression.h"
#include "../../../Sources/RandomAccessSources.h"
#include "../Common/Pcm.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaEngine/DiagnosticKeys.h>
#include <QtEndian>
#include <QString>
#include <algorithm>
#include <bit>
#include <charconv>
#include <cmath>
#include <limits>
namespace RetroMediaLab::MediaEngine::Uef {
namespace {
using namespace Constants;
bool supported(std::span<const std::uint8_t> bytes){return bytes.size()>=HeaderSize&&std::equal(Signature.begin(),Signature.end(),bytes.begin())&&bytes[MajorOffset]==SupportedMajorVersion&&bytes[MinorOffset]<=SupportedMinorVersion;}
bool understood(std::uint16_t id){return std::find(UnderstoodTapeChunks.begin(),UnderstoodTapeChunks.end(),id)!=UnderstoodTapeChunks.end();}
std::string hex(std::uint16_t id){return QString::number(id,HexRadix).rightJustified(HexWidth,DigitZero).toStdString();}
std::string number(double value){std::array<char,NumberBuffer> buffer{};const auto result=std::to_chars(buffer.data(),buffer.data()+buffer.size(),value);if(result.ec!=std::errc{})throw MediaError(DiagnosticCode::UefValue);return {buffer.data(),result.ptr};}
SequentialSegmentKind kind(std::uint16_t id){switch(id){case ImplicitData:case MultiplexedData:case ExplicitData:case DefinedData:case ImplicitAtomData:return SequentialSegmentKind::DataBlock;case CarrierTone:case CarrierToneWithDummy:return SequentialSegmentKind::Carrier;case IntegerGap:case FloatingGap:return SequentialSegmentKind::Silence;case SecurityCycles:return SequentialSegmentKind::Pulse;case PhaseChange:case BaseFrequency:case BaudRate:case PositionMarker:case TapeSetInformation:case TapeSide:return SequentialSegmentKind::TapeMark;default:return SequentialSegmentKind::Unknown;}}
}
const ReaderCapabilities& Reader::capabilities()const{static const ReaderCapabilities caps{{std::string(FormatId)},{std::string(Extension)},{{Signature.begin(),Signature.end()}},{},{MediaKind::Tape},{RepresentationKind::Sequential}};return caps;}
bool Reader::supportsFormatId(std::string_view id)const{return sameIdentifier(id,FormatId);}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{if(context.requestedFormatId()&&!Reader{}.supportsFormatId(*context.requestedFormatId()))return false;try{bool gzip=false;return supported(logicalBytes(context,gzip,true,token));}catch(const MediaError& error){if(error.diagnostic().code==DiagnosticCode::UefCompression||error.diagnostic().code==DiagnosticCode::UefSize)return false;throw;}});}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{
    bool gzip=false;auto bytes=logicalBytes(context,gzip,false,token);if(!supported(bytes))throw MediaError(DiagnosticCode::UefHeader);const auto data=memorySource(bytes);std::vector<SequentialSegment> segments;std::vector<MediaFileSystems::MessageRecipe> messages;std::size_t offset=HeaderSize;double baseFrequency=DefaultBaseFrequency;int baudRate=DefaultBaudRate;std::uint16_t phase=InitialPhase;
    while(offset<bytes.size()){checkCancellation(token);if(bytes.size()-offset<ChunkHeaderSize)throw MediaError(DiagnosticCode::UefChunk);const auto id=qFromLittleEndian<std::uint16_t>(bytes.data()+offset);const auto length=qFromLittleEndian<std::uint32_t>(bytes.data()+offset+LengthOffset);const auto payloadOffset=offset+ChunkHeaderSize;if(length>bytes.size()-payloadOffset)throw MediaError(DiagnosticCode::UefChunk);const auto* payload=bytes.data()+payloadOffset;
        SequentialSegmentFields value;value.position=segments.size();value.kind=kind(id);value.direction=SequentialTravelDirection::Forward;value.metadata={{std::string(ChunkIdMetadataKey),hex(id)},{std::string(KnownKey),std::string(understood(id)?TrueValue:FalseValue)},{std::string(BaseFrequencyMetadataKey),number(baseFrequency)},{std::string(BaudRateMetadataKey),std::to_string(baudRate)},{std::string(PhaseMetadataKey),std::to_string(phase)}};
        if(id==BaseFrequency&&length==FloatBytes){const auto frequency=std::bit_cast<float>(qFromLittleEndian<std::uint32_t>(payload));if(std::isfinite(frequency)&&frequency>0)baseFrequency=frequency;value.metadata[std::string(BaseFrequencyMetadataKey)]=number(baseFrequency);}
        else if(id==BaudRate&&(length==WordBytes||length==FloatBytes)){const double rate=length==WordBytes?qFromLittleEndian<std::uint16_t>(payload):std::bit_cast<float>(qFromLittleEndian<std::uint32_t>(payload));if(std::isfinite(rate)&&rate>0){const auto rounded=TapePcm::roundEven(rate);if(rounded>std::numeric_limits<int>::max())throw MediaError(DiagnosticCode::UefValue);baudRate=static_cast<int>(rounded);}value.metadata[std::string(BaudRateMetadataKey)]=std::to_string(baudRate);}
        else if(id==PhaseChange&&length==WordBytes){phase=qFromLittleEndian<std::uint16_t>(payload);value.metadata[std::string(PhaseMetadataKey)]=std::to_string(phase);}
        else if(id==FloatingGap&&length==FloatBytes){const auto seconds=std::bit_cast<float>(qFromLittleEndian<std::uint32_t>(payload));if(std::isfinite(seconds)&&seconds>0){const auto ticks=double(seconds)*TicksPerSecond;if(ticks>=double(std::numeric_limits<std::int64_t>::max()))throw MediaError(DiagnosticCode::UefValue);value.duration=SequentialTime(static_cast<std::int64_t>(ticks));}}
        else if(id==TapeSide&&length)value.faceNumber=payload[0];
        if(length){value.length=length;value.dataRange=DataRange(payloadOffset,length,DataRangeKind::Stored,data,payloadOffset);}segments.emplace_back(std::move(value));if(!understood(id))messages.push_back({std::string(DiagnosticKeys::UefUnknown),{hex(id)},{}});offset=payloadOffset+length;
    }
    std::map<std::string,std::string> metadata{{std::string(GzipMetadataKey),std::string(gzip?TrueValue:FalseValue)},{std::string(MajorVersionMetadataKey),std::to_string(bytes[MajorOffset])},{std::string(MinorVersionMetadataKey),std::to_string(bytes[MinorOffset])}};
    return std::make_shared<const MediaDocument>(context.source(),std::string(FormatId),MediaKind::Tape,std::make_shared<SequentialImage>(bytes.size(),std::nullopt,std::move(segments)),std::vector<MediaFileSystems::VolumeDescriptor>{},std::move(messages),std::move(metadata));
});}
}
