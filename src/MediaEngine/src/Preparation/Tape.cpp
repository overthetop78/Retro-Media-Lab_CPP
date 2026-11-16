#include <RetroMediaLab/MediaEngine/TapePreparation.h>
#include "../Formats/Tape/Common/Signal.h"
namespace RetroMediaLab::MediaEngine {
std::future<MediaTapeSignal> readTapeAsync(std::filesystem::path path,std::stop_token token,std::optional<std::string> machine,std::map<std::string,std::string> parameters){return std::async(std::launch::async,[path=std::move(path),token,machine=std::move(machine),parameters=std::move(parameters)]{return TapeSignal::read(path,machine,parameters,token);});}
std::future<void> writeTapeAsync(std::filesystem::path path,MediaTapeSignal signal,std::stop_token token,std::map<std::string,std::string> parameters){return std::async(std::launch::async,[path=std::move(path),signal=std::move(signal),token,parameters=std::move(parameters)]{TapeSignal::write(path,signal,parameters,token);});}
std::vector<std::string> tapeRecordingExtensions(){const auto extensions=TapeSignal::writableExtensions();return {extensions.begin(),extensions.end()};}
}
