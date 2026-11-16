#include "Reader.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaEngine/DiagnosticKeys.h>
#include <algorithm>
#include <array>
namespace RetroMediaLab::MediaEngine::MsxCas {
namespace {
using namespace Constants;
std::vector<std::int64_t> separators(const RecognitionContext& context,std::stop_token token,bool firstOnly){
    std::vector<std::int64_t> result;std::size_t matched{};const auto source=context.dataSource();std::vector<std::uint8_t> bytes(BufferSize);
    for(std::int64_t offset=0;offset<context.length();){
        checkCancellation(token);const auto count=static_cast<std::int32_t>(std::min<std::int64_t>(BufferSize,context.length()-offset));
        bytes.resize(count);source->readExactly(offset,bytes,token);
        for(std::size_t index=0;index<bytes.size();++index){
            if(bytes[index]==Separator[matched]){
                if(++matched!=Separator.size())continue;
                result.push_back(offset+static_cast<std::int64_t>(index)-SeparatorLength+1);matched=0;if(firstOnly)return result;
            }else matched=bytes[index]==Separator[0]?1:0;
        }
        offset+=count;
    }
    return result;
}
std::optional<std::string> marker(const RecognitionContext& context,std::int64_t offset,std::int64_t available,std::stop_token token){
    if(available<FileTypeMarkerLength)return {};
    std::array<std::uint8_t,FileTypeMarkerLength> bytes{};context.dataSource()->readExactly(offset,bytes,token);
    const auto repeated=[&](std::uint8_t value){return std::all_of(bytes.begin(),bytes.end(),[&](auto byte){return byte==value;});};
    if(repeated(BinaryFileMarker))return std::string(BinaryType);
    if(repeated(TokenizedBasicFileMarker))return std::string(BasicType);
    if(repeated(AsciiFileMarker))return std::string(AsciiType);
    return {};
}
}
const ReaderCapabilities& Reader::capabilities()const{static const ReaderCapabilities caps{{std::string(FormatId)},{std::string(Extension)},{{Separator.begin(),Separator.end()}},{},{MediaKind::Tape},{RepresentationKind::Sequential}};return caps;}
bool Reader::supportsFormatId(std::string_view id)const{return sameIdentifier(id,FormatId);}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{
    if(context.requestedFormatId()&&!Reader{}.supportsFormatId(*context.requestedFormatId()))return false;
    const auto found=separators(context,token,true);if(found.empty())return false;if(context.requestedFormatId())return true;
    return found.front()==0&&marker(context,SeparatorLength,context.length()-SeparatorLength,token).has_value();
});}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{
    const auto found=separators(context,token,false);if(found.empty())throw MediaError(DiagnosticCode::MsxCasSeparator);
    const auto source=context.dataSource();std::vector<SequentialSegment> segments;std::vector<MediaFileSystems::MessageRecipe> messages;
    if(found.front()>0){SequentialSegmentFields value;value.position=segments.size();value.kind=SequentialSegmentKind::Unknown;value.length=found.front();value.dataRange=DataRange(0,found.front(),DataRangeKind::Stored,source);segments.emplace_back(std::move(value));messages.push_back({std::string(DiagnosticKeys::MsxCasPrefix),{},{}});}
    for(std::size_t index=0;index<found.size();++index){
        checkCancellation(token);const auto offset=found[index];SequentialSegmentFields separator;separator.position=segments.size();separator.kind=SequentialSegmentKind::TapeMark;separator.length=SeparatorLength;
        separator.dataRange=DataRange(offset,SeparatorLength,DataRangeKind::Stored,source,offset);separator.metadata={{std::string(SeparatorMetadataKey),std::string(TrueValue)}};segments.emplace_back(std::move(separator));
        const auto dataOffset=offset+SeparatorLength;const auto end=index+1<found.size()?found[index+1]:context.length();const auto length=end-dataOffset;
        if(length<=0){messages.push_back({std::string(DiagnosticKeys::MsxCasEmptySeparator),{std::to_string(offset)},{}});continue;}
        SequentialSegmentFields data;data.position=segments.size();data.kind=SequentialSegmentKind::DataBlock;data.length=length;data.direction=SequentialTravelDirection::Forward;
        data.dataRange=DataRange(dataOffset,length,DataRangeKind::Stored,source,dataOffset);data.metadata={{std::string(TimingMetadataKey),std::string(NoTiming)}};
        if(const auto type=marker(context,dataOffset,length,token))data.metadata[std::string(FileTypeMetadataKey)]=*type;
        segments.emplace_back(std::move(data));
    }
    return std::make_shared<const MediaDocument>(context.source(),std::string(FormatId),MediaKind::Tape,std::make_shared<SequentialImage>(context.length(),std::nullopt,std::move(segments)),std::vector<MediaFileSystems::VolumeDescriptor>{},std::move(messages),std::map<std::string,std::string>{{std::string(TimingMetadataKey),std::string(NoTiming)}});
});}
}
