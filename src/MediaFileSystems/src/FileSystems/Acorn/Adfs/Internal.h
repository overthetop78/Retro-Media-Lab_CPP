#pragma once
#include <RetroMediaLab/MediaFileSystems/Adfs.h>
#include <RetroMediaLab/MediaFileSystems/FileCore.h>
#include "Constants.h"
#include <set>
namespace RetroMediaLab::MediaFileSystems::Adfs::Internal {
struct Context {
    const SectorImage& image;
    std::unique_ptr<FileCore::AddressResolver> resolver;
    std::vector<Diagnostic> warnings;
    bool mapIntegrity = true;
};
struct Directory {std::string name,title;std::vector<FileSystemEntry> entries;bool integrity=true;};
std::unique_ptr<FileCore::AddressResolver> layout(const SectorImage&,std::stop_token,std::vector<std::uint64_t>* mapBlocks = nullptr);
bool validIntegrity(const SectorBlock&) noexcept;
bool readBytes(const SectorImage&,const FileCore::AddressResolver&,std::int32_t,std::span<std::uint8_t>,bool&,std::vector<Diagnostic>*,std::stop_token);
std::optional<std::vector<std::uint8_t>> directoryBytes(const SectorImage&,const FileCore::AddressResolver&,std::int32_t,bool&,std::vector<Diagnostic>*,std::stop_token);
Directory directory(Context&,std::int32_t,std::set<std::int32_t>&,std::uint32_t,bool,std::stop_token);
std::optional<std::vector<std::uint8_t>> file(Context&,std::int32_t,std::uint32_t,const std::string&,FileSystemEntry&,std::stop_token);
}
