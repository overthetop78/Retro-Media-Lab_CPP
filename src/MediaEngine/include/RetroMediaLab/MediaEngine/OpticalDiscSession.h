#pragma once
#include "Export.h"
#include "OpticalTrackMode.h"
#include <cstdint>
#include <filesystem>
#include <memory>
#include <future>
#include <optional>
#include <span>
#include <stop_token>
#include <string>
#include <vector>
namespace RetroMediaLab::MediaEngine {
class MediaDocument;
struct OpticalDiscTrack {
    int trackNumber=0;OpticalTrackMode mode=OpticalTrackMode::Audio;
    std::int64_t firstSector=0,sectorCount=0;int storedSectorSize=0,userDataOffset=0,userDataLength=0;
    std::int64_t pregapSectors=0,postgapSectors=0,storedPregapSectors=0;
    bool hasSubchannels=false;int loopMode=0,loopOffset=0;
    bool isAudio()const noexcept{return mode==OpticalTrackMode::Audio;}
};
// A session owns its reference to the document. Engine composition supplies it.
class OpticalDiscSession final {
public:
    RETROMEDIALAB_MEDIAENGINE_API static std::future<std::unique_ptr<OpticalDiscSession>> openAsync(std::filesystem::path,std::stop_token={},bool includeCompanionAudio=false);
    RETROMEDIALAB_MEDIAENGINE_API explicit OpticalDiscSession(std::shared_ptr<const MediaDocument>);
    RETROMEDIALAB_MEDIAENGINE_API ~OpticalDiscSession()noexcept;
    OpticalDiscSession(const OpticalDiscSession&)=delete;OpticalDiscSession& operator=(const OpticalDiscSession&)=delete;
    RETROMEDIALAB_MEDIAENGINE_API std::vector<OpticalDiscTrack> tracks()const;
    RETROMEDIALAB_MEDIAENGINE_API std::string formatId()const;
    RETROMEDIALAB_MEDIAENGINE_API bool audioBigEndian()const;
    // Destination must remain alive until the future has completed.
    RETROMEDIALAB_MEDIAENGINE_API std::future<void> readStoredSectorAsync(int trackIndex,std::int64_t relativeSector,std::span<std::uint8_t>,std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::optional<std::vector<std::uint8_t>>> readSubchannelAsync(int trackIndex,std::int64_t relativeSector,std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API std::future<void> precacheAsync(std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API void close();
private:
    OpticalDiscSession(std::shared_ptr<const MediaDocument>,bool resetOnFailure);
    struct State;std::shared_ptr<State> state_;
};
}
