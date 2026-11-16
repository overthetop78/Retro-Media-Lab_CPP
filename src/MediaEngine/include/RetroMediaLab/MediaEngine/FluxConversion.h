#pragma once
#include "Export.h"
#include <filesystem>
#include <future>
#include <stop_token>
#include <string_view>
namespace RetroMediaLab::MediaEngine {
RETROMEDIALAB_MEDIAENGINE_API bool canConvertFluxContainer(const std::filesystem::path& source,std::string_view format,std::string_view targetExtension);
RETROMEDIALAB_MEDIAENGINE_API std::future<void> convertFluxContainerAsync(std::filesystem::path source,std::filesystem::path output,std::stop_token={});
}
