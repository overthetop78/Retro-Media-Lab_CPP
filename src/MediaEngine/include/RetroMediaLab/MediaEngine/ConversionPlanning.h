#pragma once
#include "ImageFormat.h"
#include "ConversionConstants.h"
#include <chrono>
#include <filesystem>
namespace RetroMediaLab::MediaEngine {
enum class ConversionFidelityLevel {SectorData,ReconstructedTracks,PreservedFlux};
RETROMEDIALAB_MEDIAENGINE_API ConversionFidelityLevel conversionFidelity(std::string_view sourceExtension,std::string_view targetExtension);
RETROMEDIALAB_MEDIAENGINE_API ConversionFidelityLevel rebuiltConversionFidelity(std::string_view extension);
struct ConversionSelection {std::string formatId;std::vector<std::string> explicitExtensions;};
struct ConversionOutput {
 RETROMEDIALAB_MEDIAENGINE_API ConversionOutput(std::string format,std::string extension,std::filesystem::path,bool implicit,std::optional<ConversionFidelityLevel> = {});
 std::string formatId,extension;std::filesystem::path outputPath;bool usesImplicitExtension;
 ConversionFidelityLevel fidelity;
 bool preservesOriginalProtection()const noexcept{return fidelity==ConversionFidelityLevel::PreservedFlux;}
};
RETROMEDIALAB_MEDIAENGINE_API std::string formatConversionTag(std::string_view pattern,const DiskFormat&,std::string_view extension,std::string_view sourceName,std::chrono::system_clock::time_point timestamp);
class ConversionPlanner final {
public:
 RETROMEDIALAB_MEDIAENGINE_API ConversionPlanner();
 RETROMEDIALAB_MEDIAENGINE_API explicit ConversionPlanner(std::vector<DiskFormat>);
 RETROMEDIALAB_MEDIAENGINE_API std::vector<ConversionOutput> plan(const std::filesystem::path&,std::string outputBaseName,std::span<const ConversionSelection>,bool addTags,std::string_view pattern=ConversionPlanningConstants::DefaultTagPattern)const;
 RETROMEDIALAB_MEDIAENGINE_API std::vector<DiskFormat> outputsForSource(std::string_view sourceExtension,std::optional<std::string_view> detectedFormat={},bool requiresUserChoice=false)const;
private:std::vector<DiskFormat> formats_;
};
}
