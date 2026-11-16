#pragma once
#include "Constants.h"
#include "../../../Images/RecognitionContext.h"
namespace RetroMediaLab::MediaEngine::WiiU {
struct Header {std::uint32_t chunkBytes=0,indexCount=0;std::uint64_t logicalBytes=0,arrayOffset=0,storedCount=0;};
RETROMEDIALAB_MEDIAENGINE_API bool wudLength(std::uint64_t)noexcept;
RETROMEDIALAB_MEDIAENGINE_API std::optional<Header> parseHeader(std::span<const std::uint8_t>,std::uint64_t);
RETROMEDIALAB_MEDIAENGINE_API std::optional<std::vector<std::uint32_t>> readIndex(const RecognitionContext&,const Header&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::RandomAccessData> wuxSource(std::shared_ptr<const MediaFileSystems::RandomAccessData>,Header,std::vector<std::uint32_t>);
}
