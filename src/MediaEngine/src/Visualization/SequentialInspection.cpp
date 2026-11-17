#include <RetroMediaLab/MediaEngine/SequentialVisualization.h>
#include "Providers.h"
#include "Constants.h"
#include "../Models/Sequential/Image.h"
#include <algorithm>
#include <set>
namespace RetroMediaLab::MediaEngine {
SequentialVisualizationModel describeSequentialVisualization(std::shared_ptr<const MediaDocument> document,const VisualizationDescriptor& descriptor,std::stop_token token){
    if(!document)throw MediaError(DiagnosticCode::ReaderNullDocument);const auto& image=Visualization::representation<SequentialImage>(*document);SequentialVisualizationModel result;result.logicalLength=image.logicalLength();if(image.duration())result.durationTicks=image.duration()->count();result.metadata=document->metadata();
    double maximumTime=0;if(image.segments())for(const auto& segment:*image.segments()){checkCancellation(token);const auto& fields=segment.fields();maximumTime=std::max(maximumTime,static_cast<double>(fields.start?fields.start->count():0)+static_cast<double>(fields.duration?fields.duration->count():0));}
    result.timed=result.durationTicks?*result.durationTicks>0:maximumTime>0;result.totalLength=result.timed?std::max(maximumTime,static_cast<double>(result.durationTicks.value_or(0))):0;std::set<std::int32_t> lanes;double position=0;
    if(image.segments()){if(image.segments()->size()!=descriptor.fields().elements.size())throw MediaError(DiagnosticCode::VisualizationValue,{std::string(Visualization::Constants::Descriptor)});
        for(std::size_t index=0;index<image.segments()->size();++index){checkCancellation(token);const auto& fields=image.segments()->at(index).fields();SequentialVisualizationSegment value;value.position=fields.position;value.storedLength=fields.length;if(fields.start)value.startTicks=fields.start->count();if(fields.duration)value.durationTicks=fields.duration->count();value.face=fields.faceNumber;value.track=fields.trackNumber;value.channel=fields.channelNumber;value.metadata=fields.metadata;value.lane=fields.channelNumber.value_or(fields.trackNumber.value_or(fields.faceNumber.value_or(0)));lanes.insert(value.lane);
            switch(fields.kind){case SequentialSegmentKind::Samples:case SequentialSegmentKind::Pulse:case SequentialSegmentKind::Carrier:value.kind=SequentialVisualizationKind::Signal;break;case SequentialSegmentKind::Silence:value.kind=SequentialVisualizationKind::Silence;break;case SequentialSegmentKind::DataBlock:case SequentialSegmentKind::Record:value.kind=SequentialVisualizationKind::DecodedBlock;break;default:break;}
            value.start=result.timed?static_cast<double>(value.startTicks.value_or(0)):position;value.length=std::max(Visualization::Constants::MinimumSequentialLength,static_cast<double>(result.timed?value.durationTicks.value_or(0):value.storedLength.value_or(1)));position+=value.length;if(result.timed)result.totalLength=std::max(result.totalLength,value.start+value.length);result.segments.push_back(std::move(value));
        }
    }
    if(!result.timed)result.totalLength=position;result.lanes.assign(lanes.begin(),lanes.end());if(result.lanes.empty())result.lanes.push_back(0);return result;
}
std::future<std::vector<std::uint8_t>> readSequentialVisualizationPreviewAsync(std::shared_ptr<const MediaDocument> document,std::size_t index,std::stop_token token){return std::async(std::launch::async,[document=std::move(document),index,token]{if(!document)throw MediaError(DiagnosticCode::ReaderNullDocument);checkCancellation(token);const auto& image=Visualization::representation<SequentialImage>(*document);if(!image.segments() || index>=image.segments()->size())throw MediaError(DiagnosticCode::VisualizationValue,{std::string(Visualization::Constants::Position)});const auto& range=image.segments()->at(index).fields().dataRange;if(!range)return std::vector<std::uint8_t>{};return readStoredRange(&*range,0,std::min<std::int64_t>(range->length(),Visualization::Constants::PreviewBytes),token).value_or(std::vector<std::uint8_t>{});});}
}
