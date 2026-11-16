#pragma once
#include "ImageFormat.h"
#include "MediaKind.h"
#include "GwFormatCapabilities.h"
#include <filesystem>
namespace RetroMediaLab::MediaEngine {
enum class DiskMediaCategory {Unknown,ThreeInch,ThreeHalfDd,ThreeHalfHd,FiveQuarterDd,FiveQuarterHd,EightInch};
struct DiskVisualizationClassification {std::optional<std::string> decoderId;DiskMediaCategory mediaCategory=DiskMediaCategory::Unknown;};
struct DiskProtection {std::string id,machine;std::vector<std::string> formatIds;std::string displayName;};
class DiskClassificationCatalog final {
public:
 RETROMEDIALAB_MEDIAENGINE_API explicit DiskClassificationCatalog(std::vector<DiskFormat>);
 RETROMEDIALAB_MEDIAENGINE_API static DiskClassificationCatalog createDefault();
 std::span<const DiskFormat> formats()const noexcept{return formats_;}
 std::span<const std::string> machines()const noexcept{return machines_;}
 std::span<const DiskProtection> protections()const noexcept{return protections_;}
 RETROMEDIALAB_MEDIAENGINE_API const DiskFormat* resolveFormat(std::optional<std::string_view>)const;
 RETROMEDIALAB_MEDIAENGINE_API std::vector<DiskFormat> formatsFor(std::optional<std::string_view> machine)const;
 RETROMEDIALAB_MEDIAENGINE_API std::vector<DiskProtection> protectionsFor(std::optional<std::string_view> machine,std::optional<std::string_view> format)const;
private:std::vector<DiskFormat> formats_;std::vector<std::string> machines_;std::vector<DiskProtection> protections_;
};
RETROMEDIALAB_MEDIAENGINE_API DiskVisualizationClassification classifyDiskVisualization(std::optional<std::string_view> machine,std::optional<std::string_view> formatId,std::optional<std::string_view> protectionId,bool automaticDetection);
RETROMEDIALAB_MEDIAENGINE_API std::string_view selectMediaIcon(std::optional<MediaKind>,const std::filesystem::path&,const DiskFormat* format=nullptr);
RETROMEDIALAB_MEDIAENGINE_API bool canConvertToScpForVisualization(const std::filesystem::path&,const DiskFormat* detectedFormat,const GwFormatCapabilities&);
}
