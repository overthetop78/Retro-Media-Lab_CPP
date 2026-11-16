#include "Internal.h"
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <RetroMediaLab/MediaFileSystems/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::KryoFlux {
std::vector<TrackFile> findTrackFiles(const std::filesystem::path& path,std::stop_token token){
    using namespace Constants;const QFileInfo primary(QString::fromStdU16String(path.u16string()));
    const QRegularExpression pattern(QString::fromUtf8(FilePattern),QRegularExpression::CaseInsensitiveOption|QRegularExpression::UseUnicodePropertiesOption);
    const auto match=pattern.match(primary.fileName());if(!match.hasMatch())throw MediaError(DiagnosticCode::KryoFileName);
    if(!primary.absoluteDir().exists()||!QFileInfo(primary.absolutePath()).isReadable())throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::OpenFailed,{primary.absolutePath().toUtf8().toStdString()});
    const auto prefix=match.captured(PrefixGroup);std::vector<TrackFile> files;
    for(const auto& entry:primary.absoluteDir().entryInfoList(QDir::Files|QDir::NoDotAndDotDot,QDir::NoSort)){
        checkCancellation(token);const auto candidate=pattern.match(entry.fileName());if(!candidate.hasMatch()||candidate.captured(PrefixGroup).compare(prefix,Qt::CaseInsensitive)!=0)continue;
        bool cylinderValid=false,headValid=false;const auto cylinder=candidate.captured(CylinderGroup).toInt(&cylinderValid),head=candidate.captured(HeadGroup).toInt(&headValid);
        if(!cylinderValid||!headValid)throw MediaError(DiagnosticCode::KryoFileName);if(cylinder>MaximumTrack||head>MaximumHead)continue;
        files.push_back({std::filesystem::path(entry.absoluteFilePath().toStdU16String()),cylinder,head});
    }
    if(files.empty())throw MediaError(DiagnosticCode::KryoNoFiles);
    std::sort(files.begin(),files.end(),[](const auto& a,const auto& b){return std::pair(a.cylinder,a.head)<std::pair(b.cylinder,b.head);});return files;
}
}
