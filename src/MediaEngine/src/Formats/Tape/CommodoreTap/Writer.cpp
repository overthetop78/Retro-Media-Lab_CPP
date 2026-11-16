#include "Reader.h"
#include "Profile.h"
#include "Constants.h"
#include "../../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::CommodoreTap {
namespace {
using namespace Constants;
struct Record {std::array<std::uint8_t,MaximumRecordSize> bytes{};int length=1;};
bool select(const MediaDocument& document,std::uint8_t version,std::vector<Record>& records,std::uint64_t& length,std::stop_token token={}){
    const auto* image=dynamic_cast<const SequentialImage*>(&document.representation());if(!image||!image->segments()||image->segments()->empty())return false;
    std::vector<const SequentialSegmentFields*> sorted;for(const auto& segment:*image->segments())sorted.push_back(&segment.fields());std::stable_sort(sorted.begin(),sorted.end(),[](const auto* a,const auto* b){return a->position<b->position;});
    for(const auto* value:sorted){
        checkCancellation(token);int cycles{};if(value->kind!=SequentialSegmentKind::Pulse||!integer(value->metadata,CycleCountMetadataKey,cycles)||cycles<=0)return false;Record record;
        if(cycles%ShortPulseCycleMultiplier==0&&cycles/ShortPulseCycleMultiplier<=MaximumByte)record.bytes[0]=static_cast<std::uint8_t>(cycles/ShortPulseCycleMultiplier);
        else{if(version==OriginalVersion||cycles>MaximumCycles)return false;record.length=MaximumRecordSize;record.bytes={0,static_cast<std::uint8_t>(cycles),static_cast<std::uint8_t>(cycles>>ByteBits),static_cast<std::uint8_t>(cycles>>WordShift)};}
        length+=record.length;records.push_back(record);
    }
    return true;
}
}
const WriterCapabilities& Writer::capabilities()const{static const WriterCapabilities caps{std::string(WriterId),{std::string(FormatId)},{std::string(Extension)},{RepresentationKind::Sequential},false};return caps;}
bool Writer::canWrite(const MediaDocument& document,std::string_view format,std::string_view extension)const{Profile selected;std::vector<Record> records;std::uint64_t length=0;return sameIdentifier(format,FormatId)&&sameIdentifier(extension,Extension)&&document.mediaKind()==MediaKind::Tape&&profile(document.metadata(),selected)&&select(document,selected.version,records,length);}
std::future<std::vector<std::filesystem::path>> Writer::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path path,std::string format,std::stop_token token)const{return std::async(std::launch::deferred,[document=std::move(document),path=std::move(path),format=std::move(format),token]{
    checkCancellation(token);Profile selected;std::vector<Record> records;std::uint64_t length=0;const auto ext=path.extension().u8string();
    if(!document||!sameIdentifier(format,FormatId)||!sameIdentifier(std::string(ext.begin(),ext.end()),Extension)||document->mediaKind()!=MediaKind::Tape||!profile(document->metadata(),selected)||!select(*document,selected.version,records,length,token))throw MediaError(DiagnosticCode::CommodoreTapWrite);
    if(length>std::numeric_limits<std::uint32_t>::max())throw MediaError(DiagnosticCode::CommodoreTapeLimit);
    writeAtomicImage(path,[&](const auto& output,auto stop){
        std::array<std::uint8_t,HeaderSize> header{};std::copy(selected.signature.begin(),selected.signature.end(),header.begin());header[VersionOffset]=selected.version;header[PlatformOffset]=selected.platform;header[VideoStandardOffset]=selected.video;qToLittleEndian(static_cast<std::uint32_t>(length),header.data()+DataLengthOffset);output(header);
        for(const auto& record:records){checkCancellation(stop);output(std::span(record.bytes).first(record.length));}
    },token);return std::vector<std::filesystem::path>{path};
});}
}
