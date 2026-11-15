#include "WriterAdapter.h"
#include "Constants.h"
#include "../../Images/WriterAdapters.h"
#include "../../Models/Sectors/LinearSectorWriter.h"
namespace RetroMediaLab::MediaEngine::AcornAtom {
namespace {
MediaFileSystems::SectorGeometry geometry(){return {std::string(Constants::FormatId),Constants::BlockSize,Constants::Cylinders,Constants::Heads,Constants::SectorsPerTrack};}
}
std::shared_ptr<const ImageWriter> writerAdapter(){return sectorWriterAdapter(std::string(Constants::WriterId),{std::string(Constants::FormatId)},{std::string(Constants::Extensions.front())},
    [](const auto& image,const auto&,auto,auto){return matchesSectorGeometry(image,geometry(),false);},
    [](const auto& image,const auto&,const std::filesystem::path& output,auto,std::stop_token token){writeLinearSectorImage(image,output,geometry(),false,token);return std::vector<std::filesystem::path>{output};});}
}
