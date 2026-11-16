#pragma once
#include "../../../Decoding/Sequential/Models.h"
namespace RetroMediaLab::MediaEngine::AtomTap {
RETROMEDIALAB_MEDIAENGINE_API SequentialDecodeResult decode(const SequentialImage&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API SequentialDecodeResult decodeSerial(std::span<const std::uint8_t>,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API bool canEncodeFiles(std::span<const SequentialDecodedBlock>);
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> encode(std::span<const SequentialDecodedBlock>,std::stop_token={});
}
