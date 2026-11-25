#pragma once
#include "Export.h"
#include "HardDiskGeometry.h"
#include <filesystem>
#include <future>
#include <map>
#include <optional>
#include <stop_token>
#include <string>
namespace RetroMediaLab::MediaEngine {
namespace HardDiskPreparationIds {
inline constexpr auto AmigaRdbOfs = "amiga-rdb-ofs";
inline constexpr auto AmigaRdbFfs = "amiga-rdb-ffs";
inline constexpr auto AtariAhdiFat16 = "atari-ahdi-fat16";
inline constexpr auto AtariIdeFat16 = "atari-ide-fat16";
}
RETROMEDIALAB_MEDIAENGINE_API std::future<void> extractAtrPayloadAsync(std::filesystem::path source,std::filesystem::path output,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::future<void> createBlankDiskAsync(std::filesystem::path,std::string formatId,std::stop_token={});
// HDD creation never overwrites an existing target or descriptor.
RETROMEDIALAB_MEDIAENGINE_API std::future<void> createHardDiskAsync(std::filesystem::path,std::string formatId,HardDiskGeometry,std::optional<std::string> fileSystemId={},std::stop_token={},bool preallocate=false);
RETROMEDIALAB_MEDIAENGINE_API std::future<void> convertScpAsync(std::filesystem::path source,std::filesystem::path output,std::string outputExtension,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API const std::map<std::string,std::string>& mediaFileExtensions();
RETROMEDIALAB_MEDIAENGINE_API std::int32_t rawOpticalSectorSize()noexcept;
RETROMEDIALAB_MEDIAENGINE_API std::int32_t audioOpticalSectorSize()noexcept;
RETROMEDIALAB_MEDIAENGINE_API std::int32_t opticalSubchannelSize()noexcept;
}
