#include "SignalBlocks.h"
#include "SignalIoConstants.h"
#include "../../../Composition/SequentialMedia.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QString>
#include <algorithm>
#include <cmath>
#include <limits>

namespace RetroMediaLab::MediaEngine::TapeSignal {
namespace {
using namespace IoConstants;
int number(const std::string& text){bool valid=false;const auto value=QString::fromUtf8(text).toInt(&valid);if(!valid||value<0)throw MediaError(DiagnosticCode::TapePcmData);return value;}
int sampleOffset(const SequentialTime& time,int rate){const auto value=TapePcm::roundEven(double(time.count())/TicksPerSecond*rate);if(!std::isfinite(value)||value<0||value>std::numeric_limits<int>::max())throw MediaError(DiagnosticCode::TapePcmData);return static_cast<int>(value);}
SequentialTime timeAt(int sample,int rate){return SequentialTime(static_cast<std::int64_t>(double(sample)/rate*TicksPerSecond));}
std::size_t validCount(std::span<const SequentialDecodedBlock> blocks){return static_cast<std::size_t>(std::count_if(blocks.begin(),blocks.end(),[](const auto& block){return block.integrityValid()==true;}));}
}
std::shared_ptr<const MediaDocument> indexBlocks(std::shared_ptr<const MediaDocument> document,std::stop_token token){
    checkCancellation(token);if(!document)throw MediaError(DiagnosticCode::TapePcmData);
    const auto& format=document->formatId();const auto* image=dynamic_cast<const SequentialImage*>(&document->representation());int channels{},rate{},bits{},align{};
    if((format!=TapeFormatIds::Wav&&format!=TapeFormatIds::Csw&&format!=TapeFormatIds::Raw&&format!=TapeFormatIds::Uef)||!image||!image->segments()||image->segments()->empty()||!TapePcm::tryProfile(document->metadata(),channels,rate,bits,align))return document;
    const auto composition=SequentialMediaComposition::createDefault();std::vector<SequentialDecodedBlock> best;std::size_t maximum=0;
    for(const auto& decoder:composition.decoders().findCompatible(*document,{},token)){
        checkCancellation(token);const auto result=decoder->decodeAsync(*document,{},token).get();const auto count=validCount(result.blocks());
        if(count>maximum){maximum=count;best.assign(result.blocks().begin(),result.blocks().end());}
    }
    std::vector<std::pair<int,const SequentialDecodedBlock*>> indexed;
    for(const auto& block:best)if(block.integrityValid()==true){const auto found=block.metadata().find(std::string(SamplePositionKey));if(found!=block.metadata().end())indexed.emplace_back(number(found->second),&block);}
    if(indexed.empty())return document;
    std::stable_sort(indexed.begin(),indexed.end(),[](const auto& a,const auto& b){return a.first<b.first;});
    std::vector<SequentialSegment> selected;
    for(const auto& segment:*image->segments()){
        checkCancellation(token);const auto& fields=segment.fields();
        if(fields.kind!=SequentialSegmentKind::Samples||!fields.dataRange||!fields.dataRange->source()){selected.push_back(segment);continue;}
        const auto& range=*fields.dataRange;const auto start=sampleOffset(fields.start.value_or(SequentialTime{}),rate);
        if(range.length()%align||range.length()/align>std::numeric_limits<int>::max()-start)throw MediaError(DiagnosticCode::TapePcmData);
        const auto end=start+static_cast<int>(range.length()/align);std::vector<int> points{start};
        for(const auto& [position,block]:indexed){(void)block;if(position>start&&position<end)points.push_back(position);}points.push_back(end);
        std::sort(points.begin(),points.end());points.erase(std::unique(points.begin(),points.end()),points.end());
        for(std::size_t index=1;index<points.size();++index){
            checkCancellation(token);const auto begin=points[index-1],length=points[index]-begin;auto value=fields;
            value.position=begin;value.length=length;value.start=timeAt(begin,rate);value.duration=timeAt(length,rate);
            value.dataRange=DataRange(0,std::int64_t(length)*align,DataRangeKind::Stored,range.source(),range.sourceOffset()+std::int64_t(begin-start)*align);
            const auto block=std::find_if(indexed.begin(),indexed.end(),[&](const auto& entry){return entry.first==begin;});
            if(block!=indexed.end())for(const auto& [key,entry]:block->second->metadata())value.metadata[key]=entry;
            selected.emplace_back(std::move(value));
        }
    }
    return std::make_shared<MediaDocument>(document->mediaSource(),document->formatId(),document->mediaKind(),std::make_shared<SequentialImage>(image->logicalLength(),image->duration(),std::move(selected)),std::vector<MediaFileSystems::VolumeDescriptor>(document->volumes().begin(),document->volumes().end()),std::vector<MediaFileSystems::MessageRecipe>(document->messages().begin(),document->messages().end()),document->metadata(),std::vector<std::string>(document->diagnostics().begin(),document->diagnostics().end()));
}
}
