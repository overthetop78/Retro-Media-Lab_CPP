#include "Reader.h"
#include "Constants.h"
#include "../../../Models/Blocks/Internal.h"
#include "../../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::MsxCas {
namespace {
using namespace Constants;
struct Part {bool separator=false;std::optional<DataRange> range;};
bool select(const MediaDocument& document,std::vector<Part>& parts){
    const auto* image=dynamic_cast<const SequentialImage*>(&document.representation());if(!image||!image->segments()||image->segments()->empty())return false;
    std::vector<const SequentialSegmentFields*> sorted;for(const auto& segment:*image->segments())sorted.push_back(&segment.fields());
    std::stable_sort(sorted.begin(),sorted.end(),[](const auto* a,const auto* b){return a->position<b->position;});bool separator=false;
    for(const auto* value:sorted){
        if(value->kind==SequentialSegmentKind::TapeMark&&value->metadata.contains(std::string(SeparatorMetadataKey))){parts.push_back({true,{}});separator=true;continue;}
        if(!value->dataRange||value->dataRange->kind()!=DataRangeKind::Stored||!value->dataRange->source())return false;
        parts.push_back({false,value->dataRange});
    }
    return separator;
}
}
const WriterCapabilities& Writer::capabilities()const{static const WriterCapabilities caps{std::string(WriterId),{std::string(FormatId)},{std::string(Extension)},{RepresentationKind::Sequential},false};return caps;}
bool Writer::canWrite(const MediaDocument& document,std::string_view format,std::string_view extension)const{std::vector<Part> parts;return sameIdentifier(format,FormatId)&&sameIdentifier(extension,Extension)&&document.mediaKind()==MediaKind::Tape&&select(document,parts);}
std::future<std::vector<std::filesystem::path>> Writer::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path path,std::string format,std::stop_token token)const{return std::async(std::launch::deferred,[document=std::move(document),path=std::move(path),format=std::move(format),token]{
    checkCancellation(token);const auto ext=path.extension().u8string();std::vector<Part> parts;
    if(!document||!sameIdentifier(format,FormatId)||!sameIdentifier(std::string(ext.begin(),ext.end()),Extension)||document->mediaKind()!=MediaKind::Tape||!select(*document,parts))throw MediaError(DiagnosticCode::MsxCasWrite);
    writeAtomicImage(path,[&](const auto& output,auto stop){for(const auto& part:parts){checkCancellation(stop);if(part.separator)output(Separator);else copyStoredRange(*part.range,output,stop);}},token);
    return std::vector<std::filesystem::path>{path};
});}
}
