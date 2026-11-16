#include "Internal.h"
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <RetroMediaLab/MediaFileSystems/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::HxcStream {
std::vector<TrackFile> findTrackFiles(const std::filesystem::path& path) {
    using namespace Constants;const auto name=QString::fromStdU16String(path.u16string());const QFileInfo primary(name);
    const QRegularExpression pattern(QString::fromUtf8(FilePattern.data(),qsizetype(FilePattern.size())),QRegularExpression::CaseInsensitiveOption|QRegularExpression::UseUnicodePropertiesOption);
    const auto match=pattern.match(primary.fileName());if(!match.hasMatch())throw MediaError(DiagnosticCode::HxcFileName);
    const auto prefix=match.captured(PrefixGroup);std::vector<TrackFile> files;
    if(!primary.absoluteDir().exists()||!QFileInfo(primary.absolutePath()).isReadable())throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::OpenFailed,{primary.absolutePath().toUtf8().toStdString()});
    const auto entries=primary.absoluteDir().entryInfoList(QDir::Files|QDir::NoDotAndDotDot,QDir::NoSort);
    for(const auto& entry:entries){const auto candidate=pattern.match(entry.fileName());if(!candidate.hasMatch()||candidate.captured(PrefixGroup).compare(prefix,Qt::CaseInsensitive)!=0)continue;
        bool cylinderValid=false,headValid=false;const auto cylinder=candidate.captured(CylinderGroup).toInt(&cylinderValid),head=candidate.captured(HeadGroup).toInt(&headValid);if(!cylinderValid||!headValid)throw MediaError(DiagnosticCode::HxcFileName);if(cylinder>std::int32_t(MaximumTrack)||head>std::int32_t(MaximumHead))continue;
        files.push_back({std::filesystem::path(entry.absoluteFilePath().toStdU16String()),cylinder,head});
    }
    if(files.empty())throw MediaError(DiagnosticCode::HxcNoFiles);
    std::sort(files.begin(),files.end(),[](const auto& a,const auto& b){return std::pair(a.cylinder,a.head)<std::pair(b.cylinder,b.head);});return files;
}
}
