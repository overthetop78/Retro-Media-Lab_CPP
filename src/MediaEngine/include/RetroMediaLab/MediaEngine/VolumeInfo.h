#pragma once
#include "MessageInfo.h"
#include <cstdint>
#include <optional>
namespace RetroMediaLab::MediaEngine {
struct VolumeInfo {
    std::int64_t start=0,length=0;
    std::string origin;
    std::optional<std::string> partitionTable;
    std::optional<std::int32_t> partitionNumber,sessionNumber,trackNumber;
    std::optional<std::string> fileSystemId,partitionType,partitionId,name;
    std::optional<MessageInfo> nameRecipe;
};
}
