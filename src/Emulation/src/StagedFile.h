#pragma once

#include <QFile>
#include <QString>
#include <filesystem>
#include <system_error>

namespace RetroMediaLab::Emulation {

// The staged file and destination are on the same filesystem.
inline std::error_code replaceStagedFile(const QString& source, const QString& destination) {
    std::error_code error;
#ifdef Q_OS_WIN
    std::filesystem::rename(source.toStdWString(), destination.toStdWString(), error);
#else
    std::filesystem::rename(QFile::encodeName(source).toStdString(), QFile::encodeName(destination).toStdString(), error);
#endif
    return error;
}

}
