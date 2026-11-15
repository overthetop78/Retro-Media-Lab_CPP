#include "WriterAdapter.h"
#include "BbcDfsImages.h"
#include "Constants.h"
#include "../../Images/WriterAdapters.h"
#include "../../Primitives/Identifiers.h"
namespace RetroMediaLab::MediaEngine::BbcDfs {
std::shared_ptr<const ImageWriter> writerAdapter(){std::vector<std::string> formats;for(const auto& g:geometries())formats.push_back(g.formatId);return sectorWriterAdapter(std::string(Constants::WriterId),std::move(formats),{std::string(Constants::Ssd),std::string(Constants::Dsd)},
    [](const auto& image,const auto&,auto target,auto){return canWrite(image,target);},
    [](const auto& image,const auto&,const std::filesystem::path& output,auto target,std::stop_token token){write(image,output,target,token);return std::vector<std::filesystem::path>{output};},false,
    [](auto format,auto ext){const auto g=findGeometry(format);return g&&Primitives::ordinalIdentifierEqualsIgnoreCase(ext,g->heads==1?Constants::Ssd:Constants::Dsd);});}
}
