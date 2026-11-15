#pragma once
#include <string_view>
namespace RetroMediaLab::MediaFileSystems::MetadataNumbers::Values {
inline constexpr char Null='\0',FirstDigit='0',LastDigit='9',Plus='+',Minus='-';
inline constexpr std::string_view Whitespace=" \t\n\r\v\f";
}
