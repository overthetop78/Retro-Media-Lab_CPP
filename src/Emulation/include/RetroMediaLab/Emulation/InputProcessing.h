#pragma once

#include "EmulationKey.h"
#include "HostProtocol.h"
#include "InputSettings.h"
#include <QSet>
#include <type_traits>

namespace RetroMediaLab::Emulation::InputProcessing {
// Returned references remain valid while the supplied settings and their lists are unchanged.
RETROMEDIALAB_EMULATION_API const ControllerPortSettings* controllerPort(const InputSettings&, int index);
RETROMEDIALAB_EMULATION_API const QList<InputBindingDefinition>* controllerBindingDefinitions(
    const InputSettings&, QStringView peripheral, int index);

// USB indices are relative to the USB bus, sorted by public port number.
RETROMEDIALAB_EMULATION_API QList<const ControllerChoice*> supportedControllerChoices(
    const InputSettings&, std::optional<int> port = {}, bool usb = false);

// The adapter resolves identifiers to defined values of its own enum, using a
// case-insensitive lookup as in the managed SDK. Unknown identifiers return nullopt.
template<class Peripheral, class Resolve>
QList<Peripheral> peripherals(const InputSettings& settings, Resolve&& resolve,
    std::optional<int> port = {}, bool usb = false) {
    static_assert(std::is_enum_v<Peripheral>);
    QList<Peripheral> result;
    for (const auto* choice : supportedControllerChoices(settings, port, usb))
        if (const std::optional<Peripheral> value = resolve(QStringView(choice->id)); value && !result.contains(*value))
            result.append(*value);
    return result;
}

struct InputActionDefinition {
    InputBindingDefinition definition;
    std::optional<EmulationKey> key;
};

RETROMEDIALAB_EMULATION_API QList<InputActionDefinition> describeInput(
    const InputSettings&, int port, QStringView peripheral);

RETROMEDIALAB_EMULATION_API int controllerButtonIndex(QStringView);
RETROMEDIALAB_EMULATION_API QHash<QString, QString> resolveBindings(const InputBindingSet&);
RETROMEDIALAB_EMULATION_API bool isValidBinding(QStringView);
// An absent result indicates an unknown command, duplicate definition, or invalid override.
RETROMEDIALAB_EMULATION_API std::optional<QHash<QString, QString>> resolveKeyboardBindings(const InputBindingSet&);
RETROMEDIALAB_EMULATION_API bool hasInvalidControllerPortNumbers(const InputSettings&, int expectedCount);
RETROMEDIALAB_EMULATION_API const HostProtocol::CoreController* resolveController(
    QStringView deviceId, const QList<HostProtocol::CoreController>&, qsizetype fallbackIndex = -1);
RETROMEDIALAB_EMULATION_API float applyDeadZone(float pressure, int percent);
RETROMEDIALAB_EMULATION_API HostProtocol::CoreController applyDeadZones(HostProtocol::CoreController, int percent);

// The unqualified controller is supplied by the caller. An explicitly named
// device that is absent always contributes zero pressure.
RETROMEDIALAB_EMULATION_API float sourcePressure(QStringView source, const HostProtocol::CoreInput&,
    const HostProtocol::CoreController* fallback = nullptr, int deadZone = 0);
RETROMEDIALAB_EMULATION_API qint16 mapControllerAxis(const QHash<QString, QString>& mappings,
    const QString& positiveCommand, const QString& negativeCommand, const HostProtocol::CoreInput&,
    const HostProtocol::CoreController* fallback = nullptr, int deadZone = 0);
RETROMEDIALAB_EMULATION_API qint16 mapControllerPressure(const QHash<QString, QString>& mappings,
    const QString& command, const HostProtocol::CoreInput&,
    const HostProtocol::CoreController* fallback = nullptr, int deadZone = 0);

// The caller supplies the resolved controller for a single controller mapping.
RETROMEDIALAB_EMULATION_API bool isMappedInputPressed(QStringView mapping,
    const HostProtocol::CoreInput&, const HostProtocol::CoreController* = nullptr);
RETROMEDIALAB_EMULATION_API float mappedInputMagnitude(QStringView mapping,
    const HostProtocol::CoreInput&, const HostProtocol::CoreController*, int deadZone);

RETROMEDIALAB_EMULATION_API std::optional<QString> oppositeControllerAxisSource(QStringView);

// The first assignment in definition order owns a physical key. Chords,
// mouse and controller inputs contribute additional emulated keys.
RETROMEDIALAB_EMULATION_API QSet<EmulationKey> mapKeyboard(
    const HostProtocol::CoreInput&, const InputBindingSet&);

struct NativeKeyboardEvent {
    bool pressed;
    quint32 code;
    quint32 character;
    quint16 modifiers;
};

RETROMEDIALAB_EMULATION_API const QHash<EmulationKey, quint32>& nativeKeyboardCodes();
RETROMEDIALAB_EMULATION_API quint16 nativeKeyboardModifiers(const QSet<EmulationKey>&);
RETROMEDIALAB_EMULATION_API quint32 nativeKeyboardCharacter(quint32 code, const QSet<EmulationKey>&);
RETROMEDIALAB_EMULATION_API QList<NativeKeyboardEvent> nativeKeyboardTransitions(
    const QSet<EmulationKey>& keys, const QSet<EmulationKey>& previous,
    const QHash<EmulationKey, quint32>& codes, const QSet<EmulationKey>& modifierKeys);
RETROMEDIALAB_EMULATION_API qint16 normalizedAxis(float value);

// Keys and peripheral requests use the hosted-core protocol identities.
RETROMEDIALAB_EMULATION_API qint16 nativeInputState(const HostProtocol::CoreInput&, quint32 port,
    quint32 device, quint32 index, quint32 id);
}
