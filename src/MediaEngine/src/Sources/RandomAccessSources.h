#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/Sources.h>
#include <filesystem>
namespace RetroMediaLab::MediaEngine {
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::RandomAccessData> memorySource(MediaFileSystems::Bytes);
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::RandomAccessData> fileSource(const std::filesystem::path&);
}
