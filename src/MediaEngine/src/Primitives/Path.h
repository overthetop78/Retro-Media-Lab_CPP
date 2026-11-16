#pragma once
#include <filesystem>
#include <string>
#include <string_view>
#include "IdentifierConstants.h"
namespace RetroMediaLab::MediaEngine::Primitives {
inline std::string pathExtension(const std::filesystem::path& path){const auto filename=path.filename().u8string();const auto point=filename.find_last_of(char8_t(IdentifierConstants::ExtensionSeparator));if(point==std::u8string::npos||point+1==filename.size())return {};return {filename.begin()+point,filename.end()};}
inline std::string pathExtensionUtf8(std::string_view path){return pathExtension(std::filesystem::path(std::u8string(path.begin(),path.end())));}
}
