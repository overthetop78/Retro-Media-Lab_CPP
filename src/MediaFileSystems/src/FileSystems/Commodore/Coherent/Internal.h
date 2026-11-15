#pragma once
#include <RetroMediaLab/MediaFileSystems/Coherent.h>
#include "Constants.h"
#include "../../../Primitives/Binary.h"
#include <array>
#include <map>
#include <limits>
#include <set>
namespace RetroMediaLab::MediaFileSystems::Coherent::Internal {
using namespace Constants;
class Image {
 const SectorImage& source_;std::map<std::uint64_t,std::shared_ptr<const SectorBlock>> cache_;
public:
 std::stop_token token;explicit Image(const SectorImage& source,std::stop_token t):source_(source),token(t){}
 std::uint64_t blockCount()const{return source_.blockCount();}
 std::shared_ptr<const SectorBlock> block(std::uint64_t);
 bool present(std::uint64_t offset,std::uint64_t length);
 std::vector<std::uint8_t> bytes(std::uint64_t offset,std::size_t length);
};
struct Inode {std::uint16_t mode;std::uint32_t size,modified;std::array<std::uint32_t,PointerCount> blocks;};
struct FileData {std::vector<std::uint8_t> content;bool valid=true;};
struct DirectoryData {std::vector<FileSystemEntry> entries;bool valid=true;};
Inode inode(Image&,std::uint32_t zoneEnd,std::uint16_t number);
FileData file(Image&,const Inode&,std::vector<Diagnostic>&,const std::string& name);
DirectoryData directory(Image&,std::uint32_t zoneEnd,std::uint16_t number,std::set<std::uint16_t>& stack,std::set<std::uint16_t>& visited,std::vector<Diagnostic>&,const std::string& name,std::uint32_t depth);
}
