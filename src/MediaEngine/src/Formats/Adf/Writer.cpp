#include "AdfImages.h"
#include "../../Models/Sectors/LinearSectorWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Adf {
bool canWrite(const MediaFileSystems::SectorImage& image){const auto profile=findGeometry(image.geometry().formatId);return profile&&matchesSectorGeometry(image,profile->geometry)&&hasCompleteSectorPayload(image,profile->geometry.blockSize);}
void write(const MediaFileSystems::SectorImage& image,const std::filesystem::path& path,std::stop_token token){
    checkCancellation(token);const auto profile=findGeometry(image.geometry().formatId);
    if(!profile)throw MediaError(DiagnosticCode::AdfUnsupportedFormat,{image.geometry().formatId});
    writeLinearSectorImage(image,path,profile->geometry,true,token);
}
}
