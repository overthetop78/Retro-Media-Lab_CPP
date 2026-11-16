#pragma once
#include "ImageFormatDetection.h"
#include "GwFormatCapabilities.h"
#include <memory>
namespace RetroMediaLab::MediaEngine {
class ImageFormatWorkspaceSnapshot final {
public:
 const GwFormatCapabilities& capabilities()const noexcept{return capabilities_;}
 std::span<const DiskFormat> formats()const noexcept{return detector_.formats();}
 const ImageFormatDetector& detector()const noexcept{return detector_;}
private:
 friend class ImageFormatWorkspace;
 ImageFormatWorkspaceSnapshot(GwFormatCapabilities capabilities,std::vector<DiskFormat> formats):capabilities_(std::move(capabilities)),detector_(std::move(formats)){}
 GwFormatCapabilities capabilities_;ImageFormatDetector detector_;
};
class ImageFormatWorkspace final {
public:
 RETROMEDIALAB_MEDIAENGINE_API ImageFormatWorkspace();
 std::shared_ptr<const ImageFormatWorkspaceSnapshot> snapshot()const noexcept{return state_;}
 RETROMEDIALAB_MEDIAENGINE_API void setCapabilities(GwFormatCapabilities);
 RETROMEDIALAB_MEDIAENGINE_API void addDiskDefinitions(const std::filesystem::path&);
 RETROMEDIALAB_MEDIAENGINE_API void rebuild();
private:
 static std::shared_ptr<const ImageFormatWorkspaceSnapshot> build(GwFormatCapabilities);
 std::shared_ptr<const ImageFormatWorkspaceSnapshot> state_;
};
}
