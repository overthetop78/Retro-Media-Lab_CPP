#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/Sources.h>
#include <vector>
namespace RetroMediaLab::MediaEngine::Chd {
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::RandomAccessData> decodedSource(std::shared_ptr<const MediaFileSystems::RandomAccessData>,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::RandomAccessData> uncompressedSource(std::shared_ptr<const MediaFileSystems::RandomAccessData>,std::uint64_t logicalBytes,std::uint32_t hunkBytes,std::vector<std::uint32_t> storedHunks);
}
