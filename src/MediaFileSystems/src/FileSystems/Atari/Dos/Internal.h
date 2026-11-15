#pragma once
#include <RetroMediaLab/MediaFileSystems/AtariDos.h>
#include "Constants.h"
#include "../../../Primitives/Binary.h"
#include "../../../Primitives/Identifiers.h"
#include <algorithm>
#include <set>
namespace RetroMediaLab::MediaFileSystems::AtariDos::Internal {
bool extended(const SectorImage&,std::stop_token);
bool validEntry(Bytes,std::size_t,std::uint8_t,bool validateDeletedName);
std::optional<std::vector<DirectoryEntry>> parseEntries(const SectorImage&,const DirectoryLocation&,bool realRead,bool requireValid,bool& ended,std::vector<Diagnostic>*,std::stop_token);
std::map<std::int32_t,FileData> files(const SectorImage&,const std::vector<DirectoryEntry>&,std::vector<Diagnostic>&,std::stop_token);
bool analyzeDirectory(const SectorImage&,const DirectoryLocation&,bool relocated,std::int32_t& recorded,std::int32_t& readable,std::stop_token);
DirectoryLocation canonical() noexcept;
}
