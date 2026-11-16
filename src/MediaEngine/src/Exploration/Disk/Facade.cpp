#include "Projection.h"
#include "../../Composition/Runtime.h"
namespace RetroMediaLab::MediaEngine {
std::future<std::shared_ptr<const ExploredDisk>> exploreDiskAsync(std::filesystem::path path,std::optional<std::string> format,DiskExplorationProgressCallback progress,std::stop_token token){return std::async(std::launch::async,[services=RuntimeDetails::defaultServices(),path=std::move(path),format=std::move(format),progress=std::move(progress),token]{return DiskProjectionAccess::create(services->disks.explore(path,format,progress,token),token);});}
std::future<std::shared_ptr<const ExploredDisk>> exploreDiskAsync(std::shared_ptr<const MediaDocument> document,std::optional<std::string> format,DiskExplorationProgressCallback progress,std::stop_token token){return std::async(std::launch::async,[services=RuntimeDetails::defaultServices(),document=std::move(document),format=std::move(format),progress=std::move(progress),token]{return DiskProjectionAccess::create(services->disks.explore(document,format,progress,token),token);});}
}
