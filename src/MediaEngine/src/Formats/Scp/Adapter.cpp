#include "Adapter.h"
#include "ProtectedImage.h"
#include "Writer.h"
#include "../Definitions/DiskDefinitions.h"
#include "../../Images/WriterAdapters.h"
namespace RetroMediaLab::MediaEngine::Scp {
std::shared_ptr<const ImageWriter> writerAdapter(){
    auto formats=Definitions::bitcellFormatIds();formats.emplace_back(Constants::FormatId);
    return fluxWriterAdapter(std::string(Constants::WriterId),std::move(formats),{std::string(Constants::Extension)},
        [](const auto&,auto,auto){return true;},
        [](const auto& image,const auto& metadata,const std::filesystem::path& path,auto,std::stop_token token){write(fromProtected(image,metadata,token),path,token);return std::vector<std::filesystem::path>{path};});
}
}
