#include <RetroMediaLab/Emulation/CoreDiagnostics.h>
#include <utility>

namespace RetroMediaLab::Emulation {
namespace {
#ifdef Q_OS_WIN
constexpr auto DiagnosticNewline = QLatin1String("\r\n");
#else
constexpr auto DiagnosticNewline = QLatin1String("\n");
#endif
}

CoreDiagnosticMonitor::CoreDiagnosticMonitor(QStringList signatures, FailureFactory createFailure)
    : signatures_(std::move(signatures)), createFailure_(std::move(createFailure)) {}

void CoreDiagnosticMonitor::capture(QStringView message) noexcept {
    try {
        std::lock_guard lock(mutex_);
        if (failure_) return;
        for (const auto& signature : signatures_) {
            if (message.contains(signature, Qt::CaseSensitive)) {
                failure_.emplace(message.toString());
                return;
            }
        }
    } catch (...) {
        // A native callback cannot propagate a C++ exception to the core.
    }
}

std::optional<QString> CoreDiagnosticMonitor::failure() const {
    std::lock_guard lock(mutex_);
    return failure_;
}

void CoreDiagnosticMonitor::throwIfFailed() const {
    if (const auto diagnostic = failure(); diagnostic && createFailure_) {
        if (auto error = createFailure_(*diagnostic)) std::rethrow_exception(error);
    }
}

NativeDiagnosticsError::NativeDiagnosticsError(const QString& message, std::exception_ptr cause)
    : std::runtime_error(message.toStdString()), cause_(std::move(cause)) {}

const std::exception_ptr& NativeDiagnosticsError::cause() const noexcept { return cause_; }

std::exception_ptr withNativeDiagnostics(std::exception_ptr cause, const QStringList& diagnostics) {
    if (!cause || diagnostics.isEmpty()) return cause;
    try {
        std::rethrow_exception(cause);
    } catch (const std::exception& error) {
        const auto block = diagnostics.join(DiagnosticNewline);
        const auto message = QString::fromUtf8(error.what());
        if (message.contains(block, Qt::CaseSensitive)) return cause;
        return std::make_exception_ptr(NativeDiagnosticsError(message + DiagnosticNewline + block, cause));
    } catch (...) {
        return cause;
    }
}
}
