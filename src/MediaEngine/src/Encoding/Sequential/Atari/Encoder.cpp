#include "Encoder.h"
#include "../../../Formats/Tape/AtariCas/Constants.h"
#include "../../../Images/RecognitionContext.h"
#include "../../../Sources/RandomAccessSources.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <limits>
#include <charconv>
namespace RetroMediaLab::MediaEngine {
namespace {
using namespace AtariTapeConstants;
std::map<std::string,std::string> chunk(std::string_view id,int auxiliary,std::optional<int> baud={}){std::map<std::string,std::string> result{{std::string(ChunkKey),std::string(id)},{std::string(AuxKey),std::to_string(auxiliary)}};if(baud)result[std::string(BaudKey)]=std::to_string(*baud);return result;}
int duration(const std::map<std::string,std::string>& parameters,std::string_view key,int fallback) {
    const auto found=parameters.find(std::string(key));if(found==parameters.end())return fallback;const auto& text=found->second;int value=0;if(text.empty()||std::any_of(text.begin(),text.end(),[](char c){return c<FirstDigit||c>LastDigit;}))throw MediaError(DiagnosticCode::AtariMarkDuration);const auto parsed=std::from_chars(text.data(),text.data()+text.size(),value);if(parsed.ec!=std::errc{}||parsed.ptr!=text.data()+text.size()||value<0||value>MaximumChunkSize)throw MediaError(DiagnosticCode::AtariMarkDuration);return value;
}
void advance(SequentialTime& elapsed,SequentialTime duration){if(duration.count()<0||duration.count()>std::numeric_limits<std::int64_t>::max()-elapsed.count())throw MediaError(DiagnosticCode::AtariEncodeData);elapsed+=duration;}
bool retainedChunks(const SequentialEncodeRequest& request) {
    const auto retained=request.retainedSegments();const auto hasFsk=std::any_of(retained.begin(),retained.end(),[](const auto& segment){const auto id=segment.fields().metadata.find(std::string(ChunkKey));return id!=segment.fields().metadata.end()&&id->second==FskChunk;});
    for(const auto& segment:retained) {
        const auto& value=segment.fields();const auto id=value.metadata.find(std::string(ChunkKey));
        if(id==value.metadata.end()){if(hasFsk&&value.kind==SequentialSegmentKind::Pulse&&!value.dataRange&&value.metadata.contains(std::string(SignalKey))&&value.metadata.contains(std::string(ChunkOffsetKey)))continue;return false;}
        if(id->second.size()!=IdentifierSize||std::any_of(id->second.begin(),id->second.end(),[](unsigned char c){return c>MaximumAscii;}))return false;
        try{duration(value.metadata,AuxKey,0);}catch(const MediaError&){return false;}
        const auto length=value.dataRange?value.dataRange->length():0;if(length>MaximumChunkSize||(value.dataRange&&(value.dataRange->kind()!=DataRangeKind::Stored||!value.dataRange->source())))return false;
        if((id->second==DataChunk&&!length)||(id->second==FskChunk&&(!length||length%WordBytes)))return false;
    }
    return true;
}
}
std::string_view AtariTapeEncoder::id()const noexcept{return EncoderId;}
std::span<const std::string> AtariTapeEncoder::formatIds()const {static const std::vector<std::string> values{std::string(FormatId)};return values;}
std::span<const std::string> AtariTapeEncoder::machineIds()const {static const std::vector<std::string> values{std::string(MachineId)};return values;}
bool AtariTapeEncoder::canEncode(const SequentialEncodeRequest& request)const {
    return sameIdentifier(request.targetFormatId(),FormatId)&&sameIdentifier(request.machineId(),MachineId)&&!request.blocks().empty()&&retainedChunks(request)&&std::all_of(request.blocks().begin(),request.blocks().end(),[](const auto& block){return block.integrityValid()==true&&block.data().size()<=MaximumChunkSize&&(!block.data().empty()||AtariTapeEncoding::logicalRecords(block));});
}
std::future<std::shared_ptr<const SequentialImage>> AtariTapeEncoder::encodeAsync(SequentialEncodeRequest request,std::stop_token token)const {
    return std::async(std::launch::deferred,[request=std::move(request),token] {
        if(!retainedChunks(request))throw MediaError(DiagnosticCode::AtariCasWrite);if(!AtariTapeEncoder{}.canEncode(request))throw MediaError(DiagnosticCode::AtariEncodeData);checkCancellation(token);
        const auto initial=duration(request.parameters(),InitialMarkKey,InitialMarkMilliseconds),subsequent=duration(request.parameters(),SubsequentMarkKey,SubsequentMarkMilliseconds);
        std::vector<SequentialSegment> output,retained(request.retainedSegments().begin(),request.retainedSegments().end());std::stable_sort(retained.begin(),retained.end(),[](const auto& a,const auto& b){return a.fields().position<b.fields().position;});
        const auto marker=std::find_if(retained.begin(),retained.end(),[](const auto& segment){const auto id=segment.fields().metadata.find(std::string(ChunkKey));return id!=segment.fields().metadata.end()&&id->second==FileMarker;});
        if(marker!=retained.end()){output.push_back(marker->atPosition(0));retained.erase(marker);}else{SequentialSegmentFields value;value.kind=SequentialSegmentKind::TapeMark;value.metadata=chunk(FileMarker,0);output.emplace_back(std::move(value));}
        struct Item {std::int64_t position;const SequentialSegment* segment;const SequentialDecodedBlock* block;};std::vector<Item> pending;for(const auto& segment:retained)pending.push_back({segment.fields().position,&segment,nullptr});
        const auto blocks=request.blocks();if(blocks.size()>std::uint64_t(std::numeric_limits<std::int64_t>::max()))throw MediaError(DiagnosticCode::AtariEncodeData);
        for(std::size_t i=0;i<blocks.size();++i){const auto source=blocks[i].sourcePositions();const auto position=source.empty()?std::numeric_limits<std::int64_t>::max()-std::int64_t(blocks.size())+std::int64_t(i):*std::min_element(source.begin(),source.end());pending.push_back({position,nullptr,&blocks[i]});}
        std::stable_sort(pending.begin(),pending.end(),[](const auto& a,const auto& b){if(a.position!=b.position)return a.position<b.position;return a.segment&&!b.segment;});
        SequentialTime elapsed{};bool first=true;
        for(const auto& item:pending) {
            checkCancellation(token);if(item.segment){output.push_back(item.segment->atPosition(output.size()));continue;}
            const auto records=AtariTapeEncoding::physicalRecords(*item.block);const auto reconstructed=AtariTapeEncoding::logicalRecords(*item.block);
            for(const auto& bytes:records) {
                checkCancellation(token);SequentialSegmentFields carrier;carrier.position=output.size();carrier.kind=SequentialSegmentKind::Carrier;carrier.metadata=chunk(BaudChunk,DefaultBaud);output.emplace_back(std::move(carrier));const auto mark=first?initial:subsequent;first=false;
                advance(elapsed,SequentialTime(std::int64_t(mark*TicksPerMillisecond)));const auto time=SequentialTime(std::int64_t(bytes.size()*double(FrameBits)/DefaultBaud*SequentialTicksPerSecond));
                SequentialSegmentFields value;value.position=output.size();value.kind=SequentialSegmentKind::DataBlock;value.length=bytes.size();value.start=elapsed;value.duration=time;value.direction=SequentialTravelDirection::Forward;value.dataRange=DataRange(0,bytes.size(),DataRangeKind::Stored,memorySource(bytes));value.metadata=chunk(DataChunk,mark,DefaultBaud);value.metadata[std::string(ReconstructedSignalKey)]=True;if(reconstructed)value.metadata[std::string(ReconstructedRecordsKey)]=True;output.emplace_back(std::move(value));advance(elapsed,time);
            }
        }
        std::int64_t logicalLength=0;auto total=elapsed;
        for(const auto& segment:output){const auto& value=segment.fields();if(value.dataRange){const auto length=value.dataRange->length();if(length>std::numeric_limits<std::int64_t>::max()-logicalLength)throw MediaError(DiagnosticCode::AtariEncodeData);logicalLength+=length;}if(value.start&&value.duration){auto end=*value.start;advance(end,*value.duration);total=std::max(total,end);}}
        return std::make_shared<const SequentialImage>(logicalLength,total.count()?std::optional(total):std::nullopt,std::move(output));
    });
}
}
