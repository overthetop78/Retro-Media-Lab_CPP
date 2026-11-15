#include "BbcDfsImages.h"
#include "Constants.h"
#include "../../Models/Sectors/LinearSectorWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QString>
namespace RetroMediaLab::MediaEngine::BbcDfs {
bool canWrite(const MediaFileSystems::SectorImage& image,std::string_view id){const auto g=findGeometry(id);return g && matchesSectorGeometry(image,*g,false) && hasCompleteSectorPayload(image,Constants::SectorSize);}
void write(const MediaFileSystems::SectorImage& image,const std::filesystem::path& path,std::string_view id,std::stop_token token){
    checkCancellation(token);const auto g=findGeometry(id);const auto extension=path.extension().u8string();const auto text=QString::fromUtf8(reinterpret_cast<const char*>(extension.data()),qsizetype(extension.size()));
    const auto expected=g && g->heads==1?Constants::Ssd:Constants::Dsd;
    if(!g || text.compare(QString::fromUtf8(expected.data(),qsizetype(expected.size())),Qt::CaseInsensitive)!=0)throw MediaError(DiagnosticCode::DfsUnsupportedTarget,{std::string(id),text.toUtf8().toStdString()});
    writeLinearSectorImage(image,path,*g,false,token);
}
}
