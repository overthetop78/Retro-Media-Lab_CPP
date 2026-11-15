#include "UcsdWriter.h"
#include "Constants.h"
#include "../TeleDisk/Constants.h"
#include "../TeleDisk/Writer.h"
#include "../../Images/WriterAdapters.h"
#include "../../Models/Sectors/LinearSectorWriter.h"
#include "../../Primitives/Identifiers.h"
namespace RetroMediaLab::MediaEngine::Raw {
namespace {
MediaFileSystems::SectorGeometry ucsd(){return {std::string(MediaFileSystems::Formats::UcsdIbmMfm),Constants::SectorSize,Constants::UcsdCylinders,Constants::UcsdHeads,Constants::UcsdSectors};}
}
std::shared_ptr<const ImageWriter> ucsdWriterAdapter(){return sectorWriterAdapter(std::string(Constants::UcsdWriterId),{std::string(MediaFileSystems::Formats::UcsdIbmMfm)},{std::string(Constants::Img),std::string(TeleDisk::Constants::Extension)},
    [](const auto& image,const auto&,auto,auto extension){return Primitives::ordinalIdentifierEqualsIgnoreCase(extension,TeleDisk::Constants::Extension)?TeleDisk::hasRepresentablePayload(image):matchesSectorGeometry(image,ucsd(),false);},
    [](const auto& image,const auto&,const std::filesystem::path& output,auto,std::stop_token token){const auto u8=output.extension().u8string();const std::string extension(u8.begin(),u8.end());if(Primitives::ordinalIdentifierEqualsIgnoreCase(extension,TeleDisk::Constants::Extension))TeleDisk::write(image,output,token);else writeLinearSectorImage(image,output,ucsd(),false,token);return std::vector<std::filesystem::path>{output};});}
}
