#pragma once

#include <QJsonObject>
#include <QKeySequence>
#include <QStringList>
#include <Qt>

namespace RetroMediaLab::EmulationKeyboardShortcuts {
struct Binding {
    const char* id;
    QKeyCombination defaultKey;
    bool hasAlternative = false;
};

inline constexpr auto Modifiers = Qt::ControlModifier | Qt::ShiftModifier;
inline constexpr auto KeypadModifiers = Modifiers | Qt::KeypadModifier;
inline constexpr auto AlternativeSuffix = "-alternative";
inline constexpr Binding Power{"power", Modifiers | Qt::Key_F1};
inline constexpr Binding PauseResume{"pause-resume", Modifiers | Qt::Key_F2};
inline constexpr Binding SoftReset{"soft-reset", Modifiers | Qt::Key_F3};
inline constexpr Binding HardReset{"hard-reset", Modifiers | Qt::Key_F4};
inline constexpr Binding OpenSettings{"open-machine-settings", Modifiers | Qt::Key_F5};
inline constexpr Binding QuickSave{"quick-save", Modifiers | Qt::Key_F6};
inline constexpr Binding QuickLoad{"quick-load", Modifiers | Qt::Key_F7};
inline constexpr Binding Screenshot{"screenshot", Modifiers | Qt::Key_F9};
inline constexpr Binding ToggleFullscreen{"toggle-fullscreen", Modifiers | Qt::Key_F11};
inline constexpr Binding ReleaseMouse{"release-mouse", Modifiers | Qt::Key_F12};
inline constexpr Binding ToggleMute{"toggle-mute", Modifiers | Qt::Key_M};
inline constexpr Binding VolumeDown{"volume-down", Modifiers | Qt::Key_Left};
inline constexpr Binding VolumeUp{"volume-up", Modifiers | Qt::Key_Right};
inline constexpr Binding OsdWithPause{"osd-with-pause", KeypadModifiers | Qt::Key_Slash, true};
inline constexpr Binding OsdWithoutPause{"osd-without-pause", KeypadModifiers | Qt::Key_Asterisk, true};
inline constexpr Binding OsdStatistics{"osd-statistics", KeypadModifiers | Qt::Key_Minus};
inline constexpr Binding OsdUp{"osd-up", Qt::Key_Up, true};
inline constexpr Binding OsdDown{"osd-down", Qt::Key_Down, true};
inline constexpr Binding OsdLeft{"osd-left", Qt::Key_Left, true};
inline constexpr Binding OsdRight{"osd-right", Qt::Key_Right, true};
inline constexpr Binding OsdConfirm{"osd-confirm", Qt::Key_Return, true};
inline constexpr Binding OsdCancel{"osd-cancel", Qt::Key_Escape, true};

inline constexpr bool isOsdToggle(const Binding& binding) {
    return &binding == &OsdWithPause || &binding == &OsdWithoutPause || &binding == &OsdStatistics;
}

inline constexpr bool isVolumeShortcut(const Binding& binding) {
    return &binding == &VolumeDown || &binding == &VolumeUp;
}

inline constexpr bool isOsdDirection(const Binding& binding) {
    return &binding == &OsdUp || &binding == &OsdDown || &binding == &OsdLeft || &binding == &OsdRight;
}

inline QKeySequence parse(QString stored) {
    constexpr auto KeyboardPrefix = "keyboard:";
    constexpr auto Plus = QLatin1Char('+');
    auto parts = stored.split(Plus);
    for (auto& part : parts) {
        part = part.trimmed();
        if (part.startsWith(QLatin1String(KeyboardPrefix), Qt::CaseInsensitive))
            part.remove(0, qsizetype(sizeof("keyboard:") - 1));
        if (part.contains(QLatin1Char(':'))) return {};
        if (part.compare(QLatin1String("Maj"), Qt::CaseInsensitive) == 0) part = QLatin1String("Shift");
        if (part.compare(QLatin1String("Win"), Qt::CaseInsensitive) == 0) part = QLatin1String("Meta");
    }
    auto& key = parts.back();
    if (key.compare(QLatin1String("Divide"), Qt::CaseInsensitive) == 0) key = QLatin1String("Num+/");
    else if (key.compare(QLatin1String("Multiply"), Qt::CaseInsensitive) == 0) key = QLatin1String("Num+*");
    else if (key.compare(QLatin1String("Subtract"), Qt::CaseInsensitive) == 0) key = QLatin1String("Num+-");
    else if (key.compare(QLatin1String("Return"), Qt::CaseInsensitive) == 0) key = QLatin1String("Return");
    const auto sequence = QKeySequence::fromString(parts.join(Plus), QKeySequence::PortableText);
    for (qsizetype index = 0; index < sequence.count(); ++index)
        if (sequence[index].key() == Qt::Key_unknown) return {};
    return sequence;
}

inline bool reserved(const QKeySequence& sequence) {
#ifdef Q_OS_WIN
    for (qsizetype index = 0; index < sequence.count(); ++index) {
        const auto combination = sequence[index];
        const auto modifiers = combination.keyboardModifiers();
        const auto key = combination.key();
        if (modifiers.testFlag(Qt::MetaModifier) || key == Qt::Key_Meta || key == Qt::Key_Super_L ||
            key == Qt::Key_Super_R || key == Qt::Key_Sleep)
            return true;
        const bool control = modifiers.testFlag(Qt::ControlModifier);
        const bool alt = modifiers.testFlag(Qt::AltModifier);
        if (alt && (key == Qt::Key_Tab || key == Qt::Key_Backtab || key == Qt::Key_Escape || key == Qt::Key_F4))
            return true;
        if (control && (key == Qt::Key_Escape || (alt && key == Qt::Key_Delete))) return true;
    }
#else
    Q_UNUSED(sequence);
#endif
    return false;
}

inline bool conflicts(const QKeySequence& first, const QKeySequence& second) {
    return !first.isEmpty() && !second.isEmpty() &&
        (first.matches(second) != QKeySequence::NoMatch || second.matches(first) != QKeySequence::NoMatch);
}

inline QList<QKeySequence> sequences(const QJsonObject& configured, const Binding& binding) {
    const auto key = QLatin1String(binding.id);
    QList<QKeySequence> active;
    const auto append = [&](const QKeySequence& sequence) {
        if (!sequence.isEmpty() && !reserved(sequence) && !active.contains(sequence)) active.append(sequence);
    };
    const auto primary = configured.value(key);
    if (primary.isUndefined()) {
        append(QKeySequence(binding.defaultKey));
        if (binding.defaultKey.key() == Qt::Key_Return) append(QKeySequence(Qt::Key_Enter));
    } else {
        append(parse(primary.toString()));
    }
    if (binding.hasAlternative) append(parse(configured.value(key + QLatin1String(AlternativeSuffix)).toString()));
    return active;
}
}
