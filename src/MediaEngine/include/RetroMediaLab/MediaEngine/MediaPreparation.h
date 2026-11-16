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
RETROMEDIALAB_MEDIAENGINE_API std::future<void> createBlankDiskAsync(std::filesystem::path,std::string formatId,std::stop_token={});
// HDD creation never overwrites an existing target or descriptor.
RETROMEDIALAB_MEDIAENGINE_API std::future<void> createHardDiskAsync(std::filesystem::path,std::string formatId,HardDiskGeometry,std::optional<std::string> fileSystemId={},std::stop_token={},bool preallocate=false);
RETROMEDIALAB_MEDIAENGINE_API const std::map<std::string,std::string>& mediaFileExtensions();
RETROMEDIALAB_MEDIAENGINE_API std::int32_t rawOpticalSectorSize()noexcept;
RETROMEDIALAB_MEDIAENGINE_API std::int32_t audioOpticalSectorSize()noexcept;
RETROMEDIALAB_MEDIAENGINE_API std::int32_t opticalSubchannelSize()noexcept;
}
