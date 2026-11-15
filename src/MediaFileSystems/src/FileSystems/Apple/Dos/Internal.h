#pragma once
#include <RetroMediaLab/MediaFileSystems/AppleDos.h>
#include "Constants.h"
#include "../../../Primitives/Binary.h"
#include <optional>
namespace RetroMediaLab::MediaFileSystems::AppleDos::Internal {
struct Vtoc {std::vector<std::uint8_t> data;std::int32_t tracks,sectors,catalogTrack,catalogSector,freeSectors;std::uint8_t number;};
struct FileData {std::vector<std::uint8_t> content;bool valid=true;std::int64_t reference=-1;std::uint64_t traversed=0;};
bool validAddress(std::int64_t track,std::int64_t sector,std::uint32_t tracks,std::uint32_t sectors) noexcept;
bool sameId(std::string_view,std::string_view) noexcept;
std::optional<Vtoc> vtoc(const SectorImage&,std::stop_token);
FileData file(const SectorImage&,std::int32_t track,std::int32_t sector,std::vector<Diagnostic>& warnings,std::string_view name,std::stop_token);
}
