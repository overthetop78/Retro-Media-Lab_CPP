#include "WriterAdapters.h"
#include "Writing.h"
#include "Raw.h"
#include "Constants.h"
#include "../FormatIds.h"
#include "../TwoImg/TwoImg.h"
#include "../DiskCopy/DiskCopy.h"
#include "../DiskCopy/Constants.h"
#include "../../Images/WriterAdapters.h"
#include "../../Models/Sectors/Representation.h"
#include "../../Primitives/Identifiers.h"
namespace RetroMediaLab::MediaEngine::Apple {
namespace {
bool same(std::string_view a,std::string_view b){return Primitives::ordinalIdentifierEqualsIgnoreCase(a,b);}
void diskCopy(const MediaDocument& document,const std::filesystem::path& output,std::stop_token token){const auto& representation=static_cast<const SectorRepresentation&>(document.representation());const auto image=representation.imageOwner();const auto restored=DiskCopy::restore(image,document.metadata());if(restored)DiskCopy::write(*image,output,*restored,token);else DiskCopy::write(image,output,token);}
std::string extension(const std::filesystem::path& path){const auto value=path.extension().u8string();return {value.begin(),value.end()};}
bool macTarget(std::string_view format,std::string_view ext){return (Primitives::ordinalIdentifierStartsWithIgnoreCase(format,Formats::AppleMacPrefix)||Primitives::ordinalIdentifierStartsWithIgnoreCase(format,Formats::MacPrefix))&&(same(ext,Constants::Img)||same(ext,Constants::Image)||same(ext,Constants::Dc42));}
bool lisaTarget(std::string_view format,std::string_view ext){return Primitives::ordinalIdentifierStartsWithIgnoreCase(format,Formats::AppleLisaPrefix)&&(same(ext,Constants::Image)||same(ext,Constants::Dc42));}
}
std::shared_ptr<const ImageWriter> sectorWriterAdapter(){return RetroMediaLab::MediaEngine::sectorWriterAdapter(std::string(Constants::SectorWriterId),{std::string(Formats::AppleIIAppleDos113),std::string(Formats::AppleIIAppleDos140),std::string(Formats::AppleIIProDos140),std::string(Formats::AppleIIProDos800),std::string(Formats::AppleIIISos)},{std::string(Constants::TwoMg),std::string(Constants::D13),std::string(Constants::Do),std::string(Constants::Dsk),std::string(Constants::Po)},
    [](const auto& image,const auto&,auto target,auto){return canWriteSector(image,target);},
    [](const auto& image,const auto&,const std::filesystem::path& output,auto target,std::stop_token token){validateSector(image,target);if(same(extension(output),Constants::TwoMg))TwoImg::write(image,output,target,token);else writeRaw(image,output,target,token);return std::vector<std::filesystem::path>{output};},false,sectorTarget);}
std::shared_ptr<const ImageWriter> nibbleWriterAdapter(){return RetroMediaLab::MediaEngine::sectorWriterAdapter(std::string(Constants::NibbleWriterId),{std::string(Formats::AppleIIAppleDos113),std::string(Formats::AppleIIAppleDos140),std::string(Formats::AppleIIProDos140)},{std::string(Constants::Nib),std::string(Constants::Woz)},
    [](const auto& image,const auto&,auto,auto ext){return nibbleTarget(image.geometry().formatId,ext);},
    [](const auto& image,const auto&,const std::filesystem::path& output,auto,std::stop_token token){DiskImageWriter{}.write(image,output,token);return std::vector<std::filesystem::path>{output};},false,nibbleTarget);}
std::shared_ptr<const ImageWriter> macintoshWriterAdapter(){return RetroMediaLab::MediaEngine::sectorWriterAdapter(std::string(Constants::MacintoshWriterId),{std::string(Formats::Mac400),std::string(Formats::Mac800),std::string(Formats::Mac1440)},{std::string(Constants::Img),std::string(Constants::Image),std::string(Constants::Dc42)},
    [](const auto& image,const auto&,auto,auto ext){return same(ext,Constants::Img)?hasMacintoshGeometry(image):DiskCopy::hasSupportedDataLayout(image);},
    [](const auto& image,const MediaDocument& document,const std::filesystem::path& output,auto,std::stop_token token){if(same(extension(output),Constants::Img))writeMacintosh(image,output,token);else diskCopy(document,output,token);return std::vector<std::filesystem::path>{output};},false,macTarget);}
std::shared_ptr<const ImageWriter> lisaWriterAdapter(){return RetroMediaLab::MediaEngine::sectorWriterAdapter(std::string(Constants::LisaWriterId),{std::string(Formats::AppleLisaOffice),std::string(DiskCopy::Constants::AppleLisaMacWorks)},{std::string(Constants::Image),std::string(Constants::Dc42)},
    [](const auto& image,const auto&,auto,auto){return DiskCopy::hasSupportedDataLayout(image);},
    [](const auto&,const MediaDocument& document,const std::filesystem::path& output,auto,std::stop_token token){diskCopy(document,output,token);return std::vector<std::filesystem::path>{output};},false,lisaTarget);}
}
