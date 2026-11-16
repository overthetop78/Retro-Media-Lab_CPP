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
    mutable std::recursive_mutex mutex;
    std::shared_ptr<RuntimeDetails::Services> services;
    std::shared_ptr<const MediaDocument> document;
    bool readOnly=true,ownsMechanics=true,closed=false,documentChanged=false;
    std::map<std::int64_t,std::vector<std::uint8_t>> changes;
    std::map<std::int64_t,bool> dataMarks;
    std::map<std::int64_t,std::shared_ptr<const MediaFileSystems::SectorBlock>> originals;
    State(std::shared_ptr<RuntimeDetails::Services>,std::shared_ptr<const MediaDocument>,bool writable,bool owns);
    ~State()noexcept;
    const MediaFileSystems::SectorImage* sectors()const;
    const BlockImage* blocks()const;
    std::int32_t size()const;
    std::int64_t count()const;
    std::int64_t cylinders()const;
    std::int32_t heads()const;
    std::int32_t sectorsPerTrack()const;
    void alive()const;
    void validate(std::int64_t)const;
    void writable()const;
    bool dirty()const;
    MediaFileSystems::SectorAddress address(std::int64_t)const;
    std::shared_ptr<const MediaFileSystems::SectorBlock> original(std::int64_t,std::stop_token={})const;
    MediaDiskSector read(std::int64_t,std::stop_token);
    void write(std::int64_t,std::span<const std::uint8_t>,std::optional<bool>);
    std::shared_ptr<const MediaDocument> updated()const;
    void flush(std::stop_token);
    void close();
    void updateMetadata(const std::map<std::string,std::string>&);
    void prepareBlockFormat(const HardDiskGeometry&);
};
}
