#pragma once
#include "Constants.h"
#include "../Optical/Common.h"
#include "../../Primitives/Binary.h"
namespace RetroMediaLab::MediaFileSystems::Udf {
using namespace UdfConstants;
template<class T> T little(Bytes data,std::size_t offset){return Binary::little<T>(Optical::slice(data,offset,sizeof(T)),0);}

struct Context {std::shared_ptr<const OpticalTrack> track;std::uint32_t start,length;std::uint16_t revision;std::string name;std::uint32_t root;};
struct Icb {std::uint32_t block;std::uint8_t type;std::uint64_t length;std::uint16_t allocationType;std::vector<std::uint8_t> allocations;std::optional<Timestamp> accessed,modified,created;};
void validate(Bytes,std::uint16_t,std::optional<std::uint32_t>);
std::string compressedName(Bytes);
std::string dstring(Bytes);
std::string revision(std::uint16_t);
std::optional<Timestamp> timestamp(Bytes,std::size_t);
std::vector<std::uint8_t> physical(const OpticalTrack&,std::uint32_t,std::stop_token);
std::vector<std::uint8_t> partitionBlock(const Context&,std::uint32_t,std::stop_token);
std::pair<std::uint32_t,std::uint16_t> longAllocation(Bytes,std::size_t);
void primary(std::uint16_t);
Context context(const ImageDocument&,const VolumeDescriptor&,std::stop_token);
Icb icb(const Context&,std::uint32_t,std::stop_token);
std::vector<std::uint8_t> content(const Context&,const Icb&,std::stop_token);
std::vector<FileSystemEntry> directory(const Context&,const Icb&,std::set<std::uint32_t>&,std::stop_token);
}
