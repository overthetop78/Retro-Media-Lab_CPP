#pragma once
#include <RetroMediaLab/MediaFileSystems/Rt11.h>
#include "Constants.h"
#include "../../../Primitives/Binary.h"
#include <set>
namespace RetroMediaLab::MediaFileSystems::Rt11::Internal {
struct Pair {std::vector<std::uint8_t> bytes;bool firstPresent=false,secondPresent=false,firstValid=false,secondValid=false;bool valid()const noexcept{return firstPresent&&secondPresent&&firstValid&&secondValid;}};
struct Content {std::vector<std::uint8_t> bytes;bool valid=false;std::vector<std::int32_t> missing;};
struct Directory {std::vector<FileSystemEntry> entries;std::uint64_t free=0;bool valid=true;std::vector<Diagnostic> warnings;};
bool has(std::uint16_t,EntryStatus)noexcept;
bool sameFormat(std::string_view)noexcept;
Pair pair(const SectorImage&,std::int32_t,std::stop_token);
Content content(const SectorImage&,std::int32_t,std::int32_t,std::stop_token);
Directory directory(const SectorImage&,std::int32_t,std::stop_token);
std::string join(const std::vector<std::int32_t>&);
}
