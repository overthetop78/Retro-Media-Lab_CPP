#include "RawImages.h"
#include "Constants.h"
#include "../../Models/Sectors/LinearSectorWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Raw {
using namespace Constants;
namespace {
bool same(std::string_view a,std::string_view b){if(a.size()!=b.size())return false;for(std::size_t i=0;i<a.size();++i){const auto lower=[](char c){return c>=UpperFirst && c<=UpperLast?char(c+CaseDelta):c;};if(lower(a[i])!=lower(b[i]))return false;}return true;}
std::optional<Geometry> selected(std::string_view id,const std::vector<Geometry>& profiles){for(const auto& p:profiles)if(same(id,p.formatId))return p;return {};}
}
bool canWriteIbm(const MediaFileSystems::SectorImage& image,std::string_view id){if(!hasCompleteSectorPayload(image,image.geometry().blockSize))return false;if(same(id,IbmScan))return matchesSectorGeometry(image,image.geometry(),false);const auto target=selected(id,ibmGeometries());return target && matchesSectorGeometry(image,*target,false);}
bool canWriteMsx(const MediaFileSystems::SectorImage& image,std::string_view id){const auto target=selected(id,msxGeometries());return target && matchesSectorGeometry(image,*target,false) && hasCompleteSectorPayload(image,image.geometry().blockSize);}
void writeIbm(const MediaFileSystems::SectorImage& image,const std::filesystem::path& path,std::string_view id,std::stop_token token){
    checkCancellation(token);if(same(id,IbmScan)){auto target=image.geometry();target.formatId=id;writeLinearSectorImage(image,path,target,false,token);return;}
    const auto target=selected(id,ibmGeometries());if(!target)throw MediaError(DiagnosticCode::UnsupportedRawFormat,{std::string(id)});writeLinearSectorImage(image,path,*target,false,token);
}
void writeMsx(const MediaFileSystems::SectorImage& image,const std::filesystem::path& path,std::string_view id,std::stop_token token){checkCancellation(token);const auto target=selected(id,msxGeometries());if(!target)throw MediaError(DiagnosticCode::UnsupportedRawFormat,{std::string(id)});writeLinearSectorImage(image,path,*target,false,token);}
}
