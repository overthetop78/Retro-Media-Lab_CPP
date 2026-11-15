#include "WriterAdapters.h"
#include "RawImages.h"
#include "Constants.h"
#include "../../Images/WriterAdapters.h"
namespace RetroMediaLab::MediaEngine::Raw {
std::shared_ptr<const ImageWriter> ibmWriterAdapter(){std::vector<std::string> formats;for(const auto& profile:ibmGeometries())formats.push_back(profile.formatId);formats.emplace_back(MediaFileSystems::Formats::IbmScan);return sectorWriterAdapter(std::string(Constants::IbmWriterId),std::move(formats),{std::string(Constants::Ima),std::string(Constants::Img)},
    [](const auto& image,const auto&,auto format,auto){return canWriteIbm(image,format);},
    [](const auto& image,const auto&,const std::filesystem::path& output,auto format,std::stop_token token){writeIbm(image,output,format,token);return std::vector<std::filesystem::path>{output};});}
std::shared_ptr<const ImageWriter> msxWriterAdapter(){std::vector<std::string> formats;for(const auto& profile:msxGeometries())formats.push_back(profile.formatId);return sectorWriterAdapter(std::string(Constants::MsxWriterId),std::move(formats),{std::string(Constants::Dsk)},
    [](const auto& image,const auto&,auto format,auto){return canWriteMsx(image,format);},
    [](const auto& image,const auto&,const std::filesystem::path& output,auto format,std::stop_token token){writeMsx(image,output,format,token);return std::vector<std::filesystem::path>{output};});}
}
