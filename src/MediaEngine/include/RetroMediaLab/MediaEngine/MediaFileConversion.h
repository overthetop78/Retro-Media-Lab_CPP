#pragma once
#include "Export.h"
#include <filesystem>
#include <future>
#include <stop_token>
#include <string>
#include <string_view>
#include <vector>
namespace RetroMediaLab::MediaEngine {
RETROMEDIALAB_MEDIAENGINE_API bool isSectorFileCatalogAliasTarget(std::string_view format,std::string_view extension);
RETROMEDIALAB_MEDIAENGINE_API bool canConvertSectorFileTarget(std::string_view format,std::string_view extension);
RETROMEDIALAB_MEDIAENGINE_API std::future<std::vector<std::filesystem::path>> convertSectorFileAsync(std::filesystem::path source,std::filesystem::path output,std::string targetFormat,std::stop_token={});
}
