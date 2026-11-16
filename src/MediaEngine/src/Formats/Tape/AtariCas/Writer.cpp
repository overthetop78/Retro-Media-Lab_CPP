#include "Reader.h"
#include "Constants.h"
#include "../../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <algorithm>
#include <array>
#include <charconv>
namespace RetroMediaLab::MediaEngine::AtariCas {
namespace {
using namespace AtariTapeConstants;
struct Chunk {std::string id;std::uint16_t auxiliary;std::optional<DataRange> range;};
bool select(const MediaDocument& document,std::vector<Chunk>& chunks) {
    const auto* image=dynamic_cast<const SequentialImage*>(&document.representation());if(!image||!image->segments())return false;
    std::vector<const SequentialSegmentFields*> sorted;for(const auto& segment:*image->segments())sorted.push_back(&segment.fields());std::stable_sort(sorted.begin(),sorted.end(),[](const auto* a,const auto* b){return a->position<b->position;});
    for(const auto* value:sorted){const auto found=value->metadata.find(std::string(ChunkKey));if(found==value->metadata.end())continue;const auto& id=found->second;if(id.size()!=IdentifierSize||std::any_of(id.begin(),id.end(),[](unsigned char v){return v>MaximumAscii;}))return false;unsigned aux=0;
        if(const auto a=value->metadata.find(std::string(AuxKey));a!=value->metadata.end()){const auto& text=a->second;if(text.empty()||std::any_of(text.begin(),text.end(),[](char v){return v<FirstDigit||v>LastDigit;}))return false;const auto parsed=std::from_chars(text.data(),text.data()+text.size(),aux);if(parsed.ec!=std::errc{}||parsed.ptr!=text.data()+text.size()||aux>MaximumChunkSize)return false;}
        if(value->dataRange&&(value->dataRange->length()>MaximumChunkSize||value->dataRange->kind()!=DataRangeKind::Stored||!value->dataRange->source()))return false;const auto length=value->dataRange?value->dataRange->length():0;if((id==DataChunk&&!length)||(id==FskChunk&&(!length||length%WordBytes)))return false;
        chunks.push_back({id,std::uint16_t(aux),value->dataRange});
    }
    return !chunks.empty()&&chunks.front().id==FileMarker;
}
}
const WriterCapabilities& Writer::capabilities()const {static const WriterCapabilities caps{std::string(WriterId),{std::string(FormatId)},{std::string(Extension)},{RepresentationKind::Sequential},false};return caps;}
bool Writer::canWrite(const MediaDocument& document,std::string_view format,std::string_view extension)const {std::vector<Chunk> chunks;return sameIdentifier(format,FormatId)&&sameIdentifier(extension,Extension)&&document.mediaKind()==MediaKind::Tape&&select(document,chunks);}
std::future<std::vector<std::filesystem::path>> Writer::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path path,std::string format,std::stop_token token)const {
    return std::async(std::launch::deferred,[document=std::move(document),path=std::move(path),format=std::move(format),token]{checkCancellation(token);if(!document)throw MediaError(DiagnosticCode::AtariCasWrite);const auto ext=path.extension().u8string();std::vector<Chunk> chunks;if(!sameIdentifier(format,FormatId)||!sameIdentifier(std::string(ext.begin(),ext.end()),Extension)||document->mediaKind()!=MediaKind::Tape||!select(*document,chunks))throw MediaError(DiagnosticCode::AtariCasWrite);
        writeAtomicImage(path,[&](const auto& output,auto stop){for(const auto& chunk:chunks){checkCancellation(stop);std::array<std::uint8_t,HeaderSize> header{};std::copy(chunk.id.begin(),chunk.id.end(),header.begin());qToLittleEndian(std::uint16_t(chunk.range?chunk.range->length():0),header.data()+LengthOffset);qToLittleEndian(chunk.auxiliary,header.data()+AuxOffset);output(header);if(chunk.range)copyStoredRange(*chunk.range,output,stop);}},token);return std::vector<std::filesystem::path>{path};});
}
}
