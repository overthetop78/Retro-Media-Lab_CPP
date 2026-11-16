#include "Service.h"
#include "../../Composition/Runtime.h"
namespace RetroMediaLab::MediaEngine {
std::future<ExploredMediaImage> exploreMediaAsync(std::filesystem::path path,std::optional<std::string> format,MediaExplorationProgressCallback progress,std::stop_token token){return std::async(std::launch::async,[services=RuntimeDetails::defaultServices(),path=std::move(path),format=std::move(format),progress=std::move(progress),token]{return services->exploration.explore(path,format,progress,token);});}
std::future<MediaOpeningAnalysisResult> analyzeMediaOpeningAsync(std::filesystem::path path,std::optional<std::string> format,DiskExplorationProgressCallback diskProgress,MediaExplorationProgressCallback progress,std::stop_token token,bool includeFileSystems,DiskCaptureLoadedCallback loaded){return std::async(std::launch::async,[services=RuntimeDetails::defaultServices(),path=std::move(path),format=std::move(format),diskProgress=std::move(diskProgress),progress=std::move(progress),token,includeFileSystems,loaded=std::move(loaded)]{return services->exploration.analyze(path,format,diskProgress,progress,token,includeFileSystems,loaded);});}
}
