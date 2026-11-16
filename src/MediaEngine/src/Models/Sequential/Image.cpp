#include "Image.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
SequentialSegment::SequentialSegment(SequentialSegmentFields value):fields_(std::move(value)){const auto& v=fields_;if(v.position<0||(v.length&&*v.length<=0)||(v.start&&v.start->count()<0)||(v.duration&&v.duration->count()<=0)||(v.faceNumber&&*v.faceNumber<0)||(v.trackNumber&&*v.trackNumber<0)||(v.channelNumber&&*v.channelNumber<0))throw MediaError(DiagnosticCode::SequentialModel);}
SequentialSegment SequentialSegment::atPosition(std::int64_t position)const{auto value=fields_;value.position=position;return SequentialSegment(std::move(value));}
SequentialImage::SequentialImage(std::optional<std::int64_t> length,std::optional<SequentialTime> duration,std::optional<std::vector<SequentialSegment>> segments):length_(length),duration_(duration),segments_(std::move(segments)){
    if((length_&&*length_<0)||(duration_&&duration_->count()<0))throw MediaError(DiagnosticCode::SequentialModel);if(!segments_)return;std::vector<int> faces,tracks,channels;
    for(const auto& segment:*segments_){const auto& v=segment.fields();if(duration_&&v.start&&v.duration&&(*v.start>*duration_||*v.duration>*duration_-*v.start))throw MediaError(DiagnosticCode::SequentialModel);if(v.faceNumber)faces.push_back(*v.faceNumber);if(v.trackNumber)tracks.push_back(*v.trackNumber);if(v.channelNumber)channels.push_back(*v.channelNumber);}
    const auto collect=[](std::vector<int> values)->std::optional<std::vector<int>>{if(values.empty())return {};std::sort(values.begin(),values.end());values.erase(std::unique(values.begin(),values.end()),values.end());return values;};faces_=collect(std::move(faces));tracks_=collect(std::move(tracks));channels_=collect(std::move(channels));
}
}
