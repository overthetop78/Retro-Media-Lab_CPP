#pragma once
#include "Export.h"
#include <filesystem>
#include <future>
#include <memory>
#include <stop_token>
namespace RetroMediaLab::MediaEngine {
class MediaDocument;
RETROMEDIALAB_MEDIAENGINE_API std::future<std::shared_ptr<const MediaDocument>> reconstructI86fSectorsAsync(std::filesystem::path,std::stop_token={});
}
