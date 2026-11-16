#pragma once
#include "Export.h"
#include "HardDiskGeometry.h"
#include "DriveMechanics.h"
#include "FluxTrackData.h"
#include <filesystem>
#include <future>
#include <map>
#include <memory>
#include <span>
#include <stop_token>
namespace RetroMediaLab::MediaEngine {
struct MediaDiskSector {
    std::int64_t index=0;
    std::vector<std::uint8_t> data;
    std::int32_t cylinder=0,head=0,number=0;
    std::optional<bool> integrityValid;
    std::optional<std::uint8_t> diagnosticCode;
    std::vector<std::uint8_t> recordedErrorCheckCode,recordedDataPrefix;
    std::optional<bool> headerIntegrityValid,dataIntegrityValid,deletedDataMark;
    std::optional<std::uint8_t> recordedSizeCode;
};
// Decoded sector/block images. Physical flux and MFM runtimes are separate backends.
// The caller serializes commands and calls close() to observe any final flush error.
class RuntimeDisk final {
public:
    RETROMEDIALAB_MEDIAENGINE_API static std::future<std::unique_ptr<RuntimeDisk>> openAsync(std::filesystem::path,bool writable=false,std::optional<std::string> format={},std::stop_token={},std::shared_ptr<DriveMechanics> mechanics={});
    RETROMEDIALAB_MEDIAENGINE_API ~RuntimeDisk()noexcept;
    RuntimeDisk(const RuntimeDisk&)=delete;
    RuntimeDisk& operator=(const RuntimeDisk&)=delete;
    RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<DriveMechanics> mechanics()const;
    RETROMEDIALAB_MEDIAENGINE_API std::string formatId()const;
    RETROMEDIALAB_MEDIAENGINE_API std::int32_t sectorSize()const;
    RETROMEDIALAB_MEDIAENGINE_API std::int64_t sectorCount()const;
    RETROMEDIALAB_MEDIAENGINE_API std::int64_t cylinders()const;
    RETROMEDIALAB_MEDIAENGINE_API std::int32_t heads()const;
    RETROMEDIALAB_MEDIAENGINE_API std::int32_t sectorsPerTrack()const;
    RETROMEDIALAB_MEDIAENGINE_API bool isReadOnly()const;
    RETROMEDIALAB_MEDIAENGINE_API bool hasChanges()const;
    RETROMEDIALAB_MEDIAENGINE_API std::map<std::string,std::string> metadata()const;
    RETROMEDIALAB_MEDIAENGINE_API void updateMetadata(const std::map<std::string,std::string>&);
    RETROMEDIALAB_MEDIAENGINE_API void prepareBlockFormat(const HardDiskGeometry&);
    RETROMEDIALAB_MEDIAENGINE_API std::optional<std::int64_t> findSector(std::int32_t cylinder,std::int32_t head,std::int32_t number)const;
    RETROMEDIALAB_MEDIAENGINE_API std::vector<std::int64_t> trackSectors(std::int32_t cylinder,std::int32_t head)const;
    RETROMEDIALAB_MEDIAENGINE_API std::future<MediaDiskSector> readSectorAsync(std::int64_t,std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API void writeSector(std::int64_t,std::span<const std::uint8_t>,std::optional<bool> deletedDataMark={});
    RETROMEDIALAB_MEDIAENGINE_API FluxTrackData readTrack(std::int32_t cylinder,std::int32_t head,std::string_view encodingId,std::uint32_t bitCellNanoseconds,std::uint32_t indexTimeNanoseconds,const std::optional<std::map<std::string,std::int32_t>>& attributes={})const;
    RETROMEDIALAB_MEDIAENGINE_API void writeTrackWindow(std::int32_t cylinder,std::int32_t head,std::string_view encodingId,std::uint32_t bitCellNanoseconds,std::uint32_t indexTimeNanoseconds,std::uint32_t startNanoseconds,std::uint32_t durationNanoseconds,std::span<const std::uint32_t> transitionOffsetsNanoseconds,const std::optional<std::map<std::string,std::int32_t>>& attributes={},bool includeEndingInstant=false);
    RETROMEDIALAB_MEDIAENGINE_API void formatTrack(std::int32_t cylinder,std::int32_t head,std::span<const MediaDiskSector>,std::optional<std::int32_t> dataRate={},std::optional<bool> mfm={});
    RETROMEDIALAB_MEDIAENGINE_API std::future<void> flushAsync(std::stop_token={});
    RETROMEDIALAB_MEDIAENGINE_API void close();
private:
    struct State;std::shared_ptr<State> state_;
    explicit RuntimeDisk(std::shared_ptr<State> value):state_(std::move(value)){}
};
}
