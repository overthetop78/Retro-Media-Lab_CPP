#include "Blocks.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QString>
#include <algorithm>
#include <array>
namespace RetroMediaLab::MediaEngine::Tzx {
namespace {
using namespace Constants;
std::array<std::uint8_t,HeaderSize> header(const RecognitionContext& context,std::stop_token token){
    checkCancellation(token);if(context.length()<HeaderSize)throw MediaError(DiagnosticCode::TzxHeader);
    std::array<std::uint8_t,HeaderSize> bytes{};context.dataSource()->readExactly(0,bytes,token);
    if(!std::equal(Signature.begin(),Signature.end(),bytes.begin())||bytes[MarkerOffset]!=EndOfTextMarker)throw MediaError(DiagnosticCode::TzxHeader);return bytes;
}
}
const ReaderCapabilities& Reader::capabilities()const{static const ReaderCapabilities caps=[] {std::vector<std::uint8_t> signature(Signature.begin(),Signature.end());signature.push_back(EndOfTextMarker);return ReaderCapabilities{{std::string(FormatId)},{std::string(TzxExtension),std::string(CdtExtension),std::string(TsxExtension)},{signature},{},{MediaKind::Tape},{RepresentationKind::Sequential}};}();return caps;}
bool Reader::supportsFormatId(std::string_view id)const{return sameIdentifier(id,FormatId);}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{
    if(context.requestedFormatId()&&!Reader{}.supportsFormatId(*context.requestedFormatId()))return false;
    try{header(context,token);return true;}catch(const MediaError& error){if(error.diagnostic().code==DiagnosticCode::TzxHeader)return false;throw;}
});}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{
    const auto bytes=header(context,token);if(bytes[MajorOffset]!=MajorVersion||bytes[MinorOffset]>MinorVersion)throw MediaError(DiagnosticCode::TzxVersion,{std::to_string(bytes[MajorOffset]),std::to_string(bytes[MinorOffset])});
    const auto blocks=readBlocks(context,token);std::vector<SequentialSegment> segments;segments.reserve(blocks.size());
    for(const auto& block:blocks){
        checkCancellation(token);SequentialSegmentFields value;value.position=static_cast<std::int64_t>(segments.size());value.kind=segmentKind(block.id);
        if(block.length){value.length=block.length;value.dataRange=DataRange(block.offset,block.length,DataRangeKind::Stored,context.dataSource(),block.offset);}
        value.metadata={{std::string(BlockIdMetadataKey),QString::number(block.id,HexRadix).rightJustified(HexWidth,QLatin1Char(DigitZero)).toStdString()},{std::string(PayloadKey),std::to_string(block.length)},
            {std::string(KnownKey),std::string(std::find(SupportedBlockIds.begin(),SupportedBlockIds.end(),block.id)!=SupportedBlockIds.end()?TrueValue:FalseValue)}};
        segments.emplace_back(std::move(value));
    }
    return std::make_shared<const MediaDocument>(context.source(),std::string(FormatId),MediaKind::Tape,std::make_shared<SequentialImage>(context.length(),std::nullopt,std::move(segments)),std::vector<MediaFileSystems::VolumeDescriptor>{},std::vector<MediaFileSystems::MessageRecipe>{},
        std::map<std::string,std::string>{{std::string(MajorKey),std::to_string(bytes[MajorOffset])},{std::string(MinorKey),std::to_string(bytes[MinorOffset])},{std::string(ContextKey),QString::fromStdString(context.extension()).toLower().toStdString()}});
});}
}
