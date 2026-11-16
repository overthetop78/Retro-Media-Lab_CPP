#include "Reader.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaEngine/DiagnosticKeys.h>
#include <QString>
#include <QtEndian>
#include <array>
namespace RetroMediaLab::MediaEngine::SpectrumTap {
namespace {
using namespace Constants;
struct Parsed {std::vector<SequentialSegment> segments;std::vector<MediaFileSystems::MessageRecipe> messages;bool valid=true;};
Parsed parse(const RecognitionContext& context,bool retain,std::stop_token token){
    Parsed result;std::int64_t offset=0,index=0;std::array<std::uint8_t,LengthFieldSize> prefix{};std::vector<std::uint8_t> bytes;
    while(offset<context.length()){
        checkCancellation(token);if(context.length()-offset<LengthFieldSize)throw MediaError(DiagnosticCode::SpectrumTapStructure);
        context.dataSource()->readExactly(offset,prefix,token);offset+=LengthFieldSize;const auto length=qFromLittleEndian<std::uint16_t>(prefix.data());
        if(length<MinimumBlockLength||length>context.length()-offset)throw MediaError(DiagnosticCode::SpectrumTapStructure);
        bytes.resize(length);context.dataSource()->readExactly(offset,bytes,token);std::uint8_t checksum=0;for(const auto byte:bytes)checksum^=byte;
        if(checksum){result.valid=false;if(!retain)return result;result.messages.push_back({std::string(DiagnosticKeys::SpectrumTapChecksum),{std::to_string(index)},{}});}
        if(retain){
            SequentialSegmentFields value;value.position=index;value.kind=SequentialSegmentKind::DataBlock;value.length=length;value.direction=SequentialTravelDirection::Forward;
            value.dataRange=DataRange(offset,length,DataRangeKind::Stored,context.dataSource(),offset);
            value.metadata={{std::string(BlockMetadataKey),std::string(TrueValue)},{std::string(FlagKey),QString::number(bytes.front(),HexRadix).rightJustified(HexWidth,QLatin1Char(DigitZero)).toStdString()},
                {std::string(ChecksumKey),std::string(checksum?FalseValue:TrueValue)},{std::string(HeaderKey),std::string(length==HeaderBlockLength&&bytes.front()==HeaderFlag?TrueValue:FalseValue)}};
            result.segments.emplace_back(std::move(value));
        }
        offset+=length;++index;
    }
    if(!index)throw MediaError(DiagnosticCode::SpectrumTapStructure);return result;
}
}
const ReaderCapabilities& Reader::capabilities()const{static const ReaderCapabilities caps{{std::string(FormatId)},{std::string(Extension)},{},{},{MediaKind::Tape},{RepresentationKind::Sequential}};return caps;}
bool Reader::supportsFormatId(std::string_view id)const{return sameIdentifier(id,FormatId);}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{
    if((context.requestedFormatId()&&!Reader{}.supportsFormatId(*context.requestedFormatId()))||!sameIdentifier(context.extension(),Extension))return false;
    try{return parse(context,false,token).valid;}catch(const MediaError& error){if(error.diagnostic().code==DiagnosticCode::SpectrumTapStructure)return false;throw;}
});}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{
    auto parsed=parse(context,true,token);return std::make_shared<const MediaDocument>(context.source(),std::string(FormatId),MediaKind::Tape,
        std::make_shared<SequentialImage>(context.length(),std::nullopt,std::move(parsed.segments)),std::vector<MediaFileSystems::VolumeDescriptor>{},std::move(parsed.messages),std::map<std::string,std::string>{{std::string(TimingKey),std::string(NoTiming)}});
});}
}
