#pragma once
#include <RetroMediaLab/MediaFileSystems/SpartaDos.h>
#include "Constants.h"
#include "../../../Primitives/Binary.h"
#include <set>
namespace RetroMediaLab::MediaFileSystems::SpartaDos::Internal {
enum class Flags:std::uint8_t {None=0,Protected=1,Hidden=2,Archived=4,InUse=8,Deleted=16,Subdirectory=32};
struct Header {std::int32_t root=0,total=0,free=0,bitmapCount=0,firstBitmap=0,firstFile=0,firstDirectory=0,tracks=0,size=0,bootMap=0;std::string name;bool doubleSided=false,writeProtected=false;std::uint8_t version=0,sequence=0,random=0;};
struct Allocation {std::vector<std::int32_t> sectors;std::size_t maps=0;};
struct File {std::vector<std::uint8_t> bytes;std::uint64_t occupied=0;bool sparse=false;};
struct Directory {std::string name;std::int32_t length=0;std::uint64_t occupied=0;std::vector<FileSystemEntry> children;};
bool has(std::uint8_t,Flags)noexcept;
std::string name(Bytes);
std::string fileName(Bytes,Bytes);
std::uint32_t length(Bytes,std::size_t);
std::pair<bool,std::optional<Timestamp>> timestamp(Bytes,Bytes);
std::optional<Header> header(const SectorImage&,std::stop_token);
std::shared_ptr<const SectorBlock> trySector(const SectorImage&,std::int32_t,std::stop_token);
std::shared_ptr<const SectorBlock> sector(const SectorImage&,std::int32_t,std::stop_token);
void validateSector(std::int32_t,std::uint64_t);
Allocation allocation(const SectorImage&,std::int32_t,std::int32_t,std::stop_token);
std::int32_t firstData(const SectorImage&,std::int32_t,std::stop_token);
File file(const SectorImage&,std::int32_t,std::int32_t,std::stop_token);
Directory directory(const SectorImage&,std::int32_t,std::set<std::int32_t>,std::stop_token);
}
