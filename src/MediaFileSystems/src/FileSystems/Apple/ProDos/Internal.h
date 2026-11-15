#pragma once
#include <RetroMediaLab/MediaFileSystems/ProDos.h>
#include "Constants.h"
#include "../../../Primitives/Binary.h"
#include <set>
namespace RetroMediaLab::MediaFileSystems::ProDos::Internal {
struct Header {std::string name;std::int32_t bitmapBlock,totalBlocks;std::optional<Timestamp> created;std::vector<std::uint8_t> root;};
struct Directory {std::vector<FileSystemEntry> entries;bool valid=true;};
struct Content {std::vector<std::uint8_t> data;bool valid=true;std::set<std::int32_t> dataBlocks,indexBlocks;};
struct Bitmap {std::int32_t freeBlocks=0;bool valid=true;};
std::string hexByte(std::uint8_t);
std::string storageName(Storage);
std::uint32_t read24(Bytes,std::size_t);
void write16(MutableBytes,std::size_t,std::int64_t);
void write24(MutableBytes,std::size_t,std::int64_t);
void writeIndex(MutableBytes,std::size_t,std::int32_t);
std::int32_t readIndex(Bytes,std::size_t);
std::optional<Header> header(const SectorImage&,std::stop_token);
Directory directory(const SectorImage&,std::int32_t,std::vector<Diagnostic>&,std::set<std::int32_t>&,std::uint32_t,std::stop_token);
Content content(const SectorImage&,Storage,std::int32_t,std::uint32_t,std::string_view,std::vector<Diagnostic>&,std::stop_token);
Bitmap bitmap(const SectorImage&,std::int32_t,std::int32_t,std::vector<Diagnostic>&,std::stop_token);
void setType(FileSystemEntry&,std::uint8_t);
bool sameId(std::string_view,std::string_view) noexcept;
}
