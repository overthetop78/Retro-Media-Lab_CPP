#include <RetroMediaLab/Emulation/ConfigurationFiles.h>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <algorithm>
#include <filesystem>
#ifdef Q_OS_WIN
#include <QCryptographicHash>
#include <windows.h>
#endif

namespace RetroMediaLab::Emulation::ConfigurationFiles {
namespace {
constexpr auto ParentDirectory = "..";
constexpr auto ParentPrefix = "../";
constexpr auto DotDirectory = ".";

QString absolutePath(const QString& path) {
    return QDir::cleanPath(QFileInfo(path).absoluteFilePath());
}

class DocumentLock final {
public:
    DocumentLock(const QString& path, std::stop_token token) {
        if (token.stop_requested()) return;
#ifdef Q_OS_WIN
        constexpr auto Prefix = "Local\\RetroMediaLab.ConfigurationFile.";
        constexpr DWORD PollMilliseconds = 25;
        auto normalized = QDir::toNativeSeparators(path);
        for (auto& character : normalized) character = character.toUpper();
        const auto hash = QCryptographicHash::hash(normalized.toUtf8(), QCryptographicHash::Sha256).toHex().toUpper();
        const auto name = QLatin1String(Prefix) + QString::fromLatin1(hash);
        handle_ = CreateMutexW(nullptr, FALSE, reinterpret_cast<LPCWSTR>(name.utf16()));
        if (!handle_) return;
        while (!token.stop_requested()) {
            const DWORD result = WaitForSingleObject(handle_, PollMilliseconds);
            if (result != WAIT_TIMEOUT) {
                locked_ = result == WAIT_OBJECT_0 || result == WAIT_ABANDONED;
                break;
            }
        }
#else
        Q_UNUSED(path);
        locked_ = true;
#endif
    }
    ~DocumentLock() {
#ifdef Q_OS_WIN
        if (locked_) ReleaseMutex(handle_);
        if (handle_) CloseHandle(handle_);
#endif
    }
    DocumentLock(const DocumentLock&) = delete;
    DocumentLock& operator=(const DocumentLock&) = delete;
    bool locked() const { return locked_; }
private:
    bool locked_ = false;
#ifdef Q_OS_WIN
    HANDLE handle_ = nullptr;
#endif
};
}

QJsonValue MachineDocument::value(QLatin1StringView name) const {
    const auto exact = fields.constFind(name);
    if (exact != fields.cend()) return exact.value();
    for (auto field = fields.cbegin(); field != fields.cend(); ++field)
        if (QStringView(field.key()).compare(name, Qt::CaseInsensitive) == 0) return field.value();
    return QJsonValue(QJsonValue::Undefined);
}

std::optional<MachineDocument> decodeMachineDocument(const QJsonDocument& json, int schemaVersion) {
    if (!json.isObject()) return std::nullopt;
    MachineDocument document{{}, json.object()};
    const auto versionField = document.value(MachineDocumentKeys::SchemaVersion);
    bool valid = false;
    const int recordedVersion = versionField.isString()
        ? versionField.toString().toInt(&valid) : versionField.toInt();
    if (versionField.isDouble()) valid = versionField.toDouble() == recordedVersion;
    if (!valid || recordedVersion != schemaVersion) return std::nullopt;
    const auto recordedId = document.value(MachineDocumentKeys::Id).toString();
    document.id = QUuid::fromString(recordedId);
    if (document.id.isNull() || recordedId.compare(document.id.toString(QUuid::WithoutBraces), Qt::CaseInsensitive) != 0)
        return std::nullopt;
    return document;
}

QString storePath(const QString& path, const QString& base) {
    if (path.trimmed().isEmpty()) return path;
    const auto full = absolutePath(path);
    const auto relative = QDir(absolutePath(base)).relativeFilePath(full);
    if (QDir::isAbsolutePath(relative) || relative == QLatin1String(ParentDirectory) || relative.startsWith(QLatin1String(ParentPrefix)))
        return QDir::toNativeSeparators(full);
    return relative.isEmpty() ? QString::fromLatin1(DotDirectory) : relative;
}

QString resolvePath(const QString& storedPath, const QString& base) {
    if (storedPath.trimmed().isEmpty() || QDir::isAbsolutePath(storedPath)) return storedPath;
    return QDir::toNativeSeparators(QDir::cleanPath(QDir(absolutePath(base)).filePath(storedPath)));
}

QString machinePath(const QString& directory, const QUuid& id) {
    const QDir root(absolutePath(directory));
    return root.filePath(id.toString(QUuid::Id128) + u'/' + QLatin1String(MachineFileName));
}

RemoveResult removeMachineDirectory(const QString& directory, const QUuid& id, MachineDirectoryRemoval mode,
    std::stop_token token) {
    if (token.stop_requested()) return RemoveResult::Cancelled;
    if (directory.trimmed().isEmpty() || id.isNull()) return RemoveResult::InvalidDirectory;

    const auto document = machinePath(directory, id);
    const DocumentLock lock(document, token);
    if (token.stop_requested()) return RemoveResult::Cancelled;
    if (!lock.locked()) return RemoveResult::IoFailure;

    const QFileInfo machine(QFileInfo(document).absolutePath());
    const auto path = machine.filesystemAbsoluteFilePath();
    std::error_code error;
    const auto status = std::filesystem::symlink_status(path, error);
    if (status.type() == std::filesystem::file_type::not_found) return RemoveResult::Missing;
    if (error) return RemoveResult::IoFailure;
    if (!std::filesystem::is_directory(status) || machine.isJunction()) return RemoveResult::InvalidDirectory;

    const bool removed = mode == MachineDirectoryRemoval::Recursive
        ? std::filesystem::remove_all(path, error) != 0 : std::filesystem::remove(path, error);
    if (error) return RemoveResult::IoFailure;
    return removed ? RemoveResult::Removed : RemoveResult::Missing;
}

QStringList enumerateDocumentPaths(const QString& directory, const QString& fileName) {
    if (fileName.isEmpty() || QFileInfo(fileName).fileName() != fileName) return {};
    const QDir root(absolutePath(directory));
    constexpr auto Directories = QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot;
    QStringList paths;
    for (const auto& child : root.entryInfoList(Directories, QDir::NoSort)) {
        const auto path = QDir(child.absoluteFilePath()).filePath(fileName);
        if (QFileInfo(path).isFile()) paths.append(path);
    }
    std::sort(paths.begin(), paths.end(), [](const QString& left, const QString& right) {
        const int order = left.compare(right, Qt::CaseInsensitive);
        return order == 0 ? left < right : order < 0;
    });
    return paths;
}

std::optional<QJsonDocument> readDocument(const QString& path, std::stop_token token) {
    if (path.trimmed().isEmpty()) return std::nullopt;
    const auto fullPath = absolutePath(path);
    const DocumentLock lock(fullPath, token);
    if (!lock.locked() || token.stop_requested()) return std::nullopt;
    QFile file(fullPath);
    if (!file.open(QIODevice::ReadOnly)) return std::nullopt;
    const auto bytes = file.readAll();
    if (file.error() != QFileDevice::NoError || token.stop_requested()) return std::nullopt;
    const auto json = QJsonDocument::fromJson(bytes);
    return json.isNull() ? std::nullopt : std::optional<QJsonDocument>(json);
}

RestoreResult restoreMachineDocuments(const QString& directory, int schemaVersion,
    const MachineDocumentRestorer& restore, std::stop_token token) {
    if (token.stop_requested()) return RestoreResult::Cancelled;
    if (directory.trimmed().isEmpty() || !restore || !QDir().mkpath(directory) || !QDir(directory).isReadable())
        return RestoreResult::DirectoryError;

    for (const auto& path : enumerateDocumentPaths(directory)) {
        if (token.stop_requested()) return RestoreResult::Cancelled;
        const auto json = readDocument(path, token);
        if (!json) continue;
        if (const auto document = decodeMachineDocument(*json, schemaVersion); document && !token.stop_requested())
            restore(*document, path, token);
    }
    return token.stop_requested() ? RestoreResult::Cancelled : RestoreResult::Complete;
}

std::optional<QList<Document>> readDocuments(const QString& directory, const QString& fileName, std::stop_token token) {
    if (token.stop_requested()) return std::nullopt;
    QList<Document> documents;
    for (const auto& path : enumerateDocumentPaths(directory, fileName)) {
        if (token.stop_requested()) return std::nullopt;
        if (auto document = readDocument(path, token)) documents.append({path, std::move(*document)});
    }
    return token.stop_requested() ? std::nullopt : std::optional<QList<Document>>(std::move(documents));
}

WriteResult writeDocument(const QString& path, const QJsonDocument& json, std::stop_token token) {
    if (token.stop_requested()) return WriteResult::Cancelled;
    if (json.isNull()) return WriteResult::InvalidDocument;
    if (path.trimmed().isEmpty()) return WriteResult::IoFailure;
    const auto fullPath = absolutePath(path);
    const DocumentLock lock(fullPath, token);
    if (token.stop_requested()) return WriteResult::Cancelled;
    if (!lock.locked() || !QDir().mkpath(QFileInfo(fullPath).absolutePath())) return WriteResult::IoFailure;
    const auto bytes = json.toJson();
    QSaveFile file(fullPath);
    file.setDirectWriteFallback(false);
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size()) return WriteResult::IoFailure;
    if (token.stop_requested()) return WriteResult::Cancelled;
    return file.commit() ? WriteResult::Saved : WriteResult::IoFailure;
}

}
