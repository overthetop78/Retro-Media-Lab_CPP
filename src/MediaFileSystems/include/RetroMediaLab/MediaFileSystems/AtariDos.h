#pragma once
#include "SectorImage.h"
#include "FileSystemModels.h"
#include "AtariDosConstants.h"
#include <map>
namespace RetroMediaLab::MediaFileSystems::AtariDos {
enum class DirectoryFlags:std::uint8_t {None=0,OpenForOutput=LayoutConstants::OpenForOutputFlag,CreatedByDos2=LayoutConstants::CreatedByDos2Flag,DoubleDensity=LayoutConstants::DoubleDensityFlag,Locked=LayoutConstants::LockedFlag,InUse=LayoutConstants::InUseFlag,Deleted=LayoutConstants::DeletedFlag};
struct DirectoryLocation {std::int32_t firstSector,sectorCount,entriesPerSector;bool isCanonical()const noexcept{return firstSector==LayoutConstants::FirstDirectorySector;}};
struct DirectoryEntry {std::int32_t entryNumber,directorySector,directorySlot;std::string name;DirectoryFlags flags;std::int32_t declaredSectors,firstSector;};
struct SectorChain {std::vector<std::int32_t> sectors;std::vector<std::optional<std::int32_t>> storedFileNumbers;std::int32_t continuationSector=0;bool complete=false;};
struct FileData {std::vector<std::uint8_t> content;SectorChain chain;bool valid=false;std::vector<std::string> attributes;std::vector<Diagnostic> diagnostics;};
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::int32_t extendedVtocSectorCount(std::uint8_t,std::int32_t sectorSize) noexcept;
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool vtocLooksValid(Bytes,std::int32_t maximumSectors) noexcept;
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool trySector(const SectorImage&,std::int32_t,std::vector<std::uint8_t>&,std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<std::int32_t> freeSectors(const SectorImage&,std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string decodeName(Bytes);
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool directoryLooksValid(Bytes);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<DirectoryLocation> locateDirectory(const SectorImage&,std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool containsRecordedEntry(const SectorImage&,std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool containsReadableEntry(const SectorImage&,std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::vector<FileSystemEntry> readDirectory(const SectorImage&,std::vector<Diagnostic>&,std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::vector<FileSystemEntry> readDirectory(const SectorImage&,const DirectoryLocation&,std::vector<Diagnostic>&,std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API FileData readFile(const SectorImage&,std::int32_t first,std::int32_t expectedSectors,bool extendedLinks,std::vector<Diagnostic>&,std::string_view name,std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::map<std::int32_t,FileData> analyzeAllocations(const std::vector<DirectoryEntry>&,const std::map<std::int32_t,FileData>&,std::vector<Diagnostic>&,std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::span<const std::string_view> catalogFormatIds() noexcept;
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool canRead(const SectorImage&,std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemVolume read(const SectorImage&,std::stop_token = {});
}
