#include "Adapter.h"
namespace RetroMediaLab::MediaEngine::Scp {
std::map<std::string,std::string> metadata(const Image& image){
    const auto& h=image.header;
    return {{std::string(Constants::RawVersionKey),std::to_string(h.version)},
        {std::string(Constants::VersionTextKey),h.versionText()},
        {std::string(Constants::DiskTypeKey),std::to_string(h.diskType)},
        {std::string(Constants::FlagsKey),std::to_string(h.flags)},
        {std::string(Constants::WidthKey),std::to_string(h.bitCellEncoding)},
        {std::string(Constants::HeadsKey),std::to_string(h.heads)},
        {std::string(Constants::ResolutionKey),std::to_string(h.resolution)},
        {std::string(Constants::ResolutionNanosecondsKey),std::to_string(h.resolutionNanoseconds())},
        {std::string(Constants::ChecksumValidKey),std::string(image.checksumValid?Constants::TrueText:Constants::FalseText)}};
}
}
