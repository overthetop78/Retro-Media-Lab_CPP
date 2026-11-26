#pragma once

#include "Shared/Emulation/EmulationKeyboardShortcuts.h"
#include "Shared/Emulation/EmulationHostKeyboard.h"
#include <RetroMediaLab/Emulation/InputProcessing.h>
#include <RetroMediaLab/Platform/Gamepads.h>
#include <QPoint>
#include <QSet>
#include <QStringList>
#include <Qt>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iterator>
#include <utility>

namespace RetroMediaLab::EmulationPhysicalShortcuts {
inline constexpr auto MousePrefix = "Mouse:";
inline constexpr auto ControllerPrefix = "Controller:";
inline constexpr auto Separator = QLatin1Char(':');
inline constexpr auto ChordSeparator = QLatin1Char('+');
inline constexpr float PressedThreshold = 0.5f;
inline constexpr float RawCaptureDelta = 0.35f;
inline constexpr auto PositiveSuffix = "Positive";
inline constexpr auto NegativeSuffix = "Negative";

inline constexpr std::pair<QLatin1StringView, Qt::MouseButton> MouseButtons[] = {
    {QLatin1String("Left"), Qt::LeftButton}, {QLatin1String("Right"), Qt::RightButton},
    {QLatin1String("Middle"), Qt::MiddleButton}, {QLatin1String("XButton1"), Qt::XButton1},
    {QLatin1String("XButton2"), Qt::XButton2},
};

namespace Controls {
using namespace Platform;
struct ButtonMapping { QLatin1StringView name; QLatin1StringView alias; GamepadButton button; };
inline constexpr ButtonMapping GamepadButtons[] = {
    {QLatin1String("ButtonA"), QLatin1String("A"), Confirm},
    {QLatin1String("ButtonB"), QLatin1String("B"), Cancel},
    {QLatin1String("ButtonX"), QLatin1String("X"), X},
    {QLatin1String("ButtonY"), QLatin1String("Y"), Y},
    {QLatin1String("Menu"), QLatin1String("Start"), Menu},
    {QLatin1String("View"), QLatin1String("Select"), View},
    {QLatin1String("DPadUp"), QLatin1String("Up"), Up},
    {QLatin1String("DPadDown"), QLatin1String("Down"), Down},
    {QLatin1String("DPadLeft"), QLatin1String("Left"), Left},
    {QLatin1String("DPadRight"), QLatin1String("Right"), Right},
    {QLatin1String("LeftShoulder"), QLatin1String("L"), LeftShoulder},
    {QLatin1String("RightShoulder"), QLatin1String("R"), RightShoulder},
    {QLatin1String("LeftStickClick"), QLatin1String("L3"), LeftStick},
    {QLatin1String("RightStickClick"), QLatin1String("R3"), RightStick},
    {QLatin1String("PaddleLeft1"), {}, PaddleLeft1},
    {QLatin1String("PaddleLeft2"), {}, PaddleLeft2},
    {QLatin1String("PaddleRight1"), {}, PaddleRight1},
    {QLatin1String("PaddleRight2"), {}, PaddleRight2},
};
enum Axis { LeftX, LeftY, RightX, RightY, LeftTrigger, RightTrigger };
struct AxisDirection {
    QLatin1StringView name;
    Axis axis;
    float sign;
    QLatin1StringView alias{};
    GamepadButton digital = NoButton;
};
inline constexpr AxisDirection Directions[] = {
    {QLatin1String("LeftStickLeft"), LeftX, -1}, {QLatin1String("LeftStickRight"), LeftX, 1},
    {QLatin1String("LeftStickUp"), LeftY, 1}, {QLatin1String("LeftStickDown"), LeftY, -1},
    {QLatin1String("RightStickLeft"), RightX, -1}, {QLatin1String("RightStickRight"), RightX, 1},
    {QLatin1String("RightStickUp"), RightY, 1}, {QLatin1String("RightStickDown"), RightY, -1},
    {QLatin1String("LeftTrigger"), LeftTrigger, 1, QLatin1String("L2"), LeftTriggerButton},
    {QLatin1String("RightTrigger"), RightTrigger, 1, QLatin1String("R2"), RightTriggerButton},
};

inline float axisPressure(const GamepadState& pad, const AxisDirection& direction, GamepadState* held = nullptr) {
    const auto value = pad.axes.value(direction.axis);
    const auto analog = std::clamp(value * direction.sign, 0.0f, 1.0f);
    const auto digital = pad.buttons & direction.digital;
    if (held && analog > PressedThreshold) {
        if (held->axes.size() <= direction.axis) held->axes.resize(direction.axis + 1);
        held->axes[direction.axis] = value;
    }
    if (held && digital) held->buttons |= digital;
    return digital && analog == 0.0f ? 1.0f : analog;
}
}

struct Input {
    Qt::MouseButtons mouse;
    QPoint wheel;
    const QList<Platform::GamepadState>& controllers;
    Qt::KeyboardModifiers modifiers{};
    QSet<int> keys;
};

inline float controllerPressure(QStringView source, const Platform::GamepadState& pad,
    Platform::GamepadState* held = nullptr) {
    if (!source.startsWith(QLatin1String(ControllerPrefix), Qt::CaseInsensitive)) return 0.0f;
    source = source.sliced(QLatin1String(ControllerPrefix).size());
    const auto separator = source.lastIndexOf(Separator);
    const auto device = separator < 0 ? QStringView{} : source.first(separator);
    const auto control = source.sliced(separator + 1);
    const bool guideAlias = control.compare(QLatin1String("Guide"), Qt::CaseInsensitive) == 0;
    if (!device.isEmpty() && device.compare(pad.deviceId, Qt::CaseInsensitive) != 0) return 0.0f;
    for (const auto& [name, alias, button] : Controls::GamepadButtons) {
        if (control.compare(name, Qt::CaseInsensitive) != 0 &&
            (alias.isEmpty() || control.compare(alias, Qt::CaseInsensitive) != 0))
            continue;
        const bool pressed = (pad.buttons & button) != 0;
        if (held && pressed) held->buttons |= button;
        return pressed ? 1.0f : 0.0f;
    }
    for (const auto& direction : Controls::Directions) {
        if (control.compare(direction.name, Qt::CaseInsensitive) == 0 ||
            (!direction.alias.isEmpty() && control.compare(direction.alias, Qt::CaseInsensitive) == 0))
            return Controls::axisPressure(pad, direction, held);
    }
    namespace Raw = Platform::RawGamepad;
    const auto positive = QLatin1String(PositiveSuffix);
    const auto negative = QLatin1String(NegativeSuffix);
    const int direction = control.endsWith(positive, Qt::CaseInsensitive) ? 1 :
        control.endsWith(negative, Qt::CaseInsensitive) ? -1 : 0;
    const auto name = direction ? control.chopped(direction > 0 ? positive.size() : negative.size()) : control;
    for (auto value = pad.controls.constBegin(); value != pad.controls.constEnd(); ++value) {
        if (name.compare(value.key(), Qt::CaseInsensitive) != 0 &&
            !(guideAlias && value.key().compare(QLatin1String(Raw::GuideButton), Qt::CaseInsensitive) == 0)) continue;
        const auto pressure = !direction ? std::clamp(*value, 0.0f, 1.0f) : Raw::isAxis(value.key()) ?
            std::clamp(static_cast<float>(direction) * (*value / Raw::AxisCenter - 1.0f), 0.0f, 1.0f) : 0.0f;
        if (held && pressure > PressedThreshold) held->controls.insert(value.key(), *value);
        return pressure;
    }
    return 0.0f;
}

inline float controllerPressure(QStringView source, const QList<Platform::GamepadState>& controllers) {
    float strongest = 0.0f;
    for (const auto& controller : controllers)
        strongest = std::max(strongest, controllerPressure(source, controller));
    return strongest;
}

inline bool pressed(QStringView source, const Input& input) {
    if (source.startsWith(QLatin1String(MousePrefix), Qt::CaseInsensitive)) {
        source = source.sliced(QLatin1String(MousePrefix).size());
        for (const auto& [name, button] : MouseButtons)
            if (source.compare(name, Qt::CaseInsensitive) == 0) return input.mouse.testFlag(button);
        if (source.compare(QLatin1String("WheelUp"), Qt::CaseInsensitive) == 0) return input.wheel.y() > 0;
        if (source.compare(QLatin1String("WheelDown"), Qt::CaseInsensitive) == 0) return input.wheel.y() < 0;
        if (source.compare(QLatin1String("WheelLeft"), Qt::CaseInsensitive) == 0) return input.wheel.x() < 0;
        if (source.compare(QLatin1String("WheelRight"), Qt::CaseInsensitive) == 0) return input.wheel.x() > 0;
        return false;
    }
    return controllerPressure(source, input.controllers) > PressedThreshold;
}

inline QString mouseSource(Qt::MouseButton button) {
    for (const auto& [name, value] : MouseButtons)
        if (button == value) return QLatin1String(MousePrefix) + name;
    return {};
}

inline QString wheelSource(const QPoint& delta) {
    if (delta.y() > 0) return QLatin1String("Mouse:WheelUp");
    if (delta.y() < 0) return QLatin1String("Mouse:WheelDown");
    if (delta.x() > 0) return QLatin1String("Mouse:WheelRight");
    if (delta.x() < 0) return QLatin1String("Mouse:WheelLeft");
    return {};
}

inline QStringList newControllerSources(const QList<Platform::GamepadState>& current,
    const QList<Platform::GamepadState>& previous,
    const QList<Platform::GamepadState>& initial) {
    QStringList sources;
    for (const auto& pad : current) {
        const auto baseline = std::ranges::find(previous, pad.id, &Platform::GamepadState::id);
        if (baseline == previous.end()) continue;
        auto prefix = QString(QLatin1String(ControllerPrefix));
        if (!pad.deviceId.isEmpty()) prefix += pad.deviceId + Separator;
        namespace Raw = Platform::RawGamepad;
        bool hasRawButtons = false, hasRawAxes = false, hasRawSwitches = false;
        const auto initialPad = std::ranges::find(initial, pad.id, &Platform::GamepadState::id);
        for (auto control = pad.controls.constBegin(); control != pad.controls.constEnd(); ++control) {
            const auto& name = control.key();
            if (Raw::isAxis(name)) {
                hasRawAxes = true;
                const float rest = initialPad == initial.end() ? Raw::AxisCenter :
                    initialPad->controls.value(name, Raw::AxisCenter);
                const float prior = baseline->controls.value(name, rest);
                const auto activates = [&](float value) {
                    const auto delta = value - rest;
                    return delta >= RawCaptureDelta ? 1 : delta <= -RawCaptureDelta ? -1 : 0;
                };
                const auto direction = activates(*control);
                if (direction != 0 && direction != activates(prior))
                    sources.append(prefix + name + QLatin1String(direction > 0 ? PositiveSuffix : NegativeSuffix));
            } else {
                hasRawButtons |= name.startsWith(QLatin1String(Raw::ButtonPrefix));
                hasRawSwitches |= name.startsWith(QLatin1String(Raw::SwitchPrefix));
                if (*control > PressedThreshold && baseline->controls.value(name) <= PressedThreshold)
                    sources.append(prefix + name);
            }
        }
        const auto newButtons = pad.buttons & ~baseline->buttons;
        constexpr quint64 DPad = Platform::Up | Platform::Down | Platform::Left | Platform::Right;
        for (const auto& [name, alias, button] : Controls::GamepadButtons) {
            const bool hasRawEquivalent = (button & DPad) != 0 ? hasRawSwitches : hasRawButtons;
            if (!hasRawEquivalent && (newButtons & button) != 0) sources.append(prefix + name);
        }
        if (!hasRawAxes) {
            for (const auto& direction : Controls::Directions)
                if (Controls::axisPressure(pad, direction) > PressedThreshold &&
                    Controls::axisPressure(*baseline, direction) <= PressedThreshold)
                    sources.append(prefix + direction.name);
        }
    }
    return sources;
}

inline constexpr auto KeyboardPrefix = "Keyboard:";
inline constexpr auto ChordModifiers = Qt::ControlModifier | Qt::AltModifier | Qt::ShiftModifier | Qt::MetaModifier;

struct Chord {
    QStringList sources;
    QSet<int> keys;
    Qt::KeyboardModifiers modifiers{};
    bool keyboard = false;
    bool reserved = false;
    bool allowSingleKey = false;

