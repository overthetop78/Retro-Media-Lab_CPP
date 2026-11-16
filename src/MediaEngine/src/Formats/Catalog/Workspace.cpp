#include <RetroMediaLab/MediaEngine/ImageFormatWorkspace.h>
#include "Constants.h"
#include "../Definitions/DiskDefinitions.h"
namespace RetroMediaLab::MediaEngine {
namespace {
using Ids=Definitions::Identifiers;
std::vector<std::string> unique(const std::vector<std::string>& source){const Ids ids(source.begin(),source.end());return {ids.begin(),ids.end()};}
}
std::shared_ptr<const ImageFormatWorkspaceSnapshot> ImageFormatWorkspace::build(GwFormatCapabilities capabilities){
 capabilities.formatIds=unique(capabilities.formatIds);capabilities.imageExtensions=unique(capabilities.imageExtensions);
 const auto curated=imageFormats();Ids ids(capabilities.formatIds.begin(),capabilities.formatIds.end()),extensions(capabilities.imageExtensions.begin(),capabilities.imageExtensions.end());
 for(const auto& [id,definition]:Definitions::embedded()){(void)definition;ids.insert(id);}
 for(const auto& format:curated)if(format.supportsPhysicalRead||format.supportsPhysicalWrite)ids.insert(gwFormatArgument(format.id).value_or(format.id));
 for(const auto extension:CatalogConstants::WorkspaceExtensions)extensions.insert(std::string(extension));
 auto formats=capabilityAwareImageFormats(curated,{{ids.begin(),ids.end()},{extensions.begin(),extensions.end()}});
 return std::shared_ptr<const ImageFormatWorkspaceSnapshot>(new ImageFormatWorkspaceSnapshot(std::move(capabilities),std::move(formats)));
}
ImageFormatWorkspace::ImageFormatWorkspace():state_(build({})){}
void ImageFormatWorkspace::setCapabilities(GwFormatCapabilities capabilities){auto next=build(std::move(capabilities));state_=std::move(next);}
void ImageFormatWorkspace::addDiskDefinitions(const std::filesystem::path& path){
 const auto discovered=Definitions::readFileIds(path);auto capabilities=state_->capabilities();Ids ids(capabilities.formatIds.begin(),capabilities.formatIds.end());ids.insert(discovered.begin(),discovered.end());capabilities.formatIds.assign(ids.begin(),ids.end());
 if(capabilities.imageExtensions.empty())for(const auto extension:CatalogConstants::FallbackExtensions)capabilities.imageExtensions.emplace_back(extension);
 auto next=build(std::move(capabilities));state_=std::move(next);
}
void ImageFormatWorkspace::rebuild(){auto next=build(state_->capabilities());state_=std::move(next);}
}
