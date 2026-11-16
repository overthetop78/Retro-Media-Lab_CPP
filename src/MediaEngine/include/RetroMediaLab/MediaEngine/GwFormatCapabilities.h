#pragma once
#include "ImageFormat.h"
#include <filesystem>
#include <future>
#include <stop_token>
namespace RetroMediaLab::MediaEngine {
// Describes software formats announced by the supplied help text, not measured drive capabilities.
struct GwFormatCapabilities {
    std::vector<std::string> formatIds,imageExtensions;
    bool isKnown()const noexcept{return !formatIds.empty()||!imageExtensions.empty();}
};
// Implemented by the host infrastructure; MediaEngine only interprets the result.
class GwFormatCapabilityReader {
public:
    virtual ~GwFormatCapabilityReader()=default;
    virtual std::future<GwFormatCapabilities> readAsync(std::filesystem::path executablePath,std::stop_token={})const=0;
};
RETROMEDIALAB_MEDIAENGINE_API GwFormatCapabilities parseGwReadHelp(std::string_view output);
RETROMEDIALAB_MEDIAENGINE_API std::optional<std::string> gwFormatArgument(std::string_view catalogId);
RETROMEDIALAB_MEDIAENGINE_API std::vector<DiskFormat> capabilityAwareImageFormats(std::span<const DiskFormat> curated,const GwFormatCapabilities&);
}
