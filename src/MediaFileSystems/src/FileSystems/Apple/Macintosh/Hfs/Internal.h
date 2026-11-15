#pragma once
#include <RetroMediaLab/MediaFileSystems/MacHfs.h>
#include "Constants.h"
#include "../Primitives.h"
#include "../../../../Primitives/Binary.h"
#include <set>
namespace RetroMediaLab::MediaFileSystems::MacHfs::Internal {
struct ExtentResult {std::vector<std::uint8_t> content;bool valid=false;std::vector<std::int32_t> missing;std::uint64_t remaining=0;};
struct Record {std::uint32_t parent=0,id=0;std::string name;bool directory=false;std::uint64_t size=0;std::optional<Timestamp> modified;std::string type;bool defaultType=false;std::vector<std::uint8_t> data,resources;bool valid=false;};
ExtentResult extents(const SectorImage&,Bytes,std::uint16_t start,std::uint32_t size,std::uint32_t length,std::stop_token);
void extentWarnings(std::string_view,Macintosh::ForkKind,std::uint32_t,const ExtentResult&,std::vector<Diagnostic>&);
std::vector<Record> catalog(Bytes,const SectorImage&,std::uint16_t,std::uint32_t,std::vector<Diagnostic>&,std::stop_token);
std::vector<FileSystemEntry> buildDirectory(const std::vector<Record>&,std::vector<Diagnostic>&,std::stop_token);
}
