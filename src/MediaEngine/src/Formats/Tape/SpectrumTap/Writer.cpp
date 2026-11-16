#include "Reader.h"
#include "Constants.h"
#include "../../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QString>
#include <QtEndian>
#include <algorithm>
#include <array>
namespace RetroMediaLab::MediaEngine::SpectrumTap {
namespace {
using namespace Constants;
bool select(const MediaDocument& document,std::vector<DataRange>& ranges){
    const auto* image=dynamic_cast<const SequentialImage*>(&document.representation());if(!image||!image->segments()||image->segments()->empty())return false;
    std::vector<const SequentialSegmentFields*> sorted;for(const auto& segment:*image->segments())sorted.push_back(&segment.fields());
    std::stable_sort(sorted.begin(),sorted.end(),[](const auto* a,const auto* b){return a->position<b->position;});
    for(const auto* value:sorted){
        const auto valid=value->metadata.find(std::string(ChecksumKey));
        if(!value->metadata.contains(std::string(BlockMetadataKey))||valid==value->metadata.end()||!sameIdentifier(QString::fromStdString(valid->second).trimmed().toStdString(),TrueValue)
            ||!value->dataRange||value->dataRange->kind()!=DataRangeKind::Stored||!value->dataRange->source()||value->dataRange->length()<MinimumBlockLength||value->dataRange->length()>MaximumBlockLength)return false;
        ranges.push_back(*value->dataRange);
    }
    return true;
}
}
const WriterCapabilities& Writer::capabilities()const{static const WriterCapabilities caps{std::string(WriterId),{std::string(FormatId)},{std::string(Extension)},{RepresentationKind::Sequential},false};return caps;}
bool Writer::canWrite(const MediaDocument& document,std::string_view format,std::string_view extension)const{std::vector<DataRange> ranges;return sameIdentifier(format,FormatId)&&sameIdentifier(extension,Extension)&&document.mediaKind()==MediaKind::Tape&&select(document,ranges);}
std::future<std::vector<std::filesystem::path>> Writer::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path path,std::string format,std::stop_token token)const{return std::async(std::launch::deferred,[document=std::move(document),path=std::move(path),format=std::move(format),token]{
    checkCancellation(token);std::vector<DataRange> ranges;const auto ext=path.extension().u8string();
    if(!document||!sameIdentifier(format,FormatId)||!sameIdentifier(std::string(ext.begin(),ext.end()),Extension)||document->mediaKind()!=MediaKind::Tape||!select(*document,ranges))throw MediaError(DiagnosticCode::SpectrumTapWrite);
    writeAtomicImage(path,[&](const auto& output,auto stop){
        std::vector<std::uint8_t> bytes;std::array<std::uint8_t,LengthFieldSize> prefix{};
        for(const auto& range:ranges){
            checkCancellation(stop);bytes.resize(static_cast<std::size_t>(range.length()));range.source()->readExactly(range.sourceOffset(),bytes,stop);
            std::uint8_t checksum=0;for(const auto byte:bytes)checksum^=byte;if(checksum)throw MediaError(DiagnosticCode::SpectrumTapWrite);
            qToLittleEndian(static_cast<std::uint16_t>(bytes.size()),prefix.data());output(prefix);output(bytes);
        }
    },token);return std::vector<std::filesystem::path>{path};
});}
}
