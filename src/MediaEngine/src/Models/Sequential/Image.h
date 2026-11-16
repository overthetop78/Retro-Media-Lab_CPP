#pragma once
#include <RetroMediaLab/MediaEngine/SequentialTypes.h>
#include "../Blocks/BlockImage.h"
#include <map>
namespace RetroMediaLab::MediaEngine {
struct SequentialSegmentFields {
    std::int64_t position=0;SequentialSegmentKind kind=SequentialSegmentKind::Unknown;
    std::optional<std::int64_t> length;
    std::optional<SequentialTime> start,duration;
    std::optional<int> faceNumber,trackNumber,channelNumber;
    SequentialTravelDirection direction=SequentialTravelDirection::Unknown;
    std::optional<DataRange> dataRange;
    std::map<std::string,std::string> metadata;
};
class SequentialSegment final {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit SequentialSegment(SequentialSegmentFields);
    const SequentialSegmentFields& fields()const noexcept{return fields_;}
    RETROMEDIALAB_MEDIAENGINE_API SequentialSegment atPosition(std::int64_t)const;
private:SequentialSegmentFields fields_;
};
class SequentialImage final:public MediaFileSystems::ImageRepresentation,public RepresentationInfo {
public:
    RETROMEDIALAB_MEDIAENGINE_API SequentialImage(std::optional<std::int64_t> length,std::optional<SequentialTime> duration={},std::optional<std::vector<SequentialSegment>> segments={});
    RepresentationKind representationKind()const noexcept override{return RepresentationKind::Sequential;}
    std::optional<std::int64_t> logicalLength()const override{return length_;}
    bool supportsRandomAccess()const override{return false;}bool supportsSequentialAccess()const override{return true;}
    const std::optional<SequentialTime>& duration()const noexcept{return duration_;}
    const std::optional<std::vector<SequentialSegment>>& segments()const noexcept{return segments_;}
    const std::optional<std::vector<int>>& faces()const noexcept{return faces_;}
    const std::optional<std::vector<int>>& tracks()const noexcept{return tracks_;}
    const std::optional<std::vector<int>>& channels()const noexcept{return channels_;}
private:std::optional<std::int64_t> length_;std::optional<SequentialTime> duration_;std::optional<std::vector<SequentialSegment>> segments_;std::optional<std::vector<int>> faces_,tracks_,channels_;
};
}