    bool isEmpty() const {
        const bool modifierOnly = keys.isEmpty() && (modifiers & ChordModifiers) != Qt::NoModifier;
        return reserved || (sources.isEmpty() && !modifierOnly && keys.size() < (allowSingleKey ? 1 : 2));
    }
};

inline bool modifierKey(int key);

inline Chord chord(const QString& configured, bool allowSingleKey = false) {
    Chord result;
    result.allowSingleKey = allowSingleKey;
    const auto parts = configured.split(ChordSeparator);
    constexpr std::pair<QLatin1StringView, Qt::KeyboardModifier> modifiers[] = {
        {QLatin1String("Ctrl"), Qt::ControlModifier}, {QLatin1String("Control"), Qt::ControlModifier},
        {QLatin1String("Shift"), Qt::ShiftModifier}, {QLatin1String("Maj"), Qt::ShiftModifier},
        {QLatin1String("Alt"), Qt::AltModifier}, {QLatin1String("Meta"), Qt::MetaModifier},
        {QLatin1String("Win"), Qt::MetaModifier}, {QLatin1String("Windows"), Qt::MetaModifier},
        {QLatin1String("Num"), Qt::KeypadModifier},
    };
    for (auto part : parts) {
        part = part.trimmed();
        if (part.startsWith(QLatin1String(MousePrefix), Qt::CaseInsensitive) ||
            part.startsWith(QLatin1String(ControllerPrefix), Qt::CaseInsensitive)) {
            if (!Emulation::InputProcessing::isValidBinding(part)) return {};
            result.sources.append(part.toCaseFolded());
            continue;
        }
        if (part.startsWith(QLatin1String(KeyboardPrefix), Qt::CaseInsensitive))
            part.remove(0, QLatin1String(KeyboardPrefix).size());
        result.keyboard = true;
        const auto modifier = std::ranges::find_if(modifiers,
            [&](const auto& entry) { return part.compare(entry.first, Qt::CaseInsensitive) == 0; });
        if (modifier != std::end(modifiers)) {
            result.modifiers |= modifier->second;
            continue;
        }
        if (part.compare(QLatin1String("OemPlus"), Qt::CaseInsensitive) == 0 ||
            part.compare(QLatin1String("Add"), Qt::CaseInsensitive) == 0) {
            if (part.compare(QLatin1String("Add"), Qt::CaseInsensitive) == 0) result.modifiers |= Qt::KeypadModifier;
            result.keys.insert(Qt::Key_Plus);
            continue;
        }
        const auto host = EmulationHostKeyboard::hostCombination(part);
        if (host.key() != Qt::Key_unknown) {
            result.modifiers |= host.keyboardModifiers();
            if (!modifierKey(host.key())) result.keys.insert(host.key());
            continue;
        }
        const auto key = EmulationKeyboardShortcuts::parse(part);
        if (key.count() != 1) return {};
        result.modifiers |= key[0].keyboardModifiers();
        result.keys.insert(key[0].key());
    }
    if (result.keyboard) {
        if (result.keys.isEmpty() && result.modifiers == Qt::NoModifier) return {};
        result.reserved = EmulationKeyboardShortcuts::reserved(
            QKeySequence(QKeyCombination(result.modifiers, Qt::Key_unknown))) ||
            std::ranges::any_of(result.keys, [&](const int key) {
                return EmulationKeyboardShortcuts::reserved(QKeySequence(QKeyCombination(result.modifiers, Qt::Key(key))));
            });
    }
    result.sources.removeDuplicates();
    result.sources.sort();
    return result;
}

inline bool pressed(const Chord& chord, const Input& input, bool allowAdditionalKeys = false) {
    if (chord.isEmpty()) return false;
    auto mask = ChordModifiers;
    if (chord.modifiers.testFlag(Qt::KeypadModifier)) mask |= Qt::KeypadModifier;
    if (chord.keyboard && ((input.modifiers & mask) != chord.modifiers ||
        (!allowAdditionalKeys && input.keys != chord.keys) ||
        (allowAdditionalKeys && !input.keys.contains(chord.keys)))) return false;
    return std::ranges::all_of(chord.sources, [&](const QString& source) { return pressed(QStringView(source), input); });
}

class ShortcutRepeat final {
public:
    using Clock = std::chrono::steady_clock;

