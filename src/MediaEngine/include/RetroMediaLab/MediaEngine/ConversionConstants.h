#pragma once
#include <array>
#include <string_view>
namespace RetroMediaLab::MediaEngine::ConversionPlanningConstants {
inline constexpr std::string_view DefaultTagPattern="[{FAMILY}-{FORMAT}] ",Tag="{TAG}",Name="{NAME}",Family="{FAMILY}",Format="{FORMAT}",Extension="{EXTENSION}",DatePrefix="{DATE:",TimePrefix="{TIME:";
inline constexpr std::array SupportedTokens{Tag,Name,Family,Format,Extension,DatePrefix,TimePrefix};
inline constexpr std::array<std::string_view,6> DateTimeTokens{"{DATE:YYYY-MM-DD}","{DATE:YYYYMMDD}","{DATE:DD-MM-YYYY}","{TIME:HH-MM-SS}","{TIME:HHMMSS}","{TIME:HH-MM}"};
inline constexpr std::array<std::string_view,6> DateTimePatterns{"yyyy-MM-dd","yyyyMMdd","dd-MM-yyyy","HH-mm-ss","HHmmss","HH-mm"};
inline constexpr char IdSeparator='.',TagSeparator='-';
inline constexpr double StartPercent=0,CompletePercent=100;
}
