#pragma once
#include <string_view>
#include <cstdint>
namespace RetroMediaLab::MediaFileSystems::SequentialConstants {
inline constexpr std::string_view FileNameKey="fileName",BlockNumberKey="blockNumber",FileTypeKey="fileType";
inline constexpr char FirstDigit='0',LastDigit='9';
inline constexpr std::int64_t MaximumFileBytes=0x7fffffff,MaximumStorageReference=0x7fffffff;
}