    bool due(const EmulationKeyboardShortcuts::Binding* direction, Clock::time_point now) {
        if (direction != direction_) {
            direction_ = direction;
            next_ = now + InitialDelay;
        } else if (direction && now >= next_) {
            next_ = now + Interval;
            return true;
        }
        return false;
    }

    void clear() { direction_ = nullptr; }

private:
    static constexpr auto InitialDelay = std::chrono::milliseconds(300);
    static constexpr auto Interval = std::chrono::milliseconds(35);
    const EmulationKeyboardShortcuts::Binding* direction_ = nullptr;
    Clock::time_point next_{};
};

inline bool isOsdShortcut(QStringView id) {
    namespace Keys = EmulationKeyboardShortcuts;
    constexpr const Keys::Binding* bindings[]{
        &Keys::OsdWithPause, &Keys::OsdWithoutPause, &Keys::OsdStatistics, &Keys::OsdUp, &Keys::OsdDown,
        &Keys::OsdLeft, &Keys::OsdRight, &Keys::OsdConfirm, &Keys::OsdCancel,
        &Keys::ToggleMute, &Keys::VolumeDown, &Keys::VolumeUp};
    const auto suffix = QLatin1String(Keys::AlternativeSuffix);
    const bool alternative = id.endsWith(suffix);
    if (alternative) id = id.chopped(suffix.size());
    return std::ranges::any_of(bindings, [&](const Keys::Binding* binding) {
        return id == QLatin1String(binding->id) && (!alternative || binding->hasAlternative);
    });
}

inline bool hasMidiSource(const Chord& chord) {
    constexpr auto MidiSourcePrefix = "Controller:midi:";
    return std::ranges::any_of(chord.sources, [](const QString& source) {
        return source.startsWith(QLatin1String(MidiSourcePrefix), Qt::CaseInsensitive);
    });
}

inline bool conflicts(const Chord& left, const Chord& right) {
    return !left.isEmpty() && !right.isEmpty() && left.sources == right.sources && left.keyboard == right.keyboard &&
        left.modifiers == right.modifiers && left.keys == right.keys;
}

inline QString formatKeyboard(Qt::KeyboardModifiers modifiers, const QList<int>& keys) {
    QStringList parts;
    if (modifiers.testFlag(Qt::ControlModifier)) parts.append(QLatin1String("Ctrl"));
    if (modifiers.testFlag(Qt::AltModifier)) parts.append(QLatin1String("Alt"));
    if (modifiers.testFlag(Qt::ShiftModifier)) parts.append(QLatin1String("Shift"));
    if (modifiers.testFlag(Qt::MetaModifier)) parts.append(QLatin1String("Meta"));
    if (modifiers.testFlag(Qt::KeypadModifier)) parts.append(QLatin1String("Num"));
    for (const int key : keys)
        parts.append(key == Qt::Key_Plus ? QString(QLatin1String("OemPlus")) :
            QKeySequence(Qt::Key(key)).toString(QKeySequence::PortableText));
    return parts.join(ChordSeparator);
}

inline bool modifierKey(int key) {
    return key == Qt::Key_Control || key == Qt::Key_Alt || key == Qt::Key_Shift || key == Qt::Key_Meta ||
        key == Qt::Key_AltGr;
}
}
