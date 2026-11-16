#pragma once
#include "MediaConversion.h"
namespace RetroMediaLab::MediaEngine {
RETROMEDIALAB_MEDIAENGINE_API std::future<void> reinterpretFat12Async(std::shared_ptr<const MediaDocument>,std::filesystem::path output,std::string target,bool sourceIsHybrid=false,std::stop_token={});
}
