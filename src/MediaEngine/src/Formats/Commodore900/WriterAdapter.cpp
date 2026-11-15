#include "WriterAdapter.h"
#include "Constants.h"
#include "CoherentImages.h"
#include "../../Images/WriterAdapters.h"
namespace RetroMediaLab::MediaEngine::Commodore900 {
std::shared_ptr<const ImageWriter> writerAdapter(){return sectorWriterAdapter(std::string(Constants::WriterId),{std::string(Constants::FormatId)},{std::string(Constants::BinExtension),std::string(Constants::ImgExtension)},
    [](const auto& image,const auto&,auto,auto){return canWrite(image);},
    [](const auto& image,const auto&,const std::filesystem::path& output,auto,std::stop_token token){write(image,output,token);return std::vector<std::filesystem::path>{output};});}
}
