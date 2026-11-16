#pragma once
#include "Export.h"
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>
namespace RetroMediaLab::MediaEngine {
struct EnabledOption {std::string argument;std::optional<std::string> value;};
RETROMEDIALAB_MEDIAENGINE_API bool supportsBuiltInDiskDefinition(std::string_view formatId);
RETROMEDIALAB_MEDIAENGINE_API std::filesystem::path builtInDiskDefinitionPath(const std::filesystem::path& applicationDirectory);
// Prepares arguments only; the caller owns the external tool and its execution.
RETROMEDIALAB_MEDIAENGINE_API void appendBuiltInDiskDefinitionArguments(std::vector<std::string>& arguments,std::string_view formatId,std::span<const EnabledOption> options,const std::filesystem::path& applicationDirectory);
}
