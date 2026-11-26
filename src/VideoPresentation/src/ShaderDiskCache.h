#pragma once

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QStandardPaths>

namespace RetroMediaLab::VideoPresentation::ShaderDiskCache {
inline constexpr auto CacheDirectory = "ShaderCache/";
inline constexpr qsizetype MaximumBinarySize = 16 * 1024 * 1024;

inline QString shaderCacheDirectory(QLatin1StringView renderer) {
    const auto root = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    return root.isEmpty() ? QString{} : QDir(root).filePath(QLatin1String(CacheDirectory) + renderer);
}

inline QByteArray readShaderBinary(const QString& path) {
    if (path.isEmpty()) return {};
    QFile input(path);
    if (!input.open(QIODevice::ReadOnly)) return {};
    const qsizetype hashSize = QCryptographicHash::hashLength(QCryptographicHash::Sha256);
    const auto length = input.size() - hashSize;
    if (length <= 0 || length > MaximumBinarySize) return {};
    const auto digest = input.read(hashSize);
    const auto binary = input.read(length);
    if (binary.size() != length || !input.atEnd()) return {};
    return QCryptographicHash::hash(binary, QCryptographicHash::Sha256) == digest ? binary : QByteArray{};
}

inline void saveShaderBinary(const QString& directory, const QString& path, const QByteArray& binary) {
    if (path.isEmpty() || binary.isEmpty() || binary.size() > MaximumBinarySize || !QDir().mkpath(directory)) return;
    QSaveFile output(path);
    if (!output.open(QIODevice::WriteOnly)) return;
    const auto digest = QCryptographicHash::hash(binary, QCryptographicHash::Sha256);
    if (output.write(digest) == digest.size() && output.write(binary) == binary.size()) output.commit();
}

}
