#pragma once
#include "../../../Decoding/Sequential/Models.h"
namespace RetroMediaLab::MediaEngine::AcornTape {
RETROMEDIALAB_MEDIAENGINE_API bool canBuildBlock(const SequentialDecodedBlock&);
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> buildBlock(const SequentialDecodedBlock&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::vector<SequentialDecodedBlock> decodeBlocks(std::span<const std::uint8_t>,const std::vector<std::int64_t>& positions,std::string_view sourceKind,std::size_t firstPosition=0,std::span<const int> byteSamples={},std::stop_token={});
}
