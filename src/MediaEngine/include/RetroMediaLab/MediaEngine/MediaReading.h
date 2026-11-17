#pragma once
#include "Export.h"
#include <filesystem>
#include <future>
#include <map>
#include <memory>
#include <optional>
#include <stop_token>
#include <string>
namespace RetroMediaLab::MediaEngine {
class MediaDocument;
struct TapePcmProfile {int sampleRate{},channels{},bitsPerSample{};bool signedSamples{},littleEndian{};};
RETROMEDIALAB_MEDIAENGINE_API TapePcmProfile readTapePcmProfile(const std::map<std::string,std::string>& parameters={});
RETROMEDIALAB_MEDIAENGINE_API std::map<std::string,std::string> tapePcmParameters(const TapePcmProfile&);
RETROMEDIALAB_MEDIAENGINE_API std::future<std::shared_ptr<const MediaDocument>> readMediaDocumentAsync(std::filesystem::path,std::optional<std::string> format={},std::map<std::string,std::string> parameters={},std::stop_token={});
}
