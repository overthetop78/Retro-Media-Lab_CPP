#pragma once

#include "Host.h"
#include <array>
#include <mutex>

namespace RetroMediaLab::Emulation {
namespace ControllerFeedbackCommands {
inline constexpr auto Strong = QLatin1String("RumbleStrong");
inline constexpr auto Weak = QLatin1String("RumbleWeak");
inline constexpr unsigned MaximumPortCount = 10;
}

struct ControllerFeedbackState {
    int port;
    QString commandId;
    float strength;
};

struct ControllerOutputBinding {
    QString deviceId;
    ControllerMotor motor;
    bool operator==(const ControllerOutputBinding&) const = default;
};

struct ControllerPortFeedback {
    int number;
    QSet<QString> availableCommands;
    QHash<QString, ControllerOutputBinding> outputs;
    QString displayName;
    QHash<QString, QString> commandResourceKeys;
};

using ControllerFeedbackBindings = QHash<QPair<int, QString>, ControllerOutputBinding>;

class IModuleControllerFeedbackSettings {
public:
    virtual ~IModuleControllerFeedbackSettings() = default;
    virtual QList<ControllerPortFeedback> controllerFeedbackPorts(const Configuration&) const = 0;
    // Apply only the ports present in this list. Empty outputs clear a port.
    virtual Configuration applyControllerFeedbackSettings(Configuration, const QList<ControllerPortFeedback>&) = 0;
};

class IMachineControllerFeedbackSettings {
public:
    virtual ~IMachineControllerFeedbackSettings() = default;
    virtual void setControllerFeedbackBindings(const ControllerFeedbackBindings&) = 0;
};

RETROMEDIALAB_EMULATION_API void assignControllerRumbleDevice(ControllerPortFeedback& port, const QString& deviceId);
RETROMEDIALAB_EMULATION_API ControllerFeedbackBindings createControllerFeedbackBindings(const QList<ControllerPortFeedback>& ports);
RETROMEDIALAB_EMULATION_API QList<ControllerOutput> resolveControllerFeedback(const QList<ControllerFeedbackState>& states,
    const ControllerFeedbackBindings& bindings);

class RETROMEDIALAB_EMULATION_API ControllerFeedbackAccumulator final {
public:
    bool setState(unsigned port, unsigned effect, quint16 strength);
    QList<ControllerFeedbackState> snapshot() const;
    void resetPort(int port);
    void reset();

private:
    static constexpr unsigned StrongEffect = 0;
    static constexpr unsigned WeakEffect = 1;
    static constexpr unsigned EffectCount = 2;
    mutable std::mutex mutex_;
    std::array<std::array<quint16, EffectCount>, ControllerFeedbackCommands::MaximumPortCount> strengths_{};
};
}
