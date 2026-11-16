#pragma once
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::ImageConstants {
inline constexpr std::string_view FileSystemsDomain="MediaFileSystems";
inline constexpr std::string_view EngineDomain="MediaEngine";
inline constexpr std::int32_t PageSize=65536;
inline constexpr double ExplicitFormatConfidence=1.0,ExtensionConfidence=0.5,AssociatedFileConfidence=0.55,SignatureConfidence=0.9,ProbeConfidence=0.8;
inline constexpr char ExtensionSeparator='.';
}
