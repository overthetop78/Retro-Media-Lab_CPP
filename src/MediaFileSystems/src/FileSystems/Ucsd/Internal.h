#pragma once
#include <RetroMediaLab/MediaFileSystems/Ucsd.h>
#include "Constants.h"
#include "../../Primitives/Binary.h"
#include <set>
namespace RetroMediaLab::MediaFileSystems::Ucsd::Internal {
struct Blocks {std::vector<std::uint8_t> bytes;std::vector<bool> present;std::vector<std::int32_t> missing;bool valid()const noexcept{return missing.empty();}};
struct Header {std::int32_t end=0,total=0,count=0,blocks=0;std::string name;std::optional<Timestamp> date;ByteOrder order=ByteOrder::LittleEndian;};
struct Directory {std::vector<FileSystemEntry> entries;std::set<std::int32_t> used;bool valid=false;};
struct Content {std::vector<std::uint8_t> bytes;bool valid=false;std::vector<std::int32_t> missing;std::uint64_t size=0;};
std::uint16_t word(Bytes,std::size_t,ByteOrder);
bool validName(Bytes)noexcept;
std::string name(Bytes,std::int32_t maximum);
std::optional<Timestamp> date(std::uint16_t);
bool directoryEnd(std::int32_t)noexcept;
std::optional<ByteOrder> detectOrder(Bytes)noexcept;
std::optional<Header> header(const SectorImage&,std::stop_token);
Blocks blocks(const SectorImage&,std::int32_t first,std::int32_t count,std::stop_token);
Content content(const SectorImage&,std::int32_t first,std::int32_t end,std::int32_t lastBytes,std::string_view name,std::vector<Diagnostic>&,std::stop_token);
Directory directory(const SectorImage&,const Header&,const Blocks&,std::vector<Diagnostic>&,std::stop_token);
std::string join(const std::vector<std::int32_t>&);
bool sameFormat(std::string_view)noexcept;
std::size_t kindIndex(FileKind)noexcept;
}
