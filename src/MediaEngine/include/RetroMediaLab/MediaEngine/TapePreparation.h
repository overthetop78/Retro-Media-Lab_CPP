#pragma once
#include "TapeSignal.h"
#include <filesystem>
#include <future>
#include <stop_token>
namespace RetroMediaLab::MediaEngine {
RETROMEDIALAB_MEDIAENGINE_API std::future<MediaTapeSignal> readTapeAsync(std::filesystem::path,std::stop_token={},std::optional<std::string> machineId={},std::map<std::string,std::string> parameters={});
RETROMEDIALAB_MEDIAENGINE_API std::future<void> writeTapeAsync(std::filesystem::path,MediaTapeSignal,std::stop_token={},std::map<std::string,std::string> parameters={});
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::string> tapeRecordingExtensions();
}
