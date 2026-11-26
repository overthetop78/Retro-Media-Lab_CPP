#include <RetroMediaLab/Emulation/CoreReleases.h>
#include <RetroMediaLab/Emulation/ConfigurationFiles.h>
#include <QDateTime>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSaveFile>
#include <QThread>
#include <QTimer>
#include <archive.h>
#include <archive_entry.h>
#include <array>
#include "StagedFile.h"
#include <exception>
#include <memory>

namespace RetroMediaLab::Emulation::CoreReleases {
namespace {
constexpr auto ReleaseVersionKey = QLatin1String("releaseVersion");
constexpr std::size_t BufferSize = 81920;
constexpr int FirstSuccessCode = 200, FirstRedirectCode = 300;
constexpr int CancellationPollMilliseconds = 25;
constexpr auto DateHeader = "Date";
constexpr char HttpUtcSuffix[] = " GMT";
constexpr auto RfcUtcSuffix = "+0000";
constexpr auto OfficialPrefix = QLatin1String("official-");
constexpr auto Latest = QLatin1String("latest");
constexpr auto OfficialVersionFormat = QLatin1String("yyyyMMdd-HHmm");
constexpr auto OfficialDisplayFormat = QLatin1String("dd/MM/yyyy HH:mm");
constexpr auto DisplaySeparator = QStringView(u" · ");

int httpCode(const QNetworkReply& reply) {
    return reply.attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
}

bool successfulStatus(int code) {
    return code >= FirstSuccessCode && code < FirstRedirectCode;
}

std::optional<qint64> contentLength(const QNetworkReply& reply) {
    bool valid = false;
    const auto length = reply.header(QNetworkRequest::ContentLengthHeader).toLongLong(&valid);
    return valid && length >= 0 ? std::optional<qint64>(length) : std::nullopt;
}

QDateTime responseDate(const QNetworkReply& reply) {
    const auto modified = reply.header(QNetworkRequest::LastModifiedHeader).toDateTime();
    if (modified.isValid()) return modified.toUTC();
    auto date = reply.rawHeader(DateHeader);
    if (date.endsWith(HttpUtcSuffix)) {
        date.chop(sizeof(HttpUtcSuffix) - 2);
        date += RfcUtcSuffix;
    }
    return QDateTime::fromString(QString::fromLatin1(date), Qt::RFC2822Date).toUTC();
}

void waitForReply(QNetworkReply& reply, std::stop_token token) {
    QEventLoop loop;
    QTimer cancellation;
    QObject::connect(&reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    if (token.stop_possible()) {
        QObject::connect(&cancellation, &QTimer::timeout, &reply, [&reply, token] {
            if (token.stop_requested() && !reply.isFinished()) reply.abort();
        });
        cancellation.start(CancellationPollMilliseconds);
    }
    if (token.stop_requested() && !reply.isFinished()) reply.abort();
    if (!reply.isFinished()) loop.exec(QEventLoop::ExcludeUserInputEvents);
}

bool isPathComponent(const QString& value) {
    return !value.trimmed().isEmpty() && !value.endsWith(u'.') && value == value.trimmed() &&
        !value.contains(u'/') && !value.contains(u'\\') && !value.contains(u':') && !value.contains(u'\0');
}

}

QueryResult queryRelease(QNetworkAccessManager& manager, const QUrl& uri, std::stop_token token) {
    if (token.stop_requested()) return {HttpStatus::Cancelled};
    if (manager.thread() != QThread::currentThread()) return {HttpStatus::NetworkError};
    QNetworkRequest request(uri);
    request.setAttribute(QNetworkRequest::AutoDeleteReplyOnFinishAttribute, false);
    const std::unique_ptr<QNetworkReply> reply(manager.head(request));
    waitForReply(*reply, token);
    const int code = httpCode(*reply);
    if (token.stop_requested()) return {HttpStatus::Cancelled, code};
    if (!reply->isFinished() || !successfulStatus(code) || reply->error() != QNetworkReply::NoError) return {HttpStatus::NetworkError, code};
    return {HttpStatus::Success, code, {responseDate(*reply), contentLength(*reply)}};
}

DownloadResult download(QNetworkAccessManager& manager, const QUrl& uri, const QString& destination,
    const DownloadProgress& progress, std::stop_token token) {
    if (token.stop_requested()) return {HttpStatus::Cancelled};
    if (manager.thread() != QThread::currentThread()) return {HttpStatus::NetworkError};
    QSaveFile output(destination);
    output.setDirectWriteFallback(false);
    if (!output.open(QIODevice::WriteOnly)) return {HttpStatus::OutputError};
    QNetworkRequest request(uri);
    request.setAttribute(QNetworkRequest::AutoDeleteReplyOnFinishAttribute, false);
    const std::unique_ptr<QNetworkReply> reply(manager.get(request));
    reply->setReadBufferSize(BufferSize);
    qint64 written = 0;
    bool outputError = false;
    std::exception_ptr callbackError;
    const auto consume = [&] {
        if (!successfulStatus(httpCode(*reply)) || outputError || callbackError) return;
        while (reply->bytesAvailable() > 0 && !token.stop_requested()) {
            const auto chunk = reply->read(BufferSize);
            if (chunk.isEmpty()) break;
            if (output.write(chunk) != chunk.size()) {
                outputError = true;
                break;
            }
            written += chunk.size();
            try {
                if (progress) progress(written, contentLength(*reply));
            } catch (...) {
                callbackError = std::current_exception();
                break;
            }
        }
        if ((outputError || callbackError || token.stop_requested()) && !reply->isFinished()) reply->abort();
    };
    QObject::connect(reply.get(), &QNetworkReply::readyRead, reply.get(), consume);
    waitForReply(*reply, token);
    consume();
    if (callbackError) std::rethrow_exception(callbackError);
    const int code = httpCode(*reply);
    if (token.stop_requested()) return {HttpStatus::Cancelled, code, written};
    if (outputError) return {HttpStatus::OutputError, code, written};
    if (!reply->isFinished() || !successfulStatus(code) || reply->error() != QNetworkReply::NoError)
        return {HttpStatus::NetworkError, code, written};
    return {output.commit() ? HttpStatus::Success : HttpStatus::OutputError, code, written};
}

std::optional<CatalogRelease> catalogRelease(const QString& emulatorId, const QUrl& uri,
    const HttpMetadata& metadata, const QString& idPrefix, const QString& qtDateFormat) {
    if (!metadata.published.isValid()) return std::nullopt;
    const auto version = metadata.published.toUTC().toString(qtDateFormat);
    return CatalogRelease{emulatorId, idPrefix + version, version, uri, metadata};
}

CatalogRelease officialRelease(const QString& emulatorId, const QUrl& uri, const HttpMetadata& metadata,
    const QString& emulatorDisplayName) {
    const bool dated = metadata.published.isValid();
    const auto suffix = dated ? metadata.published.toUTC().toString(OfficialVersionFormat) : QString(Latest);
    const auto id = OfficialPrefix + suffix;
    const auto label = dated ? metadata.published.toLocalTime().toString(OfficialDisplayFormat) +
        DisplaySeparator + emulatorDisplayName : emulatorDisplayName + DisplaySeparator + Latest;
    return {emulatorId, id, id, uri, metadata, label, true, true};
}

const CatalogRelease* selectRelease(const QHash<QString, CatalogRelease>& catalog, const QString& releaseId,
    const QString& expectedEmulator) {
    const auto release = catalog.constFind(releaseId);
    return release != catalog.cend() && (expectedEmulator.isEmpty() || release->emulatorId == expectedEmulator)
        ? &*release : nullptr;
}

bool installLibrary(const InstallationSteps& steps, std::stop_token token) {
    if (token.stop_requested() || !steps.downloadArchive || !steps.extractLibrary || !steps.createManifest ||
        !steps.replaceLibrary || !steps.writeManifest || !steps.activateInstallation) return false;
    const auto size = steps.downloadArchive(token);
    if (!size || token.stop_requested() || !steps.extractLibrary(token) || token.stop_requested() ||
        !steps.createManifest(*size, token) || token.stop_requested() ||
        (steps.prepareReplacement && !steps.prepareReplacement(token)) || token.stop_requested() ||
        !steps.replaceLibrary(token) || !steps.writeManifest(token) || !steps.activateInstallation(token)) return false;
    if (steps.reportInstalled) steps.reportInstalled(*size);
    return true;
}

namespace {
ExtractResult copyLibrary(archive& reader, archive_entry& entry, const QString& destination,
    bool preserveLastWriteTime, std::stop_token token) {
    QSaveFile output(destination);
    output.setDirectWriteFallback(false);
    if (!output.open(QIODevice::WriteOnly)) return ExtractResult::OutputError;
    std::array<char, BufferSize> buffer;
    while (!token.stop_requested()) {
        const auto bytes = archive_read_data(&reader, buffer.data(), buffer.size());
        if (bytes < 0) return ExtractResult::ArchiveError;
        if (bytes == 0) {
            if (preserveLastWriteTime && archive_entry_mtime_is_set(&entry) &&
                !output.setFileTime(QDateTime::fromSecsSinceEpoch(archive_entry_mtime(&entry)),
                    QFileDevice::FileModificationTime)) return ExtractResult::OutputError;
            if (token.stop_requested()) break;
            return output.commit() ? ExtractResult::Extracted : ExtractResult::OutputError;
        }
        if (output.write(buffer.data(), bytes) != bytes) return ExtractResult::OutputError;
    }
    return ExtractResult::Cancelled;
}
}

std::optional<InstallationPaths> installationPaths(const QString& root, const QString& coreId, const QString& version,
    const QString& libraryName, const QString& manifestName) {
    if (!QDir::isAbsolutePath(root) || !isPathComponent(coreId) || !isPathComponent(version) ||
        !isPathComponent(libraryName) || !isPathComponent(manifestName)) return std::nullopt;
    const QDir directory(QDir(root).absoluteFilePath(coreId + u'/' + version));
    return InstallationPaths{directory.absolutePath(), directory.filePath(libraryName),
        directory.filePath(manifestName), {}};
}

ExtractResult extractExpectedLibrary(const QString& archivePath, const QString& libraryName, const QString& destination,
    bool preserveLastWriteTime, std::stop_token token) {
    if (token.stop_requested()) return ExtractResult::Cancelled;
    if (!isPathComponent(libraryName)) return ExtractResult::LibraryMissing;
    const std::unique_ptr<archive, decltype(&archive_read_free)> reader(archive_read_new(), &archive_read_free);
    if (!reader || archive_read_support_format_zip(reader.get()) != ARCHIVE_OK) return ExtractResult::ArchiveError;
#ifdef Q_OS_WIN
    const auto source = archivePath.toStdWString();
    const auto opened = archive_read_open_filename_w(reader.get(), source.c_str(), BufferSize);
#else
    const auto source = QFile::encodeName(archivePath);
    const auto opened = archive_read_open_filename(reader.get(), source.constData(), BufferSize);
#endif
    if (opened != ARCHIVE_OK) return ExtractResult::ArchiveError;
    while (!token.stop_requested()) {
        archive_entry* entry = nullptr;
        const auto status = archive_read_next_header(reader.get(), &entry);
        if (status == ARCHIVE_EOF) return ExtractResult::LibraryMissing;
        if (status != ARCHIVE_OK || !entry) return ExtractResult::ArchiveError;
        if (archive_entry_filetype(entry) != AE_IFREG || archive_entry_hardlink(entry)) continue;
        const auto* utf8 = archive_entry_pathname_utf8(entry);
        auto name = utf8 ? QString::fromUtf8(utf8) : QString::fromLocal8Bit(archive_entry_pathname(entry));
        name.replace(u'\\', u'/');
        if (QStringView(name).sliced(name.lastIndexOf(u'/') + 1).compare(libraryName, Qt::CaseInsensitive) == 0)
            return copyLibrary(*reader, *entry, destination, preserveLastWriteTime, token);
    }
    return ExtractResult::Cancelled;
}

std::error_code replaceLibrary(const QString& stagedPath, const QString& destination) {
    return replaceStagedFile(stagedPath, destination);
}

QString readInstalledVersion(const QString& manifestPath, const QString& versionProperty, const QString& fallback) {
    const auto manifest = ConfigurationFiles::readDocument(manifestPath);
    return manifest ? manifest->object().value(versionProperty).toString(fallback) : fallback;
}

std::optional<InstallationPaths> activeInstallation(const QString& markerPath, const QString& root,
    const QString& coreId, const QString& libraryName, const QString& manifestName, std::stop_token token) {
    const auto marker = ConfigurationFiles::readDocument(markerPath, token);
    if (!marker || token.stop_requested()) return std::nullopt;
    const auto version = marker->object().value(ReleaseVersionKey).toString();
    auto paths = installationPaths(root, coreId, version, libraryName, manifestName);
    if (!paths || !QFileInfo(paths->libraryPath).isFile() || !QFileInfo(paths->manifestPath).isFile())
        return std::nullopt;
    paths->installedVersion = version;
    return paths;
}

}
