#include "Reader.h"
#include "Constants.h"
#include "../../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QString>
#include <algorithm>
#include <array>
#include <charconv>
namespace RetroMediaLab::MediaEngine::Tzx {
namespace {
using namespace Constants;
struct Part {std::uint8_t id;std::optional<DataRange> range;};
bool extensionSupported(std::string_view extension){return sameIdentifier(extension,TzxExtension)||sameIdentifier(extension,CdtExtension)||sameIdentifier(extension,TsxExtension);}
bool parseByte(std::string_view text,int radix,std::uint8_t& value){
    unsigned number=0;const auto parsed=std::from_chars(text.data(),text.data()+text.size(),number,radix);
    if(parsed.ec!=std::errc{}||parsed.ptr!=text.data()+text.size()||number>MaximumByte)return false;value=static_cast<std::uint8_t>(number);return true;
}
std::uint8_t version(const MediaDocument& document,std::string_view key,std::uint8_t fallback){
    const auto found=document.metadata().find(std::string(key));std::uint8_t value{};
    return found!=document.metadata().end()&&parseByte(found->second,DecimalRadix,value)?value:fallback;
}
bool select(const MediaDocument& document,std::vector<Part>& parts){
    const auto* image=dynamic_cast<const SequentialImage*>(&document.representation());if(!image||!image->segments()||image->segments()->empty())return false;
    std::vector<const SequentialSegmentFields*> sorted;for(const auto& segment:*image->segments())sorted.push_back(&segment.fields());
    std::stable_sort(sorted.begin(),sorted.end(),[](const auto* a,const auto* b){return a->position<b->position;});
    for(const auto* value:sorted){
        const auto found=value->metadata.find(std::string(BlockIdMetadataKey));std::uint8_t id{};
        if(found==value->metadata.end()||!parseByte(QString::fromStdString(found->second).trimmed().toStdString(),HexRadix,id))return false;
        if(!value->length){if(value->dataRange)return false;parts.push_back({id,{}});continue;}
        if(!value->dataRange||value->dataRange->kind()!=DataRangeKind::Stored||!value->dataRange->source()||value->dataRange->length()!=*value->length)return false;
        parts.push_back({id,value->dataRange});
    }
    return true;
}
}
const WriterCapabilities& Writer::capabilities()const{static const WriterCapabilities caps{std::string(WriterId),{std::string(FormatId)},{std::string(TzxExtension),std::string(CdtExtension),std::string(TsxExtension)},{RepresentationKind::Sequential},false};return caps;}
bool Writer::canWrite(const MediaDocument& document,std::string_view format,std::string_view extension)const{std::vector<Part> parts;return sameIdentifier(format,FormatId)&&extensionSupported(extension)&&document.mediaKind()==MediaKind::Tape&&select(document,parts);}
std::future<std::vector<std::filesystem::path>> Writer::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path path,std::string format,std::stop_token token)const{return std::async(std::launch::deferred,[document=std::move(document),path=std::move(path),format=std::move(format),token]{
    checkCancellation(token);const auto ext=path.extension().u8string();std::vector<Part> parts;
    if(!document||!sameIdentifier(format,FormatId)||!extensionSupported(std::string(ext.begin(),ext.end()))||document->mediaKind()!=MediaKind::Tape||!select(*document,parts))throw MediaError(DiagnosticCode::TzxWrite);
    const auto major=version(*document,MajorKey,MajorVersion),minor=version(*document,MinorKey,MinorVersion);
    if(major!=MajorVersion||minor>MinorVersion)throw MediaError(DiagnosticCode::TzxVersion,{std::to_string(major),std::to_string(minor)});
    writeAtomicImage(path,[&](const auto& output,auto stop){
        std::array<std::uint8_t,HeaderSize> bytes{};std::copy(Signature.begin(),Signature.end(),bytes.begin());bytes[MarkerOffset]=EndOfTextMarker;bytes[MajorOffset]=major;bytes[MinorOffset]=minor;output(bytes);
        for(const auto& part:parts){checkCancellation(stop);const std::array<std::uint8_t,ByteWidth> id{part.id};output(id);if(part.range)copyStoredRange(*part.range,output,stop);}
    },token);return std::vector<std::filesystem::path>{path};
});}
}
