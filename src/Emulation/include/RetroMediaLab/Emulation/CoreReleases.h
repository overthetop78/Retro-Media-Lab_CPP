#pragma once

#include "EmulationExport.h"

#include <QDateTime>
#include <QHash>
#include <QUrl>
#include <functional>
#include <optional>
#include <stop_token>
#include <system_error>

class QNetworkAccessManager;

namespace RetroMediaLab::Emulation::CoreReleases {

struct HttpMetadata {
    QDateTime published;
    std::optional<qint64> archiveSize;
};

struct CatalogRelease {
    QString emulatorId, id, declaredVersion;
    QUrl downloadUri;
    HttpMetadata metadata;
    QString displayName;
    bool required = false;
    bool zip = true;
};

enum class HttpStatus { Success, Cancelled, NetworkError, OutputError };

struct QueryResult {
    HttpStatus status;
    int httpCode = 0;
    HttpMetadata metadata;
};

struct DownloadResult {
    HttpStatus status;
    int httpCode = 0;
    qint64 bytesWritten = 0;
};

using DownloadProgress = std::function<void(qint64 written, std::optional<qint64> total)>;

// The manager is borrowed. Call from its owning thread.
RETROMEDIALAB_EMULATION_API QueryResult queryRelease(QNetworkAccessManager&, const QUrl&, std::stop_token = {});
RETROMEDIALAB_EMULATION_API DownloadResult download(QNetworkAccessManager&, const QUrl&, const QString& destination,
    const DownloadProgress& = {}, std::stop_token = {});

RETROMEDIALAB_EMULATION_API std::optional<CatalogRelease> catalogRelease(const QString& emulatorId,
    const QUrl&, const HttpMetadata&, const QString& idPrefix, const QString& qtDateFormat);
RETROMEDIALAB_EMULATION_API CatalogRelease officialRelease(const QString& emulatorId, const QUrl&,
    const HttpMetadata&, const QString& emulatorDisplayName);

// The returned release is borrowed from the catalog.
RETROMEDIALAB_EMULATION_API const CatalogRelease* selectRelease(const QHash<QString, CatalogRelease>&,
    const QString& releaseId, const QString& expectedEmulator = {});

struct InstallationSteps {
    std::function<std::optional<qint64>(std::stop_token)> downloadArchive;
    std::function<bool(std::stop_token)> extractLibrary;
    std::function<bool(qint64 archiveSize, std::stop_token)> createManifest;
    std::function<bool(std::stop_token)> replaceLibrary, writeManifest, activateInstallation, prepareReplacement;
    std::function<void(qint64 archiveSize)> reportInstalled;
};

RETROMEDIALAB_EMULATION_API bool installLibrary(const InstallationSteps&, std::stop_token = {});

struct InstallationPaths {
    QString versionDirectory;
    QString libraryPath;
    QString manifestPath;
    QString installedVersion;
};

enum class ExtractResult { Extracted, Cancelled, ArchiveError, LibraryMissing, OutputError };

RETROMEDIALAB_EMULATION_API std::optional<InstallationPaths> installationPaths(
    const QString& root, const QString& coreId, const QString& version, const QString& libraryName,
    const QString& manifestName);

// Only the named regular file is written. The archive never supplies a destination path.
RETROMEDIALAB_EMULATION_API ExtractResult extractExpectedLibrary(const QString& archivePath,
    const QString& libraryName, const QString& destination, bool preserveLastWriteTime = false,
    std::stop_token = {});

// Staging and installed files must be on the same filesystem.
RETROMEDIALAB_EMULATION_API std::error_code replaceLibrary(const QString& stagedPath, const QString& destination);

RETROMEDIALAB_EMULATION_API QString readInstalledVersion(const QString& manifestPath, const QString& versionProperty,
    const QString& fallback);

RETROMEDIALAB_EMULATION_API std::optional<InstallationPaths> activeInstallation(const QString& markerPath,
    const QString& root, const QString& coreId, const QString& libraryName, const QString& manifestName,
    std::stop_token = {});

}
