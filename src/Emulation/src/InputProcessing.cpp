#include <RetroMediaLab/Emulation/InputProcessing.h>

#include <QMetaEnum>
#include <algorithm>
#include <bit>
#include <cmath>
#include <limits>
#include <string_view>

namespace RetroMediaLab::Emulation::InputProcessing {
namespace {
using namespace HostProtocol;
constexpr int FirstPortNumber = 1;
constexpr quint32 DeviceMask = 0xff, PrimaryPort = 0, PrimaryIndex = 0;
constexpr quint32 Joypad = 1, Mouse = 2, Keyboard = 3, LightGun = 4, Analog = 5, Pointer = 6;
constexpr quint32 LeftAnalog = 0, RightAnalog = 1, ButtonAnalog = 2, AxisX = 0, AxisY = 1;
constexpr quint32 JoypadMask = 256;
constexpr float PressedThreshold = 0.5f, Percentage = 100.0f;
constexpr auto Minimum = std::numeric_limits<qint16>::min();
constexpr auto Maximum = std::numeric_limits<qint16>::max();
constexpr auto ControllerBits = std::numeric_limits<quint32>::digits;
constexpr auto AnalogButtonCount = std::numeric_limits<quint16>::digits;
constexpr int InvalidButton = -1;

enum Button : quint32 {
    B, Y, Select, Start, Up, Down, Left, Right, A, X, L, R, L2, R2, L3, R3,
    Guide, Share, PaddleLeft1, PaddleLeft2, PaddleRight1, PaddleRight2
};
constexpr std::pair<QLatin1StringView, Button> ButtonNames[]{
    {QLatin1String("B"), B},
    {QLatin1String("ButtonB"), B},
    {QLatin1String("Y"), Y},
    {QLatin1String("ButtonY"), Y},
    {QLatin1String("Select"), Select},
    {QLatin1String("View"), Select},
    {QLatin1String("Start"), Start},
    {QLatin1String("Menu"), Start},
    {QLatin1String("Up"), Up},
    {QLatin1String("DPadUp"), Up},
    {QLatin1String("Down"), Down},
    {QLatin1String("DPadDown"), Down},
    {QLatin1String("Left"), Left},
    {QLatin1String("DPadLeft"), Left},
    {QLatin1String("Right"), Right},
    {QLatin1String("DPadRight"), Right},
    {QLatin1String("A"), A},
    {QLatin1String("ButtonA"), A},
    {QLatin1String("X"), X},
    {QLatin1String("ButtonX"), X},
    {QLatin1String("L"), L},
    {QLatin1String("LeftShoulder"), L},
    {QLatin1String("R"), R},
    {QLatin1String("RightShoulder"), R},
    {QLatin1String("L2"), L2},
    {QLatin1String("LeftTrigger"), L2},
    {QLatin1String("R2"), R2},
    {QLatin1String("RightTrigger"), R2},
    {QLatin1String("L3"), L3},
    {QLatin1String("LeftStickClick"), L3},
    {QLatin1String("R3"), R3},
    {QLatin1String("RightStickClick"), R3},
    {QLatin1String("XboxButton"), Guide},
    {QLatin1String("Guide"), Guide},
    {QLatin1String("Share"), Share},
    {QLatin1String("PaddleLeft1"), PaddleLeft1},
    {QLatin1String("PaddleLeft2"), PaddleLeft2},
    {QLatin1String("PaddleRight1"), PaddleRight1},
    {QLatin1String("PaddleRight2"), PaddleRight2},
};
constexpr std::pair<EmulationKey, quint32> NativeKeys[]{
    {EmulationKey::Backspace, 8},
    {EmulationKey::Tab, 9},
    {EmulationKey::Return, 13},
    {EmulationKey::Escape, 27},
    {EmulationKey::Space, 32},
    {EmulationKey::Comma, ','},
    {EmulationKey::Minus, '-'},
    {EmulationKey::Period, '.'},
    {EmulationKey::Slash, '/'},
    {EmulationKey::Semicolon, ';'},
    {EmulationKey::Equals, '='},
    {EmulationKey::LeftBracket, '['},
    {EmulationKey::Backslash, '\\'},
    {EmulationKey::RightBracket, ']'},
    {EmulationKey::Quote, '\''},
    {EmulationKey::Backquote, '`'},
    {EmulationKey::Delete, 127},
    {EmulationKey::Up, 273},
    {EmulationKey::Down, 274},
    {EmulationKey::Right, 275},
    {EmulationKey::Left, 276},
    {EmulationKey::Insert, 277},
    {EmulationKey::Home, 278},
    {EmulationKey::End, 279},
    {EmulationKey::PageUp, 280},
    {EmulationKey::PageDown, 281},
    {EmulationKey::CapsLock, 301},
    {EmulationKey::RightShift, 303},
    {EmulationKey::LeftShift, 304},
    {EmulationKey::RightControl, 305},
    {EmulationKey::LeftControl, 306},
    {EmulationKey::RightAlt, 307},
    {EmulationKey::LeftAlt, 308},
    {EmulationKey::LeftAmiga, 311},
    {EmulationKey::RightAmiga, 312},
    {EmulationKey::Help, 315},
    {EmulationKey::Break, 318},
    {EmulationKey::Undo, 322},
    {EmulationKey::NumpadPeriod, 266},
    {EmulationKey::NumpadDivide, 267},
    {EmulationKey::NumpadMultiply, 268},
    {EmulationKey::NumpadMinus, 269},
    {EmulationKey::NumpadPlus, 270},
    {EmulationKey::NumpadEnter, 271},
};
struct KeyRange { EmulationKey firstKey; quint32 firstCode, count; };
constexpr KeyRange NativeKeyRanges[]{
    {EmulationKey::A, 'a', 26},
    {EmulationKey::D0, '0', 10},
    {EmulationKey::F1, 282, 10},
    {EmulationKey::Numpad0, 256, 10},
};

bool keyPressed(const CoreInput& input, quint32 id) {
    for (const auto& [key, code] : NativeKeys)
        if (id == code && input.keys.contains(static_cast<qint32>(key))) return true;
    for (const auto& [key, code, count] : NativeKeyRanges)
        if (id >= code && id - code < count &&
            input.keys.contains(static_cast<qint32>(key) + static_cast<qint32>(id - code))) return true;
    return false;
}

enum MouseId : quint32 { DeltaX, DeltaY, LeftButton, RightButton, WheelUp, WheelDown,
    MiddleButton, WheelRight, WheelLeft, ExtraButton1, ExtraButton2 };

qint16 mouseState(const CorePointer& pointer, quint32 id) {
    switch (id) {
    case DeltaX: return static_cast<qint16>(std::clamp(pointer.deltaX, qint32(Minimum), qint32(Maximum)));
    case DeltaY: return static_cast<qint16>(std::clamp(pointer.deltaY, qint32(Minimum), qint32(Maximum)));
    case LeftButton: return pointer.left;
    case RightButton: return pointer.right;
    case WheelUp: return pointer.wheel > 0;
    case WheelDown: return pointer.wheel < 0;
    case MiddleButton: return pointer.middle;
    case WheelRight: return pointer.horizontalWheel > 0;
    case WheelLeft: return pointer.horizontalWheel < 0;
    case ExtraButton1: return pointer.extendedButton1;
    case ExtraButton2: return pointer.extendedButton2;
    default: return 0;
    }
}

int projectedButton(QStringView command) {
    if (command == QLatin1String("Fire1")) return A;
    if (command == QLatin1String("Fire2")) return B;
    return controllerButtonIndex(command);
}

bool pressed(float pressure) {
    return std::isfinite(pressure) && pressure > PressedThreshold;
}

quint32 joypadButtons(const CoreController& controller) {
    auto buttons = controller.buttons;
    for (auto control = controller.controls.cbegin(); control != controller.controls.cend(); ++control) {
        const auto bit = projectedButton(control.key());
        if (bit < 0) continue;
        const quint32 mask = quint32(1) << bit;
        if (pressed(control.value())) buttons |= mask;
        else buttons &= ~mask;
    }
    return buttons;
}

bool buttonPressed(quint32 buttons, quint32 id) {
    return id < ControllerBits && (buttons & (quint32(1) << id)) != 0;
}

qint16 analogState(const CoreController& controller, quint32 index, quint32 id) {
    if (index == LeftAnalog) return id == AxisX ? controller.leftX : id == AxisY ? controller.leftY : 0;
    if (index == RightAnalog) return id == AxisX ? controller.rightX : id == AxisY ? controller.rightY : 0;
    if (index != ButtonAnalog || id >= AnalogButtonCount) return 0;
    std::optional<float> pressure;
    for (auto control = controller.controls.cbegin(); control != controller.controls.cend(); ++control)
        if (projectedButton(control.key()) == static_cast<int>(id)) pressure = control.value();
    if (pressure) {
        if (!std::isfinite(*pressure)) return 0;
        const auto scaled = std::clamp(*pressure, 0.0f, 1.0f) * Maximum;
        const auto lower = static_cast<qint16>(scaled);
        const auto fraction = scaled - lower;
        constexpr float Half = 0.5f;
        return static_cast<qint16>(lower + (fraction > Half || (fraction == Half && lower % 2 != 0)));
    }
    const auto trigger = id == L2 ? controller.leftTrigger : id == R2 ? controller.rightTrigger : qint16(0);
    return trigger > 0 ? trigger : buttonPressed(joypadButtons(controller), id) ? Maximum : 0;
}

qint16 pointingState(const CoreController& controller, quint32 device, quint32 id) {
    const bool offscreen = controller.leftX == Minimum || controller.leftX == Maximum ||
        controller.leftY == Minimum || controller.leftY == Maximum;
    const auto buttons = joypadButtons(controller);
    if (device == Pointer) {
        enum PointerId { ScreenX = 0, ScreenY = 1, Pressed = 2, Count = 3, Offscreen = 15 };
        switch (id) {
        case ScreenX: return controller.leftX;
        case ScreenY: return controller.leftY;
        case Pressed: case Count: return buttonPressed(buttons, A);
        case Offscreen: return offscreen;
        default: return 0;
        }
    }
    enum GunId { Trigger = 2, AuxA = 3, AuxB = 4, GunStart = 6, GunSelect = 7, AuxC = 8,
        GunUp = 9, GunDown = 10, GunLeft = 11, GunRight = 12, GunX = 13, GunY = 14, GunOffscreen = 15, Reload = 16 };
    switch (id) {
    case Trigger: return buttonPressed(buttons, B);
    case AuxA: return buttonPressed(buttons, A);
    case AuxB: return buttonPressed(buttons, X);
    case AuxC: return buttonPressed(buttons, Y);
    case GunStart: return buttonPressed(buttons, Start);
    case GunSelect: return buttonPressed(buttons, Select);
    case GunUp: return buttonPressed(buttons, Up);
    case GunDown: return buttonPressed(buttons, Down);
    case GunLeft: return buttonPressed(buttons, Left);
    case GunRight: return buttonPressed(buttons, Right);
    case GunX: return controller.leftX;
    case GunY: return controller.leftY;
    case GunOffscreen: return offscreen;
    case Reload: return pressed(controller.controls.value(QStringLiteral("Reload")));
    default: return 0;
    }
}
}

const ControllerPortSettings* controllerPort(const InputSettings& settings, int index) {
    if (index < 0) return nullptr;
    const auto number = qint64{index} + FirstPortNumber;
    const auto found = std::ranges::find(settings.controllerPorts, number, &ControllerPortSettings::number);
    return found == settings.controllerPorts.cend() ? nullptr : &*found;
}

const QList<InputBindingDefinition>* controllerBindingDefinitions(
    const InputSettings& settings, QStringView peripheral, int index) {
    const auto* port = controllerPort(settings, index);
    if (!port) return nullptr;
    const auto choice = std::ranges::find_if(port->controllerChoices, [peripheral](const auto& candidate) {
        return candidate.id == peripheral;
    });
    if (choice != port->controllerChoices.cend() && choice->bindingDefinitions)
        return &*choice->bindingDefinitions;
    return port->selectedControllerId == peripheral ? &port->bindings.definitions : nullptr;
}

QList<const ControllerChoice*> supportedControllerChoices(const InputSettings& settings,
    std::optional<int> index, bool usb) {
    if (index && *index < 0) return {};
    QList<const ControllerPortSettings*> ports;
    for (const auto& port : settings.controllerPorts) {
        if (port.isUsb != usb) continue;
        if (!usb && index && port.number != qint64{*index} + FirstPortNumber) continue;
        ports.append(&port);
    }

    if (usb && index) {
        if (*index >= ports.size()) return {};
        std::stable_sort(ports.begin(), ports.end(), [](const auto* left, const auto* right) {
            return left->number < right->number;
        });
        ports = {ports.at(*index)};
    }

    QList<const ControllerChoice*> result;
    for (const auto* port : ports)
        for (const auto& choice : port->controllerChoices)
            if (choice.isSupportedByEmulator) result.append(&choice);
    return result;
}

QList<InputActionDefinition> describeInput(const InputSettings& settings, int port, QStringView peripheral) {
    QList<InputActionDefinition> actions;
    if (const auto* definitions = controllerBindingDefinitions(settings, peripheral, port)) {
        actions.reserve(definitions->size());
        for (const auto& definition : *definitions) actions.append({definition, std::nullopt});
    }
    return actions;
}

int controllerButtonIndex(QStringView command) {
    const auto found = std::ranges::find_if(ButtonNames, [&](const auto& entry) {
        return command.compare(entry.first, Qt::CaseInsensitive) == 0;
    });
    return found == std::end(ButtonNames) ? InvalidButton : static_cast<int>(found->second);
}

QHash<QString, QString> resolveBindings(const InputBindingSet& set) {
    QHash<QString, QString> bindings;
    bindings.reserve(set.definitions.size());
    for (const auto& definition : set.definitions)
        bindings.insert(definition.id, set.values.value(definition.id, definition.defaultBinding));
    return bindings;
}

const HostProtocol::CoreController* resolveController(QStringView deviceId,
    const QList<HostProtocol::CoreController>& controllers, qsizetype fallbackIndex) {
    if (deviceId.trimmed().isEmpty())
        return fallbackIndex >= 0 && fallbackIndex < controllers.size() ? &controllers.at(fallbackIndex) : nullptr;
    const auto selected = std::ranges::find_if(controllers, [&](const auto& controller) {
        return deviceId.compare(controller.deviceId, Qt::CaseInsensitive) == 0;
    });
    return selected == controllers.end() ? nullptr : &*selected;
}

float applyDeadZone(float pressure, int percent) {
    if (!std::isfinite(pressure)) return 0.0f;
    return pressure <= static_cast<float>(percent) / Percentage ? 0.0f : std::clamp(pressure, 0.0f, 1.0f);
}

HostProtocol::CoreController applyDeadZones(HostProtocol::CoreController state, int percent) {
    const auto threshold = static_cast<qint64>(Maximum) * percent / static_cast<int>(Percentage);
    for (const auto axis : {&CoreController::leftX, &CoreController::leftY, &CoreController::rightX,
            &CoreController::rightY, &CoreController::leftTrigger, &CoreController::rightTrigger}) {
        if (percent >= Percentage || std::abs(static_cast<int>(state.*axis)) <= threshold) state.*axis = 0;
    }
    return state;
}

namespace {
constexpr QLatin1StringView KeyboardPrefix("Keyboard:"), MousePrefix("Mouse:"), ControllerPrefix("Controller:");
constexpr QLatin1StringView PositiveSuffix("Positive"), NegativeSuffix("Negative"), AxisPrefix("Axis");
constexpr QChar SourceSeparator(u':'), ChordSeparator(u'+');

std::optional<EmulationKey> parseKey(QStringView name) {
    static const auto values = [] {
        QHash<QString, int> names;
        const auto meta = QMetaEnum::fromType<EmulationKey>();
        for (int i = 0; i < meta.keyCount(); ++i)
            names.insert(QString::fromLatin1(meta.key(i)).toCaseFolded(), meta.value(i));
        return names;
    }();
    const auto keyName = name.trimmed().toString().toCaseFolded();
    const auto found = values.constFind(keyName);
    if (found != values.cend()) return static_cast<EmulationKey>(*found);
    bool numeric = false;
    const auto value = keyName.toInt(&numeric);
    if (!numeric) return std::nullopt;
    return static_cast<EmulationKey>(value);
}

constexpr struct ModifierBinding {
    QLatin1StringView name;
    EmulationKey left, right;
} ModifierBindings[]{
    {QLatin1String("Ctrl"), EmulationKey::LeftControl, EmulationKey::RightControl},
    {QLatin1String("Control"), EmulationKey::LeftControl, EmulationKey::RightControl},
    {QLatin1String("Alt"), EmulationKey::LeftAlt, EmulationKey::RightAlt},
    {QLatin1String("Shift"), EmulationKey::LeftShift, EmulationKey::RightShift},
    {QLatin1String("Maj"), EmulationKey::LeftShift, EmulationKey::RightShift},
};
const ModifierBinding* modifierBinding(QStringView name) {
    for (const auto& modifier : ModifierBindings)
        if (name.compare(modifier.name, Qt::CaseInsensitive) == 0) return &modifier;
    return nullptr;
}

std::optional<EmulationKey> bindingKey(QStringView name) {
    if (name == QLatin1String("LeftCtrl")) return EmulationKey::LeftControl;
    if (name == QLatin1String("RightCtrl")) return EmulationKey::RightControl;
    if (name == QLatin1String("Enter")) return EmulationKey::Return;
    if (name == QLatin1String("Back")) return EmulationKey::Backspace;
    if (name == QLatin1String("Decimal")) return EmulationKey::NumpadPeriod;
    const auto key = parseKey(name);
    if (key && *key != EmulationKey::Unknown &&
        QMetaEnum::fromType<EmulationKey>().valueToKey(static_cast<int>(*key))) return key;
    return std::nullopt;
}

bool namedKeyPressed(QStringView name, const QSet<qint32>& keys) {
    const auto has = [&](EmulationKey key) { return keys.contains(static_cast<qint32>(key)); };
    if (const auto* modifier = modifierBinding(name)) return has(modifier->left) || has(modifier->right);
    const auto key = bindingKey(name);
    return key && has(*key);
}

std::optional<MouseId> mouseBinding(QStringView name, Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive) {
    constexpr std::pair<QLatin1StringView, MouseId> bindings[]{
        {QLatin1String("Left"), LeftButton}, {QLatin1String("Right"), RightButton},
        {QLatin1String("Middle"), MiddleButton}, {QLatin1String("XButton1"), ExtraButton1},
        {QLatin1String("XButton2"), ExtraButton2}, {QLatin1String("WheelUp"), WheelUp},
        {QLatin1String("WheelDown"), WheelDown}, {QLatin1String("WheelLeft"), WheelLeft},
        {QLatin1String("WheelRight"), WheelRight},
    };
    for (const auto& [key, id] : bindings)
        if (name.compare(key, sensitivity) == 0) return id;
    return std::nullopt;
}

bool namedMousePressed(QStringView name, const CorePointer& pointer, Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive) {
    const auto id = mouseBinding(name, sensitivity);
    return id && mouseState(pointer, *id) != 0;
}

struct StickAxis { QLatin1StringView negative, positive; qint16 CoreController::*value; };
constexpr StickAxis StickAxes[]{
    {QLatin1String("LeftStickLeft"), QLatin1String("LeftStickRight"), &CoreController::leftX},
    {QLatin1String("LeftStickUp"), QLatin1String("LeftStickDown"), &CoreController::leftY},
    {QLatin1String("RightStickLeft"), QLatin1String("RightStickRight"), &CoreController::rightX},
    {QLatin1String("RightStickUp"), QLatin1String("RightStickDown"), &CoreController::rightY},
};

bool numberedAxis(QStringView name) {
    if (!name.startsWith(AxisPrefix, Qt::CaseInsensitive)) return false;
    bool valid = false;
    const auto index = name.sliced(AxisPrefix.size()).toInt(&valid);
    return valid && index >= 0;
}

QStringView sourceControlName(QStringView source) {
    while (source.endsWith(SourceSeparator)) source.chop(1);
    return source.sliced(source.lastIndexOf(SourceSeparator) + 1);
}

bool isChord(QStringView source) {
    return source.contains(ChordSeparator) && !source.endsWith(ChordSeparator);
}

float controllerSourceValue(QStringView name, const CoreController& controller) {
    const auto bit = controllerButtonIndex(name);
    const auto control = controller.controls.constFind(name.toString());
    if (bit == L2 || bit == R2) {
        if (control != controller.controls.cend()) return applyDeadZone(*control, 0);
        const auto trigger = bit == L2 ? controller.leftTrigger : controller.rightTrigger;
        if (trigger > 0) return static_cast<float>(trigger) / Maximum;
    }
    if (bit != InvalidButton) return buttonPressed(controller.buttons, bit) ? 1.0f : 0.0f;

    const bool positive = name.endsWith(PositiveSuffix, Qt::CaseInsensitive);
    const bool negative = name.endsWith(NegativeSuffix, Qt::CaseInsensitive);
    if (positive || negative) {
        const auto axis = name.chopped(positive ? PositiveSuffix.size() : NegativeSuffix.size());
        const auto value = controller.controls.constFind(axis.toString());
        if (value != controller.controls.cend()) {
            constexpr float AxisRange = 2.0f;
            if (numberedAxis(axis))
                return applyDeadZone(positive ? *value * AxisRange - 1.0f : 1.0f - *value * AxisRange, 0);
            return positive ? *value : -*value;
        }
    }
    if (control != controller.controls.cend()) return *control;

    for (const auto& axis : StickAxes) {
        if (name == axis.negative || name == axis.positive) {
            const auto value = static_cast<float>(controller.*axis.value);
            return name == axis.negative ? -value : value;
        }
    }
    return 0.0f;
}

float controllerSourcePressure(QStringView name, const CoreController& controller) {
    auto value = controllerSourceValue(name, controller);
    const bool rawStick = !controller.controls.contains(name.toString()) &&
        std::ranges::any_of(StickAxes, [name](const auto& axis) {
            return name == axis.negative || name == axis.positive;
        });
    if (rawStick || std::abs(value) > 1.0f) value /= Maximum;
    return applyDeadZone(value, 0);
}

bool controllerSourcePressed(QStringView name, const CoreController& controller) {
    constexpr float PositiveThreshold = 0.75f, NegativeThreshold = -0.25f, AnalogThreshold = 14000.0f;
    const auto value = controllerSourceValue(name, controller);
    if (!std::isfinite(value)) return false;
    if (controllerButtonIndex(name) != InvalidButton) return value > PressedThreshold;
    if (name.endsWith(PositiveSuffix, Qt::CaseInsensitive))
        return value > (numberedAxis(name.chopped(PositiveSuffix.size())) ? PressedThreshold : PositiveThreshold);
    if (name.endsWith(NegativeSuffix, Qt::CaseInsensitive))
        return value > (numberedAxis(name.chopped(NegativeSuffix.size())) ? PressedThreshold : NegativeThreshold);
    if (controller.controls.contains(name.toString())) return value > PressedThreshold;
    return value > AnalogThreshold;
}

qint16 roundAxisEven(float value) {
    const auto truncated = static_cast<qint16>(value);
    const auto fraction = std::abs(value - truncated);
    if (fraction > PressedThreshold || (fraction == PressedThreshold && truncated % 2 != 0))
        return static_cast<qint16>(truncated + (value < 0.0f ? -1 : 1));
    return truncated;
}
float bindingPressure(QStringView source, const CoreInput& input, const CoreController* fallback,
    int deadZone, bool bareIsKeyboard) {
    if (source.trimmed().isEmpty()) return 0.0f;
    if (isChord(source)) {
        float pressure = 1.0f;
        for (const auto part : source.split(ChordSeparator))
            pressure = std::min(pressure, bindingPressure(part.trimmed(), input, fallback, deadZone, true));
        return pressure;
    }
    if (source.startsWith(KeyboardPrefix, Qt::CaseInsensitive))
        return namedKeyPressed(source.sliced(KeyboardPrefix.size()), input.keys) ? 1.0f : 0.0f;
    if (source.startsWith(MousePrefix, Qt::CaseInsensitive))
        return namedMousePressed(source.sliced(MousePrefix.size()), input.pointer) ? 1.0f : 0.0f;
    const auto separator = source.lastIndexOf(SourceSeparator);
    if (separator < 0 && bareIsKeyboard) return namedKeyPressed(source, input.keys) ? 1.0f : 0.0f;
    const auto* selected = fallback;
    if (source.startsWith(ControllerPrefix, Qt::CaseInsensitive) && separator > ControllerPrefix.size())
        selected = resolveController(source.sliced(ControllerPrefix.size(), separator - ControllerPrefix.size()), input.controllers);
    if (!selected) return 0.0f;
    return applyDeadZone(controllerSourcePressure(sourceControlName(source), *selected), deadZone);
}
}

bool isValidBinding(QStringView binding) {
    if (binding.trimmed().isEmpty()) return true;
    for (const auto token : binding.split(ChordSeparator)) {
        const auto part = token.trimmed();
        if (part.startsWith(MousePrefix, Qt::CaseInsensitive)) {
            if (!mouseBinding(part.sliced(MousePrefix.size()))) return false;
        } else if (part.startsWith(ControllerPrefix, Qt::CaseInsensitive)) {
            if (part.size() <= ControllerPrefix.size() || part.sliced(part.lastIndexOf(SourceSeparator) + 1).trimmed().isEmpty())
                return false;
        } else {
            const auto key = part.startsWith(KeyboardPrefix, Qt::CaseInsensitive) ? part.sliced(KeyboardPrefix.size()) : part;
            if (!modifierBinding(key) && !bindingKey(key)) return false;
        }
    }
    return true;
}

std::optional<QHash<QString, QString>> resolveKeyboardBindings(const InputBindingSet& set) {
    auto bindings = resolveBindings(set);
    if (bindings.size() != set.definitions.size()) return std::nullopt;
    for (auto value = set.values.cbegin(); value != set.values.cend(); ++value)
        if (!bindings.contains(value.key()) || !isValidBinding(value.value())) return std::nullopt;
    return bindings;
}

bool hasInvalidControllerPortNumbers(const InputSettings& settings, int expectedCount) {
    if (settings.controllerPorts.size() != expectedCount) return true;
    QSet<int> seen;
    seen.reserve(expectedCount);
    for (const auto& port : settings.controllerPorts) {
        if (port.number < FirstPortNumber || port.number > expectedCount || seen.contains(port.number)) return true;
        seen.insert(port.number);
    }
    return false;
}

float sourcePressure(QStringView source, const CoreInput& input, const CoreController* fallback, int deadZone) {
    return bindingPressure(source, input, fallback, deadZone, true);
}

qint16 mapControllerAxis(const QHash<QString, QString>& mappings, const QString& positiveCommand,
    const QString& negativeCommand, const CoreInput& input, const CoreController* fallback, int deadZone) {
    const auto value = bindingPressure(mappings.value(positiveCommand), input, fallback, deadZone, false)
        - bindingPressure(mappings.value(negativeCommand), input, fallback, deadZone, false);
    return roundAxisEven(value >= 0.0f ? value * Maximum : -value * Minimum);
}

qint16 mapControllerPressure(const QHash<QString, QString>& mappings, const QString& command,
    const CoreInput& input, const CoreController* fallback, int deadZone) {
    return roundAxisEven(bindingPressure(mappings.value(command), input, fallback, deadZone, false) * Maximum);
}

bool isMappedInputPressed(QStringView mapping, const CoreInput& input, const CoreController* controller) {
    if (isChord(mapping)) return sourcePressure(mapping, input, controller) > PressedThreshold;
    if (mapping.startsWith(KeyboardPrefix, Qt::CaseInsensitive)) {
        const auto name = mapping.sliced(KeyboardPrefix.size()).trimmed();
        bool numeric = false;
        const auto code = name.toInt(&numeric);
        if (numeric) return input.keys.contains(code);
        const auto key = QMetaEnum::fromType<EmulationKey>().keyToValue(name.toLatin1().constData(), &numeric);
        return numeric && input.keys.contains(key);
    }
    if (mapping.startsWith(MousePrefix, Qt::CaseInsensitive))
        return namedMousePressed(mapping.sliced(MousePrefix.size()), input.pointer, Qt::CaseSensitive);
    return controller && mapping.startsWith(ControllerPrefix, Qt::CaseInsensitive)
        && controllerSourcePressed(sourceControlName(mapping), *controller);
}

float mappedInputMagnitude(QStringView mapping, const CoreInput& input, const CoreController* controller, int deadZone) {
    if (isChord(mapping)) return applyDeadZone(sourcePressure(mapping, input, controller), deadZone);
    if (!mapping.startsWith(ControllerPrefix, Qt::CaseInsensitive))
        return isMappedInputPressed(mapping, input, controller) ? 1.0f : 0.0f;
    return controller ? applyDeadZone(controllerSourcePressure(sourceControlName(mapping), *controller), deadZone) : 0.0f;
}

std::optional<QString> oppositeControllerAxisSource(QStringView source) {
    if (isChord(source)) {
        QStringList parts;
        bool inverted = false;
        for (const auto part : source.split(ChordSeparator)) {
            const auto trimmed = part.trimmed();
            if (auto opposite = oppositeControllerAxisSource(trimmed)) {
                parts.append(std::move(*opposite));
                inverted = true;
            } else {
                parts.append(trimmed.toString());
            }
        }
        return inverted ? std::optional<QString>(parts.join(ChordSeparator)) : std::nullopt;
    }
    if (!source.startsWith(ControllerPrefix, Qt::CaseInsensitive)) return std::nullopt;
    const auto commandStart = source.lastIndexOf(SourceSeparator) + 1;
    const auto command = source.sliced(commandStart);
    for (const auto& axis : StickAxes) {
        if (command == axis.negative || command == axis.positive)
            return source.first(commandStart).toString() + (command == axis.negative ? axis.positive : axis.negative);
    }
    for (const auto suffix : {PositiveSuffix, NegativeSuffix}) {
        if (!command.endsWith(suffix, Qt::CaseInsensitive) || !numberedAxis(command.chopped(suffix.size()))) continue;
        return source.chopped(suffix.size()).toString() + (suffix == PositiveSuffix ? NegativeSuffix : PositiveSuffix);
    }
    return std::nullopt;
}

QSet<EmulationKey> mapKeyboard(const CoreInput& input, const InputBindingSet& set) {
    QHash<EmulationKey, EmulationKey> simple;
    QSet<EmulationKey> additional;
    const auto* controller = input.controllers.isEmpty() ? nullptr : &input.controllers.first();
    for (const auto& definition : set.definitions) {
        const auto assignment = set.values.constFind(definition.id);
        const QStringView source = assignment == set.values.cend() ? definition.defaultBinding : *assignment;
        const auto target = parseKey(definition.id);
        const auto key = parseKey(source.startsWith(KeyboardPrefix, Qt::CaseInsensitive)
            ? source.sliced(KeyboardPrefix.size()) : source);
        if (key) {
            if (!simple.contains(*key)) simple.insert(*key, target.value_or(*key));
        } else if (target && sourcePressure(source, input, controller) > PressedThreshold) {
            additional.insert(*target);
        }
    }

    auto result = std::move(additional);
    result.reserve(result.size() + input.keys.size());
    for (const auto code : input.keys) {
        const auto key = static_cast<EmulationKey>(code);
        result.insert(simple.value(key, key));
    }
    return result;
}

namespace {
constexpr quint16 NativeShift = 1, NativeControl = 2, NativeAlt = 4, NoModifiers = 0;
constexpr quint32 NoCharacter = 0;
constexpr char UnshiftedCharacters[] = "0123456789-=[]\\;',./`";
constexpr char ShiftedCharacters[] = ")!@#$%^&*(_+{}|:\"<>?~";
static_assert(sizeof(UnshiftedCharacters) == sizeof(ShiftedCharacters));

bool shiftPressed(const QSet<EmulationKey>& keys) {
    return keys.contains(EmulationKey::LeftShift) || keys.contains(EmulationKey::RightShift);
}
}

const QHash<EmulationKey, quint32>& nativeKeyboardCodes() {
    static const auto codes = [] {
        QHash<EmulationKey, quint32> map;
        for (const auto& [key, code] : NativeKeys) map.insert(key, code);
        for (const auto& [firstKey, firstCode, count] : NativeKeyRanges)
            for (quint32 offset = 0; offset < count; ++offset)
                map.insert(static_cast<EmulationKey>(static_cast<int>(firstKey) + static_cast<int>(offset)), firstCode + offset);
        return map;
    }();
    return codes;
}

quint16 nativeKeyboardModifiers(const QSet<EmulationKey>& keys) {
    const bool control = keys.contains(EmulationKey::LeftControl) || keys.contains(EmulationKey::RightControl);
    const bool alt = keys.contains(EmulationKey::LeftAlt) || keys.contains(EmulationKey::RightAlt);
    return (shiftPressed(keys) ? NativeShift : NoModifiers)
        | (control ? NativeControl : NoModifiers) | (alt ? NativeAlt : NoModifiers);
}

quint32 nativeKeyboardCharacter(quint32 code, const QSet<EmulationKey>& keys) {
    const bool shift = shiftPressed(keys);
    if (code >= 'a' && code <= 'z')
        return shift != keys.contains(EmulationKey::CapsLock) ? code - ('a' - 'A') : code;
    if (code < ' ' || code > '~') return NoCharacter;
    if (shift) {
        constexpr std::string_view unshifted(UnshiftedCharacters);
        const auto index = unshifted.find(static_cast<char>(code));
        if (index != unshifted.npos) return static_cast<quint32>(ShiftedCharacters[index]);
    }
    return code;
}

QList<NativeKeyboardEvent> nativeKeyboardTransitions(const QSet<EmulationKey>& keys,
    const QSet<EmulationKey>& previous, const QHash<EmulationKey, quint32>& codes,
    const QSet<EmulationKey>& modifierKeys) {
    QList<NativeKeyboardEvent> events;
    const auto modifiers = nativeKeyboardModifiers(keys);
    const auto append = [&](const auto& from, const auto& to, bool pressed, bool modifier) {
        for (const auto key : from) {
            if (to.contains(key) || modifierKeys.contains(key) != modifier) continue;
            const auto code = codes.constFind(key);
            if (code == codes.cend()) continue;
            events.append({pressed, *code, pressed ? nativeKeyboardCharacter(*code, keys) : NoCharacter, modifiers});
        }
    };
    append(previous, keys, false, false);
    append(previous, keys, false, true);
    append(keys, previous, true, true);
    append(keys, previous, true, false);
    return events;
}

qint16 normalizedAxis(float value) {
    if (std::isnan(value)) return {};
    return static_cast<qint16>(std::clamp(value, -1.0f, 1.0f) * Maximum);
}

qint16 nativeInputState(const HostProtocol::CoreInput& input, quint32 port, quint32 device, quint32 index, quint32 id) {
    const auto type = device & DeviceMask;
    if (type == Mouse) return port == PrimaryPort && index == PrimaryIndex ? mouseState(input.pointer, id) : 0;
    if (type == Keyboard) return keyPressed(input, id);
    if (static_cast<quint64>(port) >= static_cast<quint64>(input.controllers.size())) return 0;
    const auto& state = input.controllers.at(port);
    switch (type) {
    case Joypad: {
        const auto buttons = joypadButtons(state);
        return id == JoypadMask ? std::bit_cast<qint16>(static_cast<quint16>(buttons)) : buttonPressed(buttons, id);
    }
    case Analog: return analogState(state, index, id);
    case LightGun: case Pointer: return index == PrimaryIndex ? pointingState(state, type, id) : 0;
    default: return 0;
    }
}
}
