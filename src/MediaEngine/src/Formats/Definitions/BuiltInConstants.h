#pragma once
#include <string_view>
#include <cstddef>
namespace RetroMediaLab::MediaEngine::BuiltInDefinitionConstants {
inline constexpr std::size_t ArgumentCount=2;
inline constexpr std::string_view Argument="--diskdefs",AssetsDirectory="Assets",DefinitionsDirectory="DiskDefinitions",FileName="built-in.cfg";
}
