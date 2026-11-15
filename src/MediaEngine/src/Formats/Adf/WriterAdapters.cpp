#include "WriterAdapters.h"
#include "AdfImages.h"
#include "Constants.h"
#include "../../Images/WriterAdapters.h"
#include "../../Primitives/Identifiers.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Adf {
namespace {
bool same(std::string_view a,std::string_view b){return Primitives::ordinalIdentifierEqualsIgnoreCase(a,b);}
bool acornTarget(std::string_view format,std::string_view ext){using namespace Constants;const auto profiles=acornGeometries();if(!std::any_of(profiles.begin(),profiles.end(),[&](const auto& p){return same(p.geometry.formatId,format);}))return false;
    if(same(format,AcornDosPlus800))return same(ext,ImgExtension)||same(ext,DosPlusExtension);
    return same(ext,AdfExtension)||(same(format,AcornAdfs640)&&(same(ext,AdlExtension)||same(ext,Adfs640Extension)))||(same(format,AcornAdfs160)&&same(ext,AdsExtension))||(same(format,AcornAdfs320)&&same(ext,AdmExtension));}
bool amigaTarget(std::string_view format,std::string_view ext){using namespace Constants;return (same(format,AmigaDos)||same(format,AmigaDosHighDensity))&&same(ext,AdfExtension);}
}
std::shared_ptr<const ImageWriter> acornWriterAdapter(){std::vector<std::string> formats;for(const auto& profile:acornGeometries())formats.push_back(profile.geometry.formatId);return sectorWriterAdapter(std::string(Constants::AcornWriterId),std::move(formats),{std::string(Constants::AdfExtension),std::string(Constants::AdsExtension),std::string(Constants::AdmExtension),std::string(Constants::AdlExtension),std::string(Constants::Adfs640Extension),std::string(Constants::ImgExtension),std::string(Constants::DosPlusExtension)},
    [](const auto& image,const auto&,auto target,auto){return same(image.geometry().formatId,target)&&canWrite(image);},
    [](const auto& image,const auto&,const std::filesystem::path& output,auto,std::stop_token token){write(image,output,token);return std::vector<std::filesystem::path>{output};},false,acornTarget);}
std::shared_ptr<const ImageWriter> amigaWriterAdapter(){std::vector<std::string> formats;for(const auto& profile:amigaGeometries())formats.push_back(profile.geometry.formatId);return sectorWriterAdapter(std::string(Constants::AmigaWriterId),std::move(formats),{std::string(Constants::AdfExtension)},
    [](const auto& image,const auto&,auto target,auto){return same(image.geometry().formatId,target);},
    [](const auto& image,const auto&,const std::filesystem::path& output,auto,std::stop_token token){write(image,output,token);return std::vector<std::filesystem::path>{output};},false,amigaTarget);}
}
