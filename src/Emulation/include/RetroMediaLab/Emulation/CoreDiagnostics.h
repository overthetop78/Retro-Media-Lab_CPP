#pragma once

#include "EmulationExport.h"
#include <QStringList>
#include <QStringView>
#include <exception>
#include <functional>
#include <mutex>
#include <optional>
#include <stdexcept>

namespace RetroMediaLab::Emulation {
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
