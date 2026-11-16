#include "Reader.h"
#include "Constants.h"
#include "../../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QString>
#include <QtEndian>
#include <algorithm>
#include <array>
namespace RetroMediaLab::MediaEngine::SimhTape {
namespace {
using namespace Constants;
struct Object {std::uint32_t word;std::optional<DataRange> range;};
bool objects(const MediaDocument& document,std::vector<Object>& output) {
    const auto* image=dynamic_cast<const SequentialImage*>(&document.representation());if(!image||!image->segments()||image->segments()->empty())return false;
    std::vector<const SequentialSegmentFields*> sorted;for(const auto& segment:*image->segments())sorted.push_back(&segment.fields());
    std::stable_sort(sorted.begin(),sorted.end(),[](const auto* left,const auto* right){return left->position<right->position;});bool ended=false;
    for(const auto* segment:sorted) {
        if(ended)return false;const auto& value=*segment;const auto found=value.metadata.find(std::string(ObjectKindKey));if(found==value.metadata.end())return false;
        if(found->second==DataRecordKind) {
            if(!value.dataRange||value.dataRange->kind()!=DataRangeKind::Stored||!value.dataRange->source()||value.dataRange->length()<=0||value.dataRange->length()>RecordLengthMask)return false;
            const auto error=value.metadata.find(std::string(RecordErrorKey));const auto hasError=error!=value.metadata.end()&&sameIdentifier(QString::fromStdString(error->second).trimmed().toStdString(),True);
            output.push_back({std::uint32_t(value.dataRange->length())|(hasError?ErrorFlag:0),*value.dataRange});continue;
        }
        if(value.dataRange)return false;std::uint32_t marker=0;
        if(found->second==TapeMarkKind)marker=TapeMark;else if(found->second==EraseGapKind)marker=EraseGap;else if(found->second==EndOfMediumKind){marker=EndOfMedium;ended=true;}else return false;
        output.push_back({marker,{}});
    }
    return !output.empty();
}
}
const WriterCapabilities& Writer::capabilities()const {
    static const WriterCapabilities value{std::string(WriterId),{std::string(FormatId)},{std::string(Extension)},{RepresentationKind::Sequential},false};return value;
}
bool Writer::canWrite(const MediaDocument& document,std::string_view format,std::string_view extension)const {
    std::vector<Object> selected;return sameIdentifier(format,FormatId)&&sameIdentifier(extension,Extension)&&document.mediaKind()==MediaKind::Tape&&objects(document,selected);
}
std::future<std::vector<std::filesystem::path>> Writer::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path path,std::string format,std::stop_token token)const {
    return std::async(std::launch::deferred,[document=std::move(document),path=std::move(path),format=std::move(format),token] {
        checkCancellation(token);if(!document)throw MediaError(DiagnosticCode::SimhWrite);const auto extension=path.extension().u8string();std::vector<Object> selected;
        if(!sameIdentifier(format,FormatId)||!sameIdentifier(std::string(extension.begin(),extension.end()),Extension)||document->mediaKind()!=MediaKind::Tape||!objects(*document,selected))throw MediaError(DiagnosticCode::SimhWrite);
        writeAtomicImage(path,[&](const auto& output,auto stop) {
            for(const auto& item:selected) {
                checkCancellation(stop);std::array<std::uint8_t,WordSize> word{};qToLittleEndian(item.word,word.data());output(word);if(!item.range)continue;
                copyStoredRange(*item.range,output,stop);if(item.range->length()&1){const std::array<std::uint8_t,1> zero{};output(zero);}output(word);
            }
        },token);return std::vector<std::filesystem::path>{path};
    });
}
}
