#pragma once
#include "ExplorationInfo.h"
#include "DiskMetadata.h"
#include "ScpHeader.h"
#include <functional>
#include <span>
namespace RetroMediaLab::MediaEngine {
using DiskBytes=std::shared_ptr<const std::vector<std::uint8_t>>;
enum class DiskSectorState {Available,Invalid,Missing};
enum class DiskRevolutionOrigin {Captured,Synthetic};
struct DiskSectorInfo {
 std::uint64_t logicalBlock=0,size=0;
 std::int64_t cylinder=0,head=0,number=0;
 DiskSectorState state=DiskSectorState::Missing;
 DiskBytes data,tag;
 std::optional<bool> headerIntegrityValid,dataValid;
 std::optional<std::uint8_t> formatCode,diagnosticCode;
 std::vector<std::int32_t> revolutions;
};
struct DiskSourceSectorInfo {std::int64_t number=0;std::uint64_t size=0;DiskBytes data;};
struct DiskRevolutionInfo {
 std::int32_t number=0,resolutionNanoseconds=0;
 std::int64_t startNanoseconds=0,durationNanoseconds=0;
 std::optional<std::uint32_t> declaredFluxCount;
 DiskRevolutionOrigin origin=DiskRevolutionOrigin::Captured;
 std::shared_ptr<const std::vector<std::uint32_t>> fluxIntervals;
};
struct DiskTrackInfo {std::optional<std::int32_t> sourceNumber;std::int32_t cylinder=0,head=0;std::vector<DiskRevolutionInfo> revolutions;std::vector<DiskSourceSectorInfo> sourceSectors;};
struct DiskCaptureSnapshot {Scp::Header header;std::vector<DiskTrackInfo> tracks;bool checksumValid=false;std::uint64_t fileSize=0;};
using DiskCaptureLoadedCallback=std::function<void(std::shared_ptr<const DiskCaptureSnapshot>)>;
struct DiskContainerInfo {
 std::optional<std::string> signature,diskType;
 std::optional<std::int32_t> resolutionNanoseconds,revolutionCount,firstTrack,lastTrack;
 std::uint64_t trackCount=0;
 std::optional<std::uint32_t> headCount;
 bool checksumPresent=false;
 std::optional<std::string> declaredChecksum,computedChecksum;
 std::optional<bool> checksumValid;
 std::map<std::string,std::string> formatProperties;
};
struct DiskFormatInfo {
 std::string machineId,formatId,encodingId;
 std::uint32_t cylinders=0,heads=0;
 std::optional<std::uint32_t> sectorsPerTrack,sectorSize;
 std::uint64_t capacity=0,validSectors=0,invalidSectors=0,missingSectors=0,entryCount=0;
 std::vector<DiskSectorInfo> sectors;
 std::shared_ptr<const FileSystemVolumeInfo> volume;
 std::optional<std::string> volumeName;
 std::optional<std::uint64_t> usedBytes,freeBytes;
 std::optional<bool> bootable;
 DiskContentMetadata content;
 std::optional<std::string> protectionId,loaderId;
 std::vector<DiagnosticInfo> diagnostics;
 std::span<const FileSystemEntryInfo> entries()const noexcept{return volume?std::span<const FileSystemEntryInfo>(volume->entries):std::span<const FileSystemEntryInfo>{};}
};
struct DiskImageInfo {
 std::string sourcePath,imageType,primaryFormatId;
 std::optional<std::uint8_t> version;
 std::uint64_t fileSize=0;
 DiskContainerInfo container;
 DiskImageMetadata metadata;
 std::shared_ptr<const FileSystemVolumeInfo> volume;
 std::vector<DiskTrackInfo> tracks;
 std::vector<DiskFormatInfo> formats;
 std::vector<std::string> detectedFormatIds;
 std::vector<DiagnosticInfo> diagnostics;
 bool usesCustomSectorLoader()const noexcept{return (!volume||volume->fileSystemId.empty())&&metadata.content.hasCataloglessOrganization();}
};
enum class DiskExplorationStage {FormatProbeStarted,FormatProbeCompleted,CandidateStarted,RevolutionDecoded,TrackDecoded,CandidateCompleted};
struct DiskExplorationProgress {DiskExplorationStage kind;std::string detail;std::size_t completed,total;std::optional<bool> recognized;std::optional<std::int32_t> cylinder,head;};
using DiskExplorationProgressCallback=std::function<void(const DiskExplorationProgress&)>;
}
