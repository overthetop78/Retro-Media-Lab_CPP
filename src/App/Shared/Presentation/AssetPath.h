#pragma once

#include <QCoreApplication>
#include <QDir>

namespace RetroMediaLab {
inline QString assetPath(const QString& relativePath) {
    constexpr auto AssetsDirectory = "Assets";
    const QDir executableDirectory(QCoreApplication::applicationDirPath());
    return QDir(executableDirectory.filePath(QLatin1String(AssetsDirectory))).filePath(relativePath);
}
}
