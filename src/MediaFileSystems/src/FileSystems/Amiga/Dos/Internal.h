#pragma once
#include <RetroMediaLab/MediaFileSystems/AmigaDos.h>
#include "Constants.h"
#include "../../../Primitives/Binary.h"
#include <set>
namespace RetroMediaLab::MediaFileSystems::AmigaDos::Internal {
using namespace Constants;
inline std::int32_t integer(Bytes data,std::size_t offset){return Binary::big<std::int32_t>(data,offset);}
inline std::uint32_t unsignedInteger(Bytes data,std::size_t offset){return Binary::big<std::uint32_t>(data,offset);}
struct Root {Variant variant;std::int64_t number;std::vector<std::uint8_t> data;int hashSize;};
struct FileData {std::vector<std::uint8_t> content;bool valid;};
bool recordedIntegrity(const SectorBlock&) noexcept;
bool dosPrefix(Bytes);
std::optional<Root> root(const SectorImage&,std::stop_token);
std::optional<FileSystemVolume> recover(const SectorImage&,std::stop_token);
std::vector<FileSystemEntry> directory(const SectorImage&,Bytes,int,Variant,std::set<std::int64_t>&,std::vector<Diagnostic>&,std::uint32_t,std::stop_token);
FileData file(const SectorImage&,Bytes,std::uint32_t,Variant,std::vector<Diagnostic>&,std::stop_token);
void sort(std::vector<FileSystemEntry>&);
bool recoverableName(std::string_view);
void setChecksum(MutableBytes,std::size_t offset);
void writeDate(MutableBytes,std::size_t,const Timestamp&);
std::uint32_t nameHash(std::string_view);
}
