#include <RetroMediaLab/Emulation/Firmware.h>
#include <RetroMediaLab/MediaEngine/MediaUtilities.h>
#include <QDir>
#include <QFileInfo>
#include <algorithm>
#include <filesystem>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace RetroMediaLab::Emulation {
namespace {
namespace fs = std::filesystem;

bool isReparsePoint(const fs::directory_entry& entry, std::error_code& error) {
#ifdef Q_OS_WIN
    const auto attributes = GetFileAttributesW(entry.path().c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        error = std::error_code(GetLastError(), std::system_category());
        return false;
    }
    return (attributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
#else
    return entry.is_symlink(error);
#endif
}
}

quint32 firmwareCrc32(QByteArrayView data) noexcept {
    return MediaEngine::computeMediaCrc32({reinterpret_cast<const std::uint8_t*>(data.data()),
        static_cast<std::size_t>(data.size())});
}

std::optional<FirmwareCandidate> selectFirmwareCandidate(const QList<FirmwareCandidate>& candidates,
    const QString& candidateId, const QString& destinationFieldId) {
    for (const bool byIdentity : {true, false}) {
        if (byIdentity ? candidateId.isNull() : destinationFieldId.isNull()) continue;
        const FirmwareCandidate* selected = nullptr;
        for (const auto& candidate : candidates) {
            if (byIdentity && candidate.id != candidateId) continue;
            if (!destinationFieldId.isNull() && candidate.destinationFieldId != destinationFieldId) continue;
            if (selected) return {};
            selected = &candidate;
        }
        if (selected) return *selected;
    }
    return candidates.size() == 1 ? std::optional{candidates.front()} : std::nullopt;
}

FirmwareScanResult scanFirmwareFiles(const QString& directory, FirmwareScanMode mode,
    const FirmwareInspector& inspect, std::stop_token token) {
    if (token.stop_requested()) return FirmwareScanResult::Cancelled;
    if (directory.trimmed().isEmpty() || !inspect) return FirmwareScanResult::DirectoryError;
    const auto root = QFileInfo(QDir(directory).absolutePath()).filesystemAbsoluteFilePath();
    std::error_code error;
    fs::create_directories(root, error);
    if (error) return FirmwareScanResult::DirectoryError;
    QStringList files;
    const bool recursive = mode == FirmwareScanMode::Recursive;
    for (fs::recursive_directory_iterator entry(root, error), end; !error && entry != end; entry.increment(error)) {
        if (token.stop_requested()) return FirmwareScanResult::Cancelled;
        const bool skip = recursive && isReparsePoint(*entry, error);
        if (!recursive || skip) entry.disable_recursion_pending();
        if (!skip && !error && entry->is_regular_file(error)) files.append(QFileInfo(entry->path()).absoluteFilePath());
    }
    if (error) return FirmwareScanResult::DirectoryError;
    std::ranges::sort(files, [](const QString& first, const QString& second) {
        const auto order = first.compare(second, Qt::CaseInsensitive);
        return order == 0 ? first < second : order < 0;
    });
    for (const auto& file : files) {
        if (token.stop_requested()) return FirmwareScanResult::Cancelled;
        inspect(file, token);
    }
    return token.stop_requested() ? FirmwareScanResult::Cancelled : FirmwareScanResult::Complete;
}
}
