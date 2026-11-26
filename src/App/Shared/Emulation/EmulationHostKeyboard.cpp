#include "Shared/Emulation/EmulationHostKeyboard.h"
#include "Shared/Emulation/EmulationPhysicalShortcuts.h"
#include <QKeyEvent>
#include <QMetaEnum>
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <utility>

#ifdef Q_OS_WIN
#include <QtCore/qt_windows.h>
#endif

namespace RetroMediaLab {
void EmulationHostKeyboard::applyMachineBindings(Emulation::InputSnapshot& snapshot,
    const QList<Emulation::RawControllerState>& rawControllers, Qt::MouseButtons buttons, const QPoint& wheel,
    const QHash<QString, QString>& bindings, const std::function<bool(const QString&)>& reserved) {
    namespace Protocol = Emulation::HostProtocol;
    namespace Processing = Emulation::InputProcessing;
    namespace Physical = EmulationPhysicalShortcuts;
    Protocol::CoreInput sources;
    sources.keys = snapshot.keys;
    sources.pointer.left = buttons.testFlag(Qt::LeftButton);
    sources.pointer.right = buttons.testFlag(Qt::RightButton);
    sources.pointer.middle = buttons.testFlag(Qt::MiddleButton);
    sources.pointer.extendedButton1 = buttons.testFlag(Qt::XButton1);
    sources.pointer.extendedButton2 = buttons.testFlag(Qt::XButton2);
    sources.pointer.wheel = wheel.y();
    sources.pointer.horizontalWheel = wheel.x();
    const auto axis = [](float value) {
        constexpr auto maximum = std::numeric_limits<qint16>::max();
        return std::isfinite(value) ? static_cast<qint16>(std::lround(std::clamp(value, -1.0f, 1.0f) * maximum))
                                    : qint16{};
    };
    for (qsizetype i = 0; i < snapshot.controllers.size() && i < rawControllers.size(); ++i) {
        const auto& controller = snapshot.controllers[i];
        const auto& raw = rawControllers[i];
        Protocol::CoreController source;
        for (const auto& mapping : Physical::Controls::GamepadButtons) {
            if ((controller.buttons & mapping.button) == 0) continue;
            const auto index = Processing::controllerButtonIndex(QString(mapping.name));
            if (index >= 0) source.buttons |= quint32{1} << index;
        }
        using enum Physical::Controls::Axis;
        source.leftX = axis(controller.axes.value(LeftX));
        source.leftY = axis(controller.axes.value(LeftY));
        source.rightX = axis(controller.axes.value(RightX));
        source.rightY = axis(controller.axes.value(RightY));
        source.leftTrigger = axis(controller.axes.value(LeftTrigger));
        source.rightTrigger = axis(controller.axes.value(RightTrigger));
        source.deviceId = raw.deviceId;
        source.controls = raw.controls;
        sources.controllers.append(std::move(source));
    }
    constexpr float PressedThreshold = 0.5f;
    const auto keys = QMetaEnum::fromType<Emulation::EmulationKey>();
    for (auto binding = bindings.cbegin(); binding != bindings.cend(); ++binding) {
        bool valid = false;
        const int target = keys.keyToValue(binding.key().toLatin1().constData(), &valid);
        if (!valid) continue;
        snapshot.hostCharacters.remove(target);
        if (binding->contains(Physical::Separator) && !reserved(*binding) &&
            Processing::sourcePressure(*binding, sources) > PressedThreshold)
            snapshot.keys.insert(target);
    }
}

namespace {
using Key = Emulation::EmulationKey;

constexpr std::array FunctionKeys{
    Key::F1, Key::F2, Key::F3, Key::F4, Key::F5, Key::F6, Key::F7, Key::F8,
    Key::F9, Key::F10, Key::F11, Key::F12, Key::F13, Key::F14, Key::F15, Key::F16,
    Key::F17, Key::F18, Key::F19, Key::F20, Key::F21, Key::F22, Key::F23, Key::F24
};
constexpr std::pair<int, Key> HostKeys[]{
    {Qt::Key_Backspace, Key::Backspace},
    {Qt::Key_Tab, Key::Tab},
    {Qt::Key_Backtab, Key::OemBackTab},
    {Qt::Key_Return, Key::Return},
    {Qt::Key_Enter, Key::Return},
    {Qt::Key_Escape, Key::Escape},
    {Qt::Key_Space, Key::Space},
    {Qt::Key_Left, Key::Left},
    {Qt::Key_Right, Key::Right},
    {Qt::Key_Up, Key::Up},
    {Qt::Key_Down, Key::Down},
    {Qt::Key_Shift, Key::LeftShift},
    {Qt::Key_Control, Key::LeftControl},
    {Qt::Key_Alt, Key::LeftAlt},
    {Qt::Key_AltGr, Key::RightAlt},
    {Qt::Key_Meta, Key::LeftWindows},
    {Qt::Key_Super_L, Key::LeftWindows},
    {Qt::Key_Super_R, Key::RightWindows},
    {Qt::Key_Delete, Key::Delete},
    {Qt::Key_Insert, Key::Insert},
    {Qt::Key_Home, Key::Home},
    {Qt::Key_End, Key::End},
    {Qt::Key_PageUp, Key::PageUp},
    {Qt::Key_PageDown, Key::PageDown},
    {Qt::Key_CapsLock, Key::CapsLock},
    {Qt::Key_Help, Key::Help},
    {Qt::Key_Pause, Key::Break},
    {Qt::Key_Comma, Key::Comma},
    {Qt::Key_Period, Key::Period},
    {Qt::Key_Slash, Key::Slash},
    {Qt::Key_Backslash, Key::Backslash},
    {Qt::Key_Minus, Key::Minus},
    {Qt::Key_Equal, Key::Equals},
    {Qt::Key_Plus, Key::Equals},
    {Qt::Key_Semicolon, Key::Semicolon},
    {Qt::Key_Apostrophe, Key::Quote},
    {Qt::Key_BracketLeft, Key::LeftBracket},
    {Qt::Key_BracketRight, Key::RightBracket},
    {Qt::Key_QuoteLeft, Key::Backquote},
    {Qt::Key_Cancel, Key::Cancel},
    {Qt::Key_Clear, Key::Clear},
    {Qt::Key_Select, Key::Select},
    {Qt::Key_Execute, Key::Execute},
    {Qt::Key_Print, Key::PrintScreen},
    {Qt::Key_Menu, Key::Applications},
    {Qt::Key_Sleep, Key::Sleep},
    {Qt::Key_NumLock, Key::NumLock},
    {Qt::Key_ScrollLock, Key::ScrollLock},
    {Qt::Key_Hangul_Hanja, Key::HanjaMode},
    {Qt::Key_Muhenkan, Key::ImeNonConvert},
    {Qt::Key_Henkan, Key::ImeConvert},
    {Qt::Key_Hiragana_Katakana, Key::KanaMode},
    {Qt::Key_Mode_switch, Key::ImeModeChange},
    {Qt::Key_Back, Key::BrowserBack},
    {Qt::Key_Forward, Key::BrowserForward},
    {Qt::Key_Refresh, Key::BrowserRefresh},
    {Qt::Key_Stop, Key::BrowserStop},
    {Qt::Key_Search, Key::BrowserSearch},
    {Qt::Key_Favorites, Key::BrowserFavorites},
    {Qt::Key_HomePage, Key::BrowserHome},
    {Qt::Key_VolumeMute, Key::VolumeMute},
    {Qt::Key_VolumeDown, Key::VolumeDown},
    {Qt::Key_VolumeUp, Key::VolumeUp},
    {Qt::Key_MediaNext, Key::MediaNextTrack},
    {Qt::Key_MediaPrevious, Key::MediaPreviousTrack},
    {Qt::Key_MediaStop, Key::MediaStop},
    {Qt::Key_MediaTogglePlayPause, Key::MediaPlayPause},
    {Qt::Key_MediaPlay, Key::Play},
    {Qt::Key_LaunchMail, Key::LaunchMail},
    {Qt::Key_LaunchMedia, Key::SelectMedia},
    {Qt::Key_Launch0, Key::LaunchApplication1},
    {Qt::Key_Launch1, Key::LaunchApplication2},
    {Qt::Key_Zoom, Key::Zoom},
};

#ifdef Q_OS_WIN
constexpr quint32 ExtendedScan = 0x100, RightShiftScan = 0x36, ScanByteMask = 0xff;
constexpr quint32 E0ScanPrefix = 0xe000;
constexpr quint32 AltGrControlIntervalMilliseconds = 50;
constexpr std::pair<quint32, Key> WindowsKeys[]{
    {VK_LSHIFT, Key::LeftShift},
    {VK_RSHIFT, Key::RightShift},
    {VK_LCONTROL, Key::LeftControl},
    {VK_RCONTROL, Key::RightControl},
    {VK_LMENU, Key::LeftAlt},
    {VK_RMENU, Key::RightAlt},
    {VK_LWIN, Key::LeftWindows},
    {VK_RWIN, Key::RightWindows},
    {VK_NUMPAD0, Key::Numpad0},
    {VK_NUMPAD1, Key::Numpad1},
    {VK_NUMPAD2, Key::Numpad2},
    {VK_NUMPAD3, Key::Numpad3},
    {VK_NUMPAD4, Key::Numpad4},
    {VK_NUMPAD5, Key::Numpad5},
    {VK_NUMPAD6, Key::Numpad6},
    {VK_NUMPAD7, Key::Numpad7},
    {VK_NUMPAD8, Key::Numpad8},
    {VK_NUMPAD9, Key::Numpad9},
    {VK_DECIMAL, Key::NumpadPeriod},
    {VK_DIVIDE, Key::NumpadDivide},
    {VK_MULTIPLY, Key::NumpadMultiply},
    {VK_SUBTRACT, Key::NumpadMinus},
    {VK_ADD, Key::NumpadPlus},
    {VK_SEPARATOR, Key::NumpadSeparator},
    {VK_OEM_COMMA, Key::Comma},
    {VK_OEM_PERIOD, Key::Period},
    {VK_OEM_2, Key::Slash},
    {VK_OEM_MINUS, Key::Minus},
    {VK_OEM_PLUS, Key::Equals},
    {VK_OEM_1, Key::Semicolon},
    {VK_OEM_7, Key::Quote},
    {VK_OEM_4, Key::LeftBracket},
    {VK_OEM_6, Key::RightBracket},
    {VK_OEM_5, Key::Backslash},
    {VK_OEM_3, Key::Backquote},
    {VK_OEM_8, Key::Oem8},
    {VK_OEM_102, Key::Oem102},
    {VK_KANA, Key::KanaMode},
    {VK_JUNJA, Key::JunjaMode},
    {VK_FINAL, Key::FinalMode},
    {VK_HANJA, Key::HanjaMode},
    {VK_CONVERT, Key::ImeConvert},
    {VK_NONCONVERT, Key::ImeNonConvert},
    {VK_ACCEPT, Key::ImeAccept},
    {VK_MODECHANGE, Key::ImeModeChange},
    {VK_ATTN, Key::Attn},
    {VK_CRSEL, Key::CrSel},
    {VK_EXSEL, Key::ExSel},
    {VK_EREOF, Key::EraseEof},
    {VK_PLAY, Key::Play},
    {VK_ZOOM, Key::Zoom},
    {VK_PA1, Key::Pa1},
    {VK_OEM_CLEAR, Key::OemClear},
    {VK_PRINT, Key::Print},
    {VK_SNAPSHOT, Key::PrintScreen},
    {VK_MEDIA_PLAY_PAUSE, Key::MediaPlayPause},
};

bool isExtended(quint32 scan) {
    return (scan & ExtendedScan) != 0 || (scan & E0ScanPrefix) == E0ScanPrefix;
}

char32_t windowsCharacter(quint32 vk, const std::array<BYTE, 256>& state, HKL layout) {
    constexpr auto PreserveDeadKeyState = 4;
    constexpr auto TextCapacity = 8;
    if (!(vk == VK_SPACE || (vk >= '0' && vk <= 'Z') ||
        (vk >= VK_NUMPAD0 && vk <= VK_DIVIDE) || (vk >= VK_OEM_1 && vk <= VK_OEM_102))) return 0;
    wchar_t text[TextCapacity]{};
    const auto scan = MapVirtualKeyExW(vk, MAPVK_VK_TO_VSC, layout);
    const auto count = ToUnicodeEx(vk, scan, state.data(), text, TextCapacity, PreserveDeadKeyState, layout);
    return count == 1 || count == -1 ? static_cast<char32_t>(text[0]) : 0;
}
#endif

}

Key EmulationHostKeyboard::keyFor(const QKeyEvent& event) {
    const int key = event.key();
    const auto keypad = event.modifiers().testFlag(Qt::KeypadModifier);
    if (key >= Qt::Key_A && key <= Qt::Key_Z)
        return static_cast<Key>(static_cast<int>(Key::A) + key - Qt::Key_A);
    if (key >= Qt::Key_0 && key <= Qt::Key_9)
        return static_cast<Key>(static_cast<int>(keypad ? Key::Numpad0 : Key::D0) + key - Qt::Key_0);
    if (key >= Qt::Key_F1 && key <= Qt::Key_F24)
        return FunctionKeys[static_cast<size_t>(key - Qt::Key_F1)];
    if (keypad) {
        switch (key) {
        case Qt::Key_Enter: case Qt::Key_Return: return Key::NumpadEnter;
        case Qt::Key_Period: return Key::NumpadPeriod;
        case Qt::Key_Comma: return Key::NumpadSeparator;
        case Qt::Key_Slash: return Key::NumpadDivide;
        case Qt::Key_Asterisk: return Key::NumpadMultiply;
        case Qt::Key_Minus: return Key::NumpadMinus;
        case Qt::Key_Plus: return Key::NumpadPlus;
        default: break;
        }
    }
#ifdef Q_OS_WIN
    const auto native = event.nativeVirtualKey();
    const auto scan = event.nativeScanCode();
    if (native >= '0' && native <= '9') return static_cast<Key>(static_cast<quint32>(Key::D0) + native - '0');
    if (native >= 'A' && native <= 'Z') return static_cast<Key>(static_cast<quint32>(Key::A) + native - 'A');
    if (native == VK_RETURN && isExtended(scan)) return Key::NumpadEnter;
    if (native == VK_SHIFT && (scan & ScanByteMask) == RightShiftScan) return Key::RightShift;
    if (native == VK_CONTROL && isExtended(scan)) return Key::RightControl;
    if (native == VK_MENU && isExtended(scan)) return Key::RightAlt;
    for (const auto& [code, mapped] : WindowsKeys)
        if (code == native) return mapped;
#endif
    for (const auto& [code, mapped] : HostKeys)
        if (code == key) return mapped;
    return Key::Unknown;
}

QKeyCombination EmulationHostKeyboard::hostCombination(QStringView bindingKey) {
    const auto meta = QMetaEnum::fromType<Key>();
    Key key = Key::Unknown;
    for (int i = 0; i < meta.keyCount(); ++i) {
        if (bindingKey.compare(QLatin1String(meta.key(i)), Qt::CaseInsensitive) == 0) {
            key = static_cast<Key>(meta.value(i));
            break;
        }
    }
    if (key >= Key::A && key <= Key::Z)
        return Qt::Key(Qt::Key_A + int(key) - int(Key::A));
    if (key >= Key::D0 && key <= Key::D9)
        return Qt::Key(Qt::Key_0 + int(key) - int(Key::D0));
    if (key >= Key::Numpad0 && key <= Key::Numpad9)
        return QKeyCombination(Qt::KeypadModifier, Qt::Key(Qt::Key_0 + int(key) - int(Key::Numpad0)));
    if (const auto function = std::ranges::find(FunctionKeys, key); function != FunctionKeys.end())
        return Qt::Key(Qt::Key_F1 + (function - FunctionKeys.begin()));
    const auto keypad = [](Qt::Key code) { return QKeyCombination(Qt::KeypadModifier, code); };
    switch (key) {
    case Key::NumpadEnter: return keypad(Qt::Key_Enter);
    case Key::NumpadPeriod: return keypad(Qt::Key_Period);
    case Key::NumpadSeparator: return keypad(Qt::Key_Comma);
    case Key::NumpadDivide: return keypad(Qt::Key_Slash);
    case Key::NumpadMultiply: return keypad(Qt::Key_Asterisk);
    case Key::NumpadMinus: return keypad(Qt::Key_Minus);
    case Key::NumpadPlus: return keypad(Qt::Key_Plus);
    case Key::LeftControl: case Key::RightControl: return QKeyCombination(Qt::ControlModifier, Qt::Key_Control);
    case Key::LeftShift: case Key::RightShift: return QKeyCombination(Qt::ShiftModifier, Qt::Key_Shift);
    case Key::LeftAlt: return QKeyCombination(Qt::AltModifier, Qt::Key_Alt);
    case Key::RightAlt: return QKeyCombination(Qt::AltModifier, Qt::Key_AltGr);
    case Key::LeftWindows: case Key::RightWindows: return QKeyCombination(Qt::MetaModifier, Qt::Key_Meta);
    default: break;
    }
    for (const auto& [code, mapped] : HostKeys)
        if (mapped == key) return Qt::Key(code);
    return Qt::Key_unknown;
}

quint64 EmulationHostKeyboard::physicalId(const QKeyEvent& event) {
    constexpr unsigned ByScanCode = 32;
    if (event.nativeScanCode() != 0) return quint64(event.nativeScanCode()) << ByScanCode;
    return static_cast<quint32>(event.key());
}

void EmulationKeyboardModifiers::observe(const QKeyEvent& event, Key key) {
#ifdef Q_OS_WIN
    if (event.isAutoRepeat()) return;
    const bool pressed = event.type() == QEvent::KeyPress;
    if (!pressed && event.type() != QEvent::KeyRelease) return;
    if (key == Key::LeftControl) {
        leftControlTimestamp_.reset();
        if (pressed && event.nativeScanCode() != 0)
            leftControlTimestamp_ = static_cast<quint32>(event.timestamp());
        syntheticLeftControl_ = false;
    } else if (key == Key::RightControl) {
        rightControl_ = pressed;
    } else if (key == Key::RightAlt) {
        if (!rightAlt_ && pressed && event.nativeScanCode() != 0 && leftControlTimestamp_ &&
            static_cast<quint32>(event.timestamp()) - *leftControlTimestamp_ <= AltGrControlIntervalMilliseconds)
            syntheticLeftControl_ = true;
        rightAlt_ = pressed;
    }
#else
    Q_UNUSED(event);
    Q_UNUSED(key);
#endif
}

Qt::KeyboardModifiers EmulationKeyboardModifiers::normalize(Qt::KeyboardModifiers modifiers) const {
    if (syntheticLeftControl_ && !rightControl_) modifiers &= ~Qt::ControlModifier;
    if (rightAlt_) modifiers |= Qt::AltModifier;
    return modifiers;
}

void EmulationKeyboardModifiers::clear() {
    leftControlTimestamp_.reset();
    syntheticLeftControl_ = false;
    rightControl_ = false;
    rightAlt_ = false;
}

bool EmulationHostKeyboard::update(const QKeyEvent& event, Emulation::InputSnapshot& input) {
    if (event.isAutoRepeat()) return false;
    const auto id = physicalId(event);
    if (event.type() == QEvent::KeyPress) {
        const auto key = keyFor(event);
        if (key == Key::Unknown) return false;
        if (!held_.contains(id)) modifiers_.observe(event, key);
        const auto characters = event.text().toUcs4();
        const auto character = characters.size() == 1 ? characters.front() : char32_t{};
        held_.insert(id, {key, event.nativeVirtualKey(), character});
    } else {
        const auto released = held_.find(id);
        if (released == held_.end()) return false;
        modifiers_.observe(event, released->key);
        held_.erase(released);
    }

    input.keys.clear();
    input.hostCharacters.clear();
    for (const auto& held : std::as_const(held_)) input.keys.insert(static_cast<int>(held.key));
    if (modifiers_.syntheticLeftControl()) input.keys.remove(static_cast<int>(Key::LeftControl));
    const auto contains = [&](Key key) { return input.keys.contains(static_cast<int>(key)); };
    const bool altGr = contains(Key::RightAlt);
    if (!altGr && (contains(Key::LeftControl) || contains(Key::RightControl) || contains(Key::LeftAlt))) return true;
#ifdef Q_OS_WIN
    constexpr BYTE Pressed = 0x80;
    std::array<BYTE, 256> state{};
    const bool useLayout = GetKeyboardState(state.data());
    const auto layout = useLayout ? GetKeyboardLayout(0) : nullptr;
    state[VK_SHIFT] = contains(Key::LeftShift) || contains(Key::RightShift) ? Pressed : 0;
    state[VK_CONTROL] = state[VK_MENU] = altGr ? Pressed : 0;
#endif
    for (const auto& held : std::as_const(held_)) {
        auto character = held.character;
#ifdef Q_OS_WIN
        if (useLayout && held.virtualKey != 0) character = windowsCharacter(held.virtualKey, state, layout);
#endif
        if (QChar::isPrint(character)) input.hostCharacters.insert(static_cast<int>(held.key), character);
    }
    return true;
}

void EmulationHostKeyboard::clear() {
    held_.clear();
    modifiers_.clear();
}
}
