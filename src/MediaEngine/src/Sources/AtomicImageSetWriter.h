#pragma once
#include "AtomicImageWriter.h"
namespace RetroMediaLab::MediaEngine {
struct ImageFileOutput {
    std::filesystem::path path;
    ImageProducer produce;
    std::uint64_t preallocationSize=0;
};
RETROMEDIALAB_MEDIAENGINE_API void writeAtomicImageSet(std::span<const ImageFileOutput>,std::stop_token={},bool replaceExisting=true);
}
