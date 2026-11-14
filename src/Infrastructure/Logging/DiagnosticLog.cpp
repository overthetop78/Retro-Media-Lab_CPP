#include "DiagnosticLog.h"
#include "LogConstants.h"
#include <QDir>
#include <QFile>
#include <QSysInfo>
#include <QStringList>
#include <mutex>
namespace RetroMediaLab {
bool writeDiagnostic(const QString& directory,DiagnosticSeverity severity,const QString& detail,
    const QString& context,const QString& language,const QString& process,const QDateTime& time) noexcept {
    try {
        static std::mutex gate;const std::lock_guard lock(gate);
        const auto prefix=severity==DiagnosticSeverity::Warning?LogConstants::Warnings:
            severity==DiagnosticSeverity::Information?LogConstants::Information:LogConstants::Errors;
        const QStringList lines{LogConstants::Separator,QString(LogConstants::TimeField)+time.toString(Qt::ISODateWithMs),
            QString(LogConstants::ContextField)+context,QString(LogConstants::ApplicationField)+LogConstants::ProductDirectory+LogConstants::Space+RETROMEDIALAB_APP_VERSION,
            QString(LogConstants::RuntimeField)+QString::fromLatin1(qVersion()),QString(LogConstants::OSField)+QSysInfo::prettyProductName(),
            QString(LogConstants::CultureField)+language,QString(LogConstants::ProcessField)+process,detail,QString()};
        if(!QDir().mkpath(directory))return false;
        QFile file(QDir(directory).filePath(QString(prefix)+LogConstants::Dash+time.toString(LogConstants::DiagnosticDate)+LogConstants::LogExtension));
        const auto bytes=(lines.join(LogConstants::NewLine)+LogConstants::NewLine).toUtf8();
        return file.open(QIODevice::WriteOnly|QIODevice::Append)&&file.write(bytes)==bytes.size()&&file.flush();
    }catch(...){return false;}
}
}
