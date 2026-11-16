#include "Reader.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaEngine/DiagnosticKeys.h>
#include <QtEndian>
#include <QString>
#include <algorithm>
#include <set>
#include <limits>
#include <type_traits>
namespace RetroMediaLab::MediaEngine::AtariCas {
namespace {
using namespace AtariTapeConstants;
std::string ascii(std::span<const std::uint8_t> bytes){std::string text;for(auto value:bytes)text.push_back(value<=MaximumAscii?char(value):AsciiReplacement);return text;}
template<class Range>std::string join(const Range& values,std::string_view separator){std::string result;bool first=true;for(const auto& item:values){if(!first)result+=separator;first=false;if constexpr(std::is_integral_v<std::decay_t<decltype(item)>>)result+=std::to_string(item);else result+=item;}return result;}
SequentialTime milliseconds(double value){return SequentialTime(std::int64_t(value*TicksPerMillisecond));}
void advance(SequentialTime& elapsed,SequentialTime duration){if(duration.count()<0||duration.count()>std::numeric_limits<std::int64_t>::max()-elapsed.count())throw MediaError(DiagnosticCode::AtariCasData);elapsed+=duration;}
}
const ReaderCapabilities& Reader::capabilities()const {static const ReaderCapabilities caps{{std::string(FormatId)},{std::string(Extension)},{{FileMarker.begin(),FileMarker.end()}},{},{MediaKind::Tape},{RepresentationKind::Sequential}};return caps;}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token]{checkCancellation(token);const auto requested=context.requestedFormatId();if((requested&&!sameIdentifier(*requested,FormatId))||context.length()<HeaderSize)return false;const auto bytes=context.readHeaderAsync(IdentifierSize,token).get();return std::equal(FileMarker.begin(),FileMarker.end(),bytes.begin());});
}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token] {
        checkCancellation(token);if(context.length()<HeaderSize)throw MediaError(DiagnosticCode::AtariCasData);
        std::vector<SequentialSegment> segments;std::vector<MediaFileSystems::MessageRecipe> messages;std::vector<std::string> names,types;std::set<int> baudRates;
        int baud=DefaultBaud;std::int64_t offset=0,position=0,chunks=0,dataChunks=0,fskChunks=0;SequentialTime elapsed{};const auto source=context.dataSource();
        while(offset<context.length()) {
            checkCancellation(token);if(context.length()-offset<HeaderSize)throw MediaError(DiagnosticCode::AtariCasData);
            const auto header=context.readAsync(offset,HeaderSize,token).get();const auto id=ascii(std::span(header).first(IdentifierSize));const auto length=qFromLittleEndian<std::uint16_t>(header.data()+LengthOffset),aux=qFromLittleEndian<std::uint16_t>(header.data()+AuxOffset);offset+=HeaderSize;
            if(length>context.length()-offset||(!chunks&&id!=FileMarker))throw MediaError(DiagnosticCode::AtariCasData);++chunks;if(std::find(types.begin(),types.end(),id)==types.end())types.push_back(id);
            SequentialSegmentFields value;value.position=position++;value.metadata={{std::string(ChunkKey),id},{std::string(AuxKey),std::to_string(aux)}};
            if(length)value.dataRange=DataRange(offset,length,DataRangeKind::Stored,source,offset);
            if(id==FileMarker) {
                const auto payload=context.readAsync(offset,length,token).get();auto name=ascii(payload);while(!name.empty()&&(name.back()==Null||name.back()==Blank))name.pop_back();value.metadata[std::string(NameKey)]=name;
                if(!QString::fromStdString(name).trimmed().isEmpty()&&std::find(names.begin(),names.end(),name)==names.end())names.push_back(name);
                value.kind=SequentialSegmentKind::TapeMark;if(length)value.length=length;segments.emplace_back(std::move(value));
            } else if(id==BaudChunk) {
                if(length)messages.push_back({std::string(DiagnosticKeys::AtariCasBaudPayload),{}, {}});baud=aux;baudRates.insert(baud);value.metadata[std::string(BaudKey)]=std::to_string(baud);value.kind=SequentialSegmentKind::Carrier;segments.emplace_back(std::move(value));
            } else if(id==DataChunk) {
                if(!length)throw MediaError(DiagnosticCode::AtariCasData);++dataChunks;baudRates.insert(baud);
                if(aux){SequentialSegmentFields carrier;carrier.position=value.position;carrier.kind=SequentialSegmentKind::Carrier;carrier.start=elapsed;carrier.duration=milliseconds(aux);carrier.direction=SequentialTravelDirection::Forward;carrier.metadata={{std::string(SignalKey),std::string(Mark)}};segments.emplace_back(std::move(carrier));advance(elapsed,milliseconds(aux));value.position=position++;}
                value.kind=SequentialSegmentKind::DataBlock;value.length=length;value.start=elapsed;value.direction=SequentialTravelDirection::Forward;value.metadata[std::string(BaudKey)]=std::to_string(baud);if(baud>0)value.duration=SequentialTime(std::int64_t((double(length)*FrameBits/baud)*SequentialTicksPerSecond));if(value.duration)advance(elapsed,*value.duration);segments.emplace_back(std::move(value));
            } else if(id==FskChunk) {
                if(!length||length%WordBytes)throw MediaError(DiagnosticCode::AtariCasData);++fskChunks;if(aux)advance(elapsed,milliseconds(aux));const auto payload=context.readAsync(offset,length,token).get();--position;
                for(std::size_t i=0;i<payload.size();i+=WordBytes){const auto units=qFromLittleEndian<std::uint16_t>(payload.data()+i);if(!units)throw MediaError(DiagnosticCode::AtariCasData);SequentialSegmentFields pulse;pulse.position=position++;pulse.kind=SequentialSegmentKind::Pulse;pulse.length=units;pulse.start=elapsed;pulse.duration=milliseconds(units*FskUnitMilliseconds);pulse.direction=SequentialTravelDirection::Forward;pulse.metadata={{std::string(SignalKey),std::string((i/WordBytes&1)?Mark:Space)},{std::string(ChunkOffsetKey),std::to_string(i)}};advance(elapsed,*pulse.duration);segments.emplace_back(std::move(pulse));}
                value.position=position++;value.kind=SequentialSegmentKind::DataBlock;value.length=length;segments.emplace_back(std::move(value));
            } else {value.kind=SequentialSegmentKind::Unknown;if(length)value.length=length;segments.emplace_back(std::move(value));messages.push_back({std::string(DiagnosticKeys::AtariCasUnknown),{id},{}});}
            offset+=length;
        }
        std::map<std::string,std::string> metadata{{std::string(SystemKey),std::string(MachineId)},{std::string(DefaultBaudKey),std::to_string(DefaultBaud)},{std::string(ChunkCountKey),std::to_string(chunks)},{std::string(ChunkTypesKey),join(types,ListSeparator)},{std::string(DataCountKey),std::to_string(dataChunks)},{std::string(FskCountKey),std::to_string(fskChunks)},{std::string(BaudRatesKey),join(baudRates,ListSeparator)}};if(!names.empty())metadata[std::string(InternalNameKey)]=join(names,NameSeparator);
        return std::make_shared<const MediaDocument>(context.source(),std::string(FormatId),MediaKind::Tape,std::make_shared<SequentialImage>(context.length(),elapsed.count()?std::optional(elapsed):std::nullopt,std::move(segments)),std::vector<MediaFileSystems::VolumeDescriptor>{},std::move(messages),std::move(metadata));
    });
}
}
