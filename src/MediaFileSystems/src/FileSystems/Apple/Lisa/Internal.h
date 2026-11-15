#pragma once
#include <RetroMediaLab/MediaFileSystems/Lisa.h>
#include "Constants.h"
#include "../../../Primitives/Binary.h"
#include "../../../Primitives/Identifiers.h"
#include <algorithm>
namespace RetroMediaLab::MediaFileSystems::Lisa::Internal {
struct Tag {std::uint16_t fileId;std::int32_t pageNumber;};
struct Page {std::shared_ptr<const SectorBlock> block;std::optional<Tag> tag;};
struct Mddf {std::uint16_t version;std::string volumeName;};
struct Content {std::vector<std::uint8_t> bytes;bool valid=true;std::uint64_t firstBlock=0;};
std::optional<Tag> tag(const SectorBlock&);
bool userFile(std::uint16_t) noexcept;
std::vector<Page> pages(const SectorImage&,std::stop_token);
std::string hex(std::uint16_t);
void append(std::vector<std::uint8_t>&,Bytes);
Mddf mddf(const std::vector<Page>&);
CatalogResult catalog(const std::vector<Page>&,std::uint16_t,std::stop_token);
std::map<std::uint16_t,Content> contents(const std::vector<Page>&,std::vector<Diagnostic>&,std::stop_token);
}

