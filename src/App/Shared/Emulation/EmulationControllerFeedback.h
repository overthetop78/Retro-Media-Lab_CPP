#pragma once

#include "AppApi.h"
#include <QByteArray>
#include <QSet>

namespace RetroMediaLab {
namespace Emulation { class IMachineControllerFeedback; }
namespace Platform { class GamepadSource; }

class EmulationControllerFeedback final {
public:
    RETROMEDIALAB_APP_API void update(Platform::GamepadSource&, const Emulation::IMachineControllerFeedback*,
                                     const QString& logsDirectory, const QString& language) noexcept;

private:
    void report(const QString& logsDirectory, const QString& language, QByteArray identity,
                const char* detail);

    QSet<QByteArray> loggedErrors_;
};
}
