#pragma once
#include <RetroMediaLab/MediaFileSystems/MacMfs.h>
#include "Constants.h"
#include "../Primitives.h"
#include "../../../../Primitives/Binary.h"
#include <span>
namespace RetroMediaLab::MediaFileSystems::MacMfs::Internal {
struct Chain {std::vector<std::int32_t> clusters;bool valid=false,cycle=false,outOfRange=false,premature=false;};
class AllocationMap {
public:
    static AllocationMap decode(Bytes,std::int32_t count);
    Chain traverse(std::int32_t first,std::int32_t required)const;
private:std::vector<std::uint16_t> entries;
};
struct Blocks {std::vector<std::uint8_t> bytes;std::vector<bool> present;bool valid=false;};
struct Fork {std::vector<std::uint8_t> content;bool valid=false;std::vector<std::int32_t> missing,clusters;};
enum class BlockContext {Volume,Directory};
using ForkKind=Macintosh::ForkKind;
Blocks readBlocks(const SectorImage&,std::int32_t first,std::int32_t count,BlockContext,std::vector<Diagnostic>&,std::stop_token);
Fork readFork(const SectorImage&,const AllocationMap&,std::int32_t allocationStart,std::uint32_t allocationSize,std::int32_t firstCluster,std::uint32_t logicalLength,std::string_view file,ForkKind,std::vector<Diagnostic>&,std::stop_token);
std::vector<FileSystemEntry> directory(const SectorImage&,std::int32_t start,std::int32_t count,const AllocationMap&,std::int32_t allocationStart,std::uint32_t allocationSize,std::vector<Diagnostic>&,std::stop_token);
}
