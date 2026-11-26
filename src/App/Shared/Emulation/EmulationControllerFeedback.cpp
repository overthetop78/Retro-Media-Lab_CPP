#include "EmulationControllerFeedback.h"
#include "Logging/DiagnosticLog.h"
#include <ErrorKeys.h>
#include <RetroMediaLab/Emulation/Sdk.h>
#include <RetroMediaLab/Platform/Gamepads.h>
#include <QCoreApplication>
#include <algorithm>
#include <cmath>
#include <system_error>
#include <typeinfo>

namespace RetroMediaLab {
namespace {
constexpr auto DiagnosticContext = "Emulation.ControllerFeedback";
}

void EmulationControllerFeedback::update(Platform::GamepadSource& gamepads,
        const Emulation::IMachineControllerFeedback* source, const QString& logsDirectory,
        const QString& language) noexcept {
    if (!source) {
        gamepads.stopRumble();
        return;
    }
    try {
        QHash<QString, Platform::GamepadRumble> motors;
        for (const auto& output : source->controllerOutputs()) {
            if (output.deviceId.trimmed().isEmpty() || !std::isfinite(output.strength) ||
                output.strength <= 0.f || output.strength > 1.f) continue;
            using Motor = Emulation::ControllerMotor;
            float Platform::GamepadRumble::* field = nullptr;
            switch (output.motor) {
            case Motor::LowFrequency: field = &Platform::GamepadRumble::lowFrequency; break;
            case Motor::HighFrequency: field = &Platform::GamepadRumble::highFrequency; break;
            case Motor::LeftTrigger: field = &Platform::GamepadRumble::leftTrigger; break;
            case Motor::RightTrigger: field = &Platform::GamepadRumble::rightTrigger; break;
            }
            if (field) {
                auto& strength = motors[output.deviceId].*field;
                strength = std::max(strength, output.strength);
            }
        }
        gamepads.updateRumble(motors);
    } catch (...) {
        gamepads.stopRumble();
        try {
            try { throw; }
            catch (const std::system_error& error) {
                const auto& code = error.code();
                report(logsDirectory, language, QByteArray(typeid(error).name()) + ':' +
                    code.category().name() + ':' + QByteArray::number(code.value()), error.what());
            } catch (const std::exception& error) {
                report(logsDirectory, language, typeid(error).name(), error.what());
            } catch (...) {
                report(logsDirectory, language, {}, ErrorKeys::UnexpectedApplicationException);
            }
        } catch (...) {}
    }
}

void EmulationControllerFeedback::report(const QString& logsDirectory, const QString& language,
        QByteArray identity, const char* detail) {
    if (logsDirectory.isEmpty() || loggedErrors_.contains(identity)) return;
    loggedErrors_.insert(std::move(identity));
    writeDiagnostic(logsDirectory, DiagnosticSeverity::Error, QString::fromUtf8(detail),
        QLatin1String(DiagnosticContext), language, QCoreApplication::applicationFilePath());
}
}
