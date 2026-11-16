#pragma once
#include <RetroMediaLab/MediaEngine/RuntimeDisk.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include "../Composition/Recognition.h"
#include "../Composition/Writing.h"
#include "../Models/Blocks/BlockImage.h"
#include "../Models/Sectors/Representation.h"
#include <mutex>
namespace RetroMediaLab::MediaEngine {
namespace RuntimeDetails {
struct Services {
    MediaRecognitionComposition reading=MediaRecognitionComposition::createDefault();
    MediaWritingComposition writing=MediaWritingComposition::createDefault(reading.driveService());
};
std::shared_ptr<Services> defaultServices();
std::int32_t coordinate(std::int64_t);
DrivePosition position(std::int64_t,MediaFileSystems::SectorAddress);
}
struct RuntimeDisk::State {
    using Attributes=std::optional<std::map<std::string,std::int32_t>>;
    mutable std::recursive_mutex mutex;
    std::shared_ptr<RuntimeDetails::Services> services;
    std::shared_ptr<const MediaDocument> document;
    bool readOnly=true,ownsMechanics=true,closed=false;
    State(std::shared_ptr<RuntimeDetails::Services>,std::shared_ptr<const MediaDocument>,bool writable,bool owns);
    virtual ~State()noexcept=default;
    void alive()const;void writable()const;void close();
    virtual std::int32_t size()const=0;virtual std::int64_t count()const=0;
    virtual std::int64_t cylinders()const=0;virtual std::int32_t heads()const=0;virtual std::int32_t sectorsPerTrack()const=0;
    virtual bool dirty()const=0;virtual void clear()noexcept=0;virtual void flush(std::stop_token)=0;
    virtual std::optional<std::int64_t> findSector(std::int32_t,std::int32_t,std::int32_t)const;
    virtual std::vector<std::int64_t> trackSectors(std::int32_t,std::int32_t)const;
    virtual MediaDiskSector read(std::int64_t,std::stop_token);
    virtual void write(std::int64_t,std::span<const std::uint8_t>,std::optional<bool>);
    virtual void updateMetadata(const std::map<std::string,std::string>&);
    virtual void prepareBlockFormat(const HardDiskGeometry&);
    virtual void formatTrack(std::int32_t,std::int32_t,std::span<const MediaDiskSector>,std::optional<std::int32_t>,std::optional<bool>);
    virtual FluxTrackData readTrack(std::int32_t,std::int32_t,std::string_view,std::uint32_t,std::uint32_t,const Attributes&)=0;
    virtual void writeTrackWindow(std::int32_t,std::int32_t,std::string_view,std::uint32_t,std::uint32_t,std::uint32_t,std::uint32_t,std::span<const std::uint32_t>,const Attributes&,bool)=0;
};
struct RuntimeDisk::DecodedState final:State {
    bool documentChanged=false;
    std::map<std::int64_t,std::vector<std::uint8_t>> changes;
    std::map<std::int64_t,bool> dataMarks;
    std::map<std::int64_t,std::shared_ptr<const MediaFileSystems::SectorBlock>> originals;
    using TrackAddress=std::pair<std::int32_t,std::int32_t>;
    struct ProjectionError {std::uint32_t cell;bool mfm;std::exception_ptr error;};
    std::map<TrackAddress,FluxTrackData> writtenTracks;
    std::map<TrackAddress,std::vector<MediaFileSystems::SectorBlock>> trackTemplates;
    std::map<TrackAddress,ProjectionError> trackProjectionErrors;
    std::map<TrackAddress,std::exception_ptr> winchesterProjectionErrors;
    void formatTrack(std::int32_t,std::int32_t,std::span<const MediaDiskSector>,std::optional<std::int32_t>,std::optional<bool>);
    FluxTrackData readTrack(std::int32_t,std::int32_t,std::string_view,std::uint32_t,std::uint32_t,const std::optional<std::map<std::string,std::int32_t>>&);
    void writeTrackWindow(std::int32_t,std::int32_t,std::string_view,std::uint32_t,std::uint32_t,std::uint32_t,std::uint32_t,std::span<const std::uint32_t>,const std::optional<std::map<std::string,std::int32_t>>&,bool);
    void retryTrackProjection();
    FluxTrackData readWinchesterTrack(std::int32_t,std::int32_t,std::uint32_t);
    void writeWinchesterWindow(std::int32_t,std::int32_t,std::uint32_t,std::uint32_t,std::uint32_t,std::span<const std::uint32_t>,bool);
    void projectWinchesterTrack(std::int32_t,std::int32_t);
    MediaDiskSector readWrittenWinchesterSector(std::int64_t,std::int32_t,std::int32_t);
    void writeWinchesterParameters(std::span<std::uint8_t>);
    void clearTracks(){writtenTracks.clear();trackTemplates.clear();trackProjectionErrors.clear();winchesterProjectionErrors.clear();}
    DecodedState(std::shared_ptr<RuntimeDetails::Services>,std::shared_ptr<const MediaDocument>,bool writable,bool owns);
    ~DecodedState()noexcept override;
    const MediaFileSystems::SectorImage* sectors()const;
    const BlockImage* blocks()const;
    std::int32_t size()const;
    std::int64_t count()const;
    std::int64_t cylinders()const;
    std::int32_t heads()const;
    std::int32_t sectorsPerTrack()const;
    void validate(std::int64_t)const;
    bool dirty()const override;
    void clear()noexcept override;
    std::optional<std::int64_t> findSector(std::int32_t,std::int32_t,std::int32_t)const override;
    std::vector<std::int64_t> trackSectors(std::int32_t,std::int32_t)const override;
    MediaFileSystems::SectorAddress address(std::int64_t)const;
    std::shared_ptr<const MediaFileSystems::SectorBlock> original(std::int64_t,std::stop_token={})const;
    MediaDiskSector read(std::int64_t,std::stop_token);
    void write(std::int64_t,std::span<const std::uint8_t>,std::optional<bool>);
    std::shared_ptr<const MediaDocument> updated()const;
    void flush(std::stop_token);
    void updateMetadata(const std::map<std::string,std::string>&);
    void prepareBlockFormat(const HardDiskGeometry&);
};
}
