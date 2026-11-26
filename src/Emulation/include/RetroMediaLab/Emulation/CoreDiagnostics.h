#pragma once

#include "EmulationExport.h"
#include <QDateTime>
#include <QStringList>
#include <QStringView>
#include <exception>
#include <functional>
#include <mutex>
#include <optional>
#include <stdexcept>

namespace RetroMediaLab::Emulation {
namespace CoreDiagnostics {
struct Manifest {
    QString releaseId, releaseVersion, downloadUrl;
    QDateTime downloadedUtc;
    qint64 archiveSize, librarySize;
    QString librarySha256, architecture, declaredVersion;
    QStringList exports;
};

using ErrorText = std::function<QString()>;

RETROMEDIALAB_EMULATION_API void requireArchitecture(const QString& actual, const QString& expected,
                                                      const ErrorText& error);
RETROMEDIALAB_EMULATION_API void verifyWindowsX64Library(const QString& path, const ErrorText& notPe,
    const ErrorText& invalidPe, const ErrorText& wrongArchitecture);
RETROMEDIALAB_EMULATION_API QString calculateSha256(const QString& path);
RETROMEDIALAB_EMULATION_API Manifest createManifest(const QString& releaseId, const QString& releaseVersion,
    const QString& downloadUrl, qint64 archiveSize, const QString& libraryPath);
RETROMEDIALAB_EMULATION_API QString findInstalledLibraryPath(const QString& path,
    const std::function<void(const QString&)>& verifyInstallation = {},
    const std::function<bool()>& isInstallationComplete = {});
}

class RETROMEDIALAB_EMULATION_API CoreDiagnosticMonitor final {
public:
    using FailureFactory = std::function<std::exception_ptr(const QString&)>;

    CoreDiagnosticMonitor(QStringList signatures, FailureFactory createFailure);
    // Called from the core's native logging callback.
    void capture(QStringView message) noexcept;
    std::optional<QString> failure() const;
    // Called after control has returned to the module.
    void throwIfFailed() const;

private:
    const QStringList signatures_;
    const FailureFactory createFailure_;
    mutable std::mutex mutex_;
    std::optional<QString> failure_;
};

class RETROMEDIALAB_EMULATION_API NativeDiagnosticsError final : public std::runtime_error {
public:
    NativeDiagnosticsError(const QString& message, std::exception_ptr cause);
    const std::exception_ptr& cause() const noexcept;

private:
    std::exception_ptr cause_;
};

RETROMEDIALAB_EMULATION_API std::exception_ptr withNativeDiagnostics(std::exception_ptr cause,
    const QStringList& diagnostics);
}
