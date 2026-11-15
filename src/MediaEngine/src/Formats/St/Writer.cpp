#include "Writer.h"
#include "Geometry.h"
#include "Constants.h"
#include "../../Models/Sectors/LinearSectorWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::St {
void write(const MediaFileSystems::SectorImage& image,const std::filesystem::path& output,std::stop_token token){const auto target=geometry(image.geometry().formatId);if(!target)throw MediaError(DiagnosticCode::StWriterFormat,{image.geometry().formatId,std::to_string(Constants::SectorSize)});writeLinearSectorImage(image,output,*target,true,token);}
}
