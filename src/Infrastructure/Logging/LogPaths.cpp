#include "LogPaths.h"
#include "LogConstants.h"
#include <RetroMediaLab/Platform/DataDirectory.h>
#include <QDir>
#include <QFileInfo>
namespace RetroMediaLab {
bool ensureLogDirectory(const QString& path){return QDir().mkpath(path);}
QString applicationDataDirectory(const QString& applicationDirectory) {
    if(QFileInfo(QDir(applicationDirectory).filePath(LogConstants::PortableFlag)).isFile())
        return QDir(applicationDirectory).filePath(LogConstants::PortableData);
    const auto base=QString::fromStdU16String(Platform::userDataDirectory().u16string());
    return QDir(base).filePath(LogConstants::ProductDirectory);
}
QString logsDirectory(const QString& applicationDirectory) {
    return QDir(applicationDataDirectory(applicationDirectory)).filePath(LogConstants::LogsDirectory);
}
}
