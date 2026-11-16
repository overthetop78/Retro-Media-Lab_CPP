#pragma once
#include "Constants.h"
#include "../../../Images/RecognitionContext.h"
namespace RetroMediaLab::MediaEngine::AlcoholMds {
struct Header {std::uint8_t major=0,minor=0;std::uint16_t mediumType=0,sessionCount=0;std::uint32_t sessionsOffset=0;};
struct TrackBlock {int sessionNumber=0,trackNumber=0;std::uint8_t mode=0,subchannelMode=0,adrControl=0;int sectorSize=0;std::int64_t startSector=0,startOffset=0,footerOffset=0;std::string dataPath;};
class SessionBlock final {
public:
    RETROMEDIALAB_MEDIAENGINE_API SessionBlock(int number,std::int64_t start,std::int64_t end,int firstTrack,int lastTrack,std::vector<TrackBlock>);
    int number()const noexcept{return number_;}std::int64_t startSector()const noexcept{return start_;}std::int64_t endSector()const noexcept{return end_;}
    int firstTrack()const noexcept{return first_;}int lastTrack()const noexcept{return last_;}std::span<const TrackBlock> tracks()const noexcept{return tracks_;}
private:int number_;std::int64_t start_,end_;int first_,last_;std::vector<TrackBlock> tracks_;
};
struct Descriptor {Header header;std::vector<SessionBlock> sessions;};
RETROMEDIALAB_MEDIAENGINE_API Header readHeader(const RecognitionContext&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API Descriptor readDescriptor(const RecognitionContext&,std::stop_token={});
}
