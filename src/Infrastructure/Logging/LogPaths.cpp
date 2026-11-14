#include "LogPaths.h"
#include "LogConstants.h"
#include <RetroMediaLab/Platform/DataDirectory.h>
#include <QDir>
#include <QFileInfo>
namespace RetroMediaLab {
bool ensureLogDirectory(const QString& path){return QDir().mkpath(path);}
QString logsDirectory(const QString& applicationDirectory) {
    if(QFileInfo(QDir(applicationDirectory).filePath(LogConstants::PortableFlag)).isFile())
        return QDir(QDir(applicationDirectory).filePath(LogConstants::PortableData)).filePath(LogConstants::LogsDirectory);
    const auto base=QString::fromStdU16String(Platform::userDataDirectory().u16string());
    return QDir(QDir(base).filePath(LogConstants::ProductDirectory)).filePath(LogConstants::LogsDirectory);
}
}
