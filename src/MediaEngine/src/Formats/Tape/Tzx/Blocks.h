#pragma once
#include "Reader.h"
namespace RetroMediaLab::MediaEngine::Tzx {
struct BlockInfo {std::uint8_t id;std::int64_t offset,length;std::optional<std::int16_t> jump;std::vector<std::int16_t> calls;};
std::vector<BlockInfo> readBlocks(const RecognitionContext&,std::stop_token);
SequentialSegmentKind segmentKind(std::uint8_t);
}
