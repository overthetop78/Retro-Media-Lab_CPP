#pragma once
#include "ScpHeader.h"
#include <filesystem>
#include <future>
#include <stop_token>
namespace RetroMediaLab::MediaEngine {
struct ScpCaptureInfo {
 Scp::Header header;
 std::int32_t capturedTracks=0,missingTracks=0,cylinders=0,sides=0;
 bool checksumValid=false;
 std::uint64_t fileSize=0;
};
RETROMEDIALAB_MEDIAENGINE_API std::future<ScpCaptureInfo> readScpCaptureInfoAsync(std::filesystem::path,std::stop_token={});
}
