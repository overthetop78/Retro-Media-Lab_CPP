#pragma once
#include <RetroMediaLab/MediaFileSystems/Cpm.h>
#include "Constants.h"
#include "../../Primitives/Binary.h"
#include "../../Primitives/Identifiers.h"
#include <algorithm>
#include <set>
namespace RetroMediaLab::MediaFileSystems::Cpm::Internal {
struct Layout {
 std::int32_t directoryOffset,allocationOrigin,directoryEntries,allocationSize,directoryBlocks;bool wide;
 Layout(std::int32_t,std::int32_t,std::int32_t,std::int32_t,std::int32_t,bool);
};
struct LogicalImage {std::vector<std::uint8_t> bytes;std::uint32_t blockSize;std::set<std::int32_t> missing,truncated;bool available(std::int64_t offset,std::int64_t length)const;};
struct Extent {std::uint8_t user;std::string name;std::int32_t number;std::uint8_t recordCount;std::vector<std::int32_t> allocations;};
struct Directory {std::string volumeName;std::vector<Extent> extents;};
struct Group {std::uint8_t user;std::string name;std::vector<Extent> extents;};
struct File {std::vector<std::uint8_t> content;bool valid=true;std::set<std::int32_t> used;bool rejected=false;};
bool equal(std::string_view,std::string_view);
bool lessName(std::string_view,std::string_view);
LogicalImage flatten(const SectorImage&,std::stop_token);
std::optional<Layout> genericLayout(std::string_view);
Layout epsonLayout(std::string_view,const LogicalImage&,const Layout&,std::stop_token);
std::optional<Layout> amstradLayout(const SectorImage&,const LogicalImage&,std::stop_token);
bool decodeName(Bytes,bool,std::string&);
std::string decodePart(Bytes);
std::vector<std::int32_t> allocations(Bytes,bool);
bool plausibleLabel(std::string_view);
std::int32_t score(const LogicalImage&,const Layout&,bool,std::stop_token);
bool looksLike(const LogicalImage&,const Layout&,bool allowEmpty,bool rejectLowercase,std::stop_token);
std::optional<Layout> findDirectory(const LogicalImage&,const Layout&,std::int32_t step,bool allowEmpty,bool rejectLowercase,std::stop_token);
Directory directory(const LogicalImage&,const Layout&,bool,std::stop_token);
std::vector<Group> groups(const std::vector<Extent>&);
File reconstruct(const LogicalImage&,const Layout&,const Group&,std::vector<Diagnostic>&,std::stop_token);
FileSystemVolume volume(const SectorImage&,const LogicalImage&,const Layout&,const Directory&,std::string_view,std::stop_token);
}
