#include "Reader.h"
#include "Constants.h"
#include "Compression.h"
#include "../../../Models/Blocks/Internal.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <QString>
#include <algorithm>
#include <array>
#include <charconv>
#include <limits>
namespace RetroMediaLab::MediaEngine::Uef {
namespace {
using namespace Constants;
struct Profile {bool gzip=false;unsigned major=SupportedMajorVersion,minor=SupportedMinorVersion;};
struct Chunk {std::uint16_t id;std::optional<DataRange> range;};
bool profile(const MediaDocument& document,Profile& result){const auto& values=document.metadata();if(const auto found=values.find(std::string(GzipMetadataKey));found!=values.end()){const auto text=QString::fromUtf8(found->second).trimmed();if(text.compare(QString::fromUtf8(TrueValue),Qt::CaseInsensitive)==0)result.gzip=true;else if(text.compare(QString::fromUtf8(FalseValue),Qt::CaseInsensitive)!=0)return false;}
    for(const auto& [key,target]:std::array<std::pair<std::string_view,unsigned*>,WordBytes>{{{MajorVersionMetadataKey,&result.major},{MinorVersionMetadataKey,&result.minor}}})if(const auto found=values.find(std::string(key));found!=values.end()){const auto& text=found->second;if(text.empty()||std::any_of(text.begin(),text.end(),[](char c){return c<DigitZero||c>DigitNine;}))return false;const auto parsed=std::from_chars(text.data(),text.data()+text.size(),*target);if(parsed.ec!=std::errc{}||parsed.ptr!=text.data()+text.size()||*target>MaximumByte)return false;}
    return result.major==SupportedMajorVersion&&result.minor<=SupportedMinorVersion;
}
bool select(const MediaDocument& document,std::vector<Chunk>& chunks){const auto* image=dynamic_cast<const SequentialImage*>(&document.representation());if(!image||!image->segments())return false;std::vector<const SequentialSegmentFields*> sorted;for(const auto& segment:*image->segments())sorted.push_back(&segment.fields());std::stable_sort(sorted.begin(),sorted.end(),[](const auto* a,const auto* b){return a->position<b->position;});
    for(const auto* value:sorted){const auto found=value->metadata.find(std::string(ChunkIdMetadataKey));if(found==value->metadata.end())return false;const auto text=QString::fromUtf8(found->second).trimmed().toStdString();unsigned id{};if(text.empty())return false;const auto parsed=std::from_chars(text.data(),text.data()+text.size(),id,HexRadix);if(parsed.ec!=std::errc{}||parsed.ptr!=text.data()+text.size()||id>std::numeric_limits<std::uint16_t>::max())return false;
        if(value->dataRange){const auto& range=*value->dataRange;if(range.kind()!=DataRangeKind::Stored||!range.source()||range.length()>std::numeric_limits<std::uint32_t>::max()||static_cast<std::uint64_t>(range.sourceOffset())>range.source()->length()||static_cast<std::uint64_t>(range.length())>range.source()->length()-static_cast<std::uint64_t>(range.sourceOffset()))return false;}
        chunks.push_back({static_cast<std::uint16_t>(id),value->dataRange});
    }return true;
}
void writeLogical(const Profile& profile,const std::vector<Chunk>& chunks,const ImageOutput& output,std::stop_token token){checkCancellation(token);std::array<std::uint8_t,HeaderSize> header{};std::copy(Signature.begin(),Signature.end(),header.begin());header[MinorOffset]=static_cast<std::uint8_t>(profile.minor);header[MajorOffset]=static_cast<std::uint8_t>(profile.major);output(header);
    for(const auto& chunk:chunks){checkCancellation(token);std::array<std::uint8_t,ChunkHeaderSize> bytes{};qToLittleEndian(chunk.id,bytes.data());qToLittleEndian(static_cast<std::uint32_t>(chunk.range?chunk.range->length():0),bytes.data()+LengthOffset);output(bytes);if(chunk.range)copyStoredRange(*chunk.range,output,token);}
}
}
const WriterCapabilities& Writer::capabilities()const{static const WriterCapabilities caps{std::string(WriterId),{std::string(FormatId)},{std::string(Extension)},{RepresentationKind::Sequential},false};return caps;}
bool Writer::canWrite(const MediaDocument& document,std::string_view format,std::string_view extension)const{Profile p;std::vector<Chunk> chunks;return sameIdentifier(format,FormatId)&&sameIdentifier(extension,Extension)&&document.mediaKind()==MediaKind::Tape&&profile(document,p)&&select(document,chunks);}
std::future<std::vector<std::filesystem::path>> Writer::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path path,std::string format,std::stop_token token)const{return std::async(std::launch::deferred,[document=std::move(document),path=std::move(path),format=std::move(format),token]{
    checkCancellation(token);Profile p;std::vector<Chunk> chunks;const auto ext=path.extension().u8string();if(!document||!sameIdentifier(format,FormatId)||!sameIdentifier(std::string(ext.begin(),ext.end()),Extension)||document->mediaKind()!=MediaKind::Tape||!profile(*document,p)||!select(*document,chunks))throw MediaError(DiagnosticCode::UefWrite);
    writeAtomicImage(path,[&](const auto& output,auto stop){if(p.gzip)writeGzip([&](const auto& compressed){writeLogical(p,chunks,compressed,stop);},output,stop);else writeLogical(p,chunks,output,stop);},token);return std::vector<std::filesystem::path>{path};
});}
}
