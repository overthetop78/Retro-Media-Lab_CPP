#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <filesystem>
#include <functional>
#include <span>
#include <cstdint>
#include <stop_token>
namespace RetroMediaLab::MediaEngine {
using ImageOutput=std::function<void(std::span<const std::uint8_t>)>;
using ImageProducer=std::function<void(const ImageOutput&,std::stop_token)>;
RETROMEDIALAB_MEDIAENGINE_API void writeAtomicImage(const std::filesystem::path&,const ImageProducer&,std::stop_token = {});
}
