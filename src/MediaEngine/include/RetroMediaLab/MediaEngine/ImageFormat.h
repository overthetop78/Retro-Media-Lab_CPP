#pragma once
#include "Export.h"
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>
namespace RetroMediaLab::MediaEngine {
enum class FloppyFormFactor {Unknown,ThreeInch,ThreeAndHalfInch,FiveAndQuarterInch,EightInch};
enum class FloppyDensity {Unknown,DoubleDensity,HighDensity,ExtendedDensity};
struct ImageExtension {std::string extension,displayNameKey;bool isDefault=false;std::optional<std::string> technicalDisplayName;};
struct DiskFormat {
    std::string id,family,familyNameKey,displayNameKey;
    std::vector<ImageExtension> extensions;
    bool isCommon=true;
    std::optional<std::vector<std::string>> compatibleSourceExtensions;
    std::optional<std::string> tag;
    FloppyFormFactor formFactor=FloppyFormFactor::Unknown;
    bool supportsPhysicalRead=true,supportsPhysicalWrite=true;
    FloppyDensity density=FloppyDensity::Unknown;
    std::optional<std::string> technicalDisplayName;
};
// Labels are keys in the MediaEngine translation domain, resolved by the caller's current language.
RETROMEDIALAB_MEDIAENGINE_API std::span<const DiskFormat> imageFormats();
RETROMEDIALAB_MEDIAENGINE_API const DiskFormat* findImageFormat(std::string_view id);
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::string> imageFormatIds();
RETROMEDIALAB_MEDIAENGINE_API std::vector<DiskFormat> compatibleImageFormats(std::span<const DiskFormat>,std::string_view sourceExtension);
RETROMEDIALAB_MEDIAENGINE_API std::vector<DiskFormat> compatibleImageFormats(std::string_view sourceExtension);
RETROMEDIALAB_MEDIAENGINE_API std::vector<DiskFormat> mergeRuntimeImageFormats(std::span<const DiskFormat> curated,std::span<const DiskFormat> runtime);
using FormatTextResolver=std::function<std::string(std::string_view)>;
RETROMEDIALAB_MEDIAENGINE_API std::string imageFormatName(const DiskFormat&,const FormatTextResolver&);
RETROMEDIALAB_MEDIAENGINE_API std::string imageFormatFamilyName(const DiskFormat&,const FormatTextResolver&);
RETROMEDIALAB_MEDIAENGINE_API std::string imageExtensionName(const ImageExtension&,const FormatTextResolver&);
}
