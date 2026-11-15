#pragma once
#include <RetroMediaLab/MediaFileSystems/Fat12.h>
#include "Constants.h"
#include "FormatCatalog.h"
#include <algorithm>
#include <set>
namespace RetroMediaLab::MediaFileSystems::Fat12::Internal {
struct Range {
    std::vector<std::uint8_t> bytes;
    std::vector<bool> present;
    bool trusted = true;
    bool complete() const { return std::all_of(present.begin(),present.end(),[](bool value){return value;}); }
    bool valid() const { return complete() && trusted; }
};
struct Chain { std::vector<std::uint8_t> bytes;bool valid=true,trusted=true;std::set<std::uint32_t> clusters; };
struct ReadContext {
    const SectorImage& image;
    std::vector<Diagnostic>& warnings;
    std::stop_token token;
    std::uint64_t remainingBytes;
    std::set<std::uint32_t> expandedDirectories;
};
bool same(std::string_view,std::string_view);
bool dosPlus(const SectorImage&);
bool supported(std::string_view);
Range sectors(const SectorImage&,std::uint64_t first,std::uint32_t count,std::vector<Diagnostic>&,std::stop_token);
bool usableTable(Bytes,std::uint8_t mediaDescriptor,std::uint32_t clusterCount);
Range bestTable(const SectorImage&,const Layout&,std::uint8_t descriptor,std::vector<Diagnostic>&,std::stop_token);
Chain clusterChain(ReadContext&,const Range& fat,const Layout&,std::uint32_t first,std::string_view name);
std::vector<FileSystemEntry> directory(ReadContext&,const Range&,const Range& fat,const Layout&,std::uint32_t depth,const std::string& path);
MediaError unsupported(const SectorImage&,const std::shared_ptr<const SectorBlock>& boot);
}
