#pragma once
#include "ImageFormat.h"
#include "MessageInfo.h"
#include <filesystem>
#include <stop_token>
namespace RetroMediaLab::MediaEngine {
enum class FormatConfidence {Certain,Inferred,Ambiguous,Manual};
struct DetectedImageFormat {
 std::string extension;std::optional<DiskFormat> format;FormatConfidence confidence=FormatConfidence::Ambiguous;std::vector<DiskFormat> candidates;MessageInfo explanation;
 bool requiresUserChoice()const noexcept{return confidence==FormatConfidence::Ambiguous||!format;}
};
class ImageFormatDetector final {
public:
 RETROMEDIALAB_MEDIAENGINE_API ImageFormatDetector();
 RETROMEDIALAB_MEDIAENGINE_API explicit ImageFormatDetector(std::vector<DiskFormat>);
 RETROMEDIALAB_MEDIAENGINE_API DetectedImageFormat detect(const std::filesystem::path&,std::optional<std::int64_t> knownLength={},std::stop_token={})const;
private:std::vector<DiskFormat> formats_;
};
}
