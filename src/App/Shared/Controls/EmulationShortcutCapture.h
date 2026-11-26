#pragma once

#include "Shared/Emulation/EmulationPhysicalShortcuts.h"
#include <RetroMediaLab/Platform/MidiInput.h>
#include <optional>
#include <QDialog>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QMouseEvent>
#include <QMetaEnum>
#include <QTimer>
#include <QVBoxLayout>
#include <chrono>

namespace RetroMediaLab {
class EmulationShortcutCapture final : public QDialog {
public:
    EmulationShortcutCapture(QWidget* parent, const QString& title, const QString& prompt, bool escapeCancels = true,
        bool prefixKeyboardSource = false, bool captureMidi = false)
        : QDialog(parent), samples_(source_.read()), initialSamples_(samples_), escapeCancels_(escapeCancels),
          prefixKeyboardSource_(prefixKeyboardSource) {
        setWindowTitle(title);
        auto* layout = new QVBoxLayout(this);
        auto* label = new QLabel(prompt, this);
        label->setAttribute(Qt::WA_TransparentForMouseEvents);
        layout->addWidget(label);
        if (captureMidi) {
            midi_.emplace();
            midiSamples_ = midi_->read();
        }
        const auto pollControllers = [this] {
            auto current = source_.read();
            namespace Physical = EmulationPhysicalShortcuts;
            auto pressed = Physical::newControllerSources(current, samples_, initialSamples_);
            samples_ = current;
            if (midi_) {
                auto midi = midi_->read();
                pressed.append(Physical::newControllerSources(midi, midiSamples_, {}));
                midiSamples_ = midi;
                current.append(midi);
            }
            for (const auto& source : pressed) {
                capturedSources_.insert(source);
                heldSources_.insert(source);
            }
            const Physical::Input input{{}, {}, current};
            heldSources_.removeIf([&](const QString& source) {
                return source.startsWith(QLatin1String(EmulationPhysicalShortcuts::ControllerPrefix)) &&
                    !EmulationPhysicalShortcuts::pressed(QStringView(source), input);
            });
            finishIfReleased();
        };
        connect(&timer_, &QTimer::timeout, this, pollControllers);
        timer_.start(PollInterval);
        QTimer::singleShot(CaptureTimeout, this, &QDialog::reject);
    }

    QString binding() const { return binding_; }

protected:
    bool event(QEvent* event) override {
        if (event->type() == QEvent::WindowDeactivate && isVisible()) reject();
        return QDialog::event(event);
    }

    void mousePressEvent(QMouseEvent* event) override {
        const auto source = EmulationPhysicalShortcuts::mouseSource(event->button());
        if (!source.isEmpty()) {
            capturedSources_.insert(source);
            heldSources_.insert(source);
        }
        event->accept();
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
        heldSources_.remove(EmulationPhysicalShortcuts::mouseSource(event->button()));
        finishIfReleased();
        event->accept();
    }

    void wheelEvent(QWheelEvent* event) override {
        const auto source = EmulationPhysicalShortcuts::wheelSource(event->angleDelta());
        if (!source.isEmpty()) capturedSources_.insert(source);
        finishIfReleased();
        event->accept();
    }

    void keyPressEvent(QKeyEvent* event) override {
        if (event->key() == Qt::Key_Escape && escapeCancels_) {
            reject();
        } else if (!event->isAutoRepeat()) {
            namespace Physical = EmulationPhysicalShortcuts;
            using Key = Emulation::EmulationKey;
            const auto id = EmulationHostKeyboard::physicalId(*event);
            const auto key = EmulationHostKeyboard::keyFor(*event);
            const bool wasSynthetic = modifierState_.syntheticLeftControl();
            if (!heldKeys_.contains(id)) modifierState_.observe(*event, key);
            heldKeys_.insert(id);
            if (!wasSynthetic && modifierState_.syntheticLeftControl()) {
                machineKeys_.removeAll(Key::LeftControl);
                modifiers_ &= ~Qt::ControlModifier;
            }
            if (!escapeCancels_ && key != Key::Unknown && !machineKeys_.contains(key) &&
                (key != Key::LeftControl || !modifierState_.syntheticLeftControl()))
                machineKeys_.append(key);
            modifiers_ |= modifierState_.normalize(event->modifiers()) & (Physical::ChordModifiers | Qt::KeypadModifier);
            switch (event->key()) {
            case Qt::Key_Control: modifiers_ |= Qt::ControlModifier; break;
            case Qt::Key_Shift: modifiers_ |= Qt::ShiftModifier; break;
            case Qt::Key_AltGr:
            case Qt::Key_Alt: modifiers_ |= Qt::AltModifier; break;
            case Qt::Key_Meta: modifiers_ |= Qt::MetaModifier; break;
            default:
                if (!Physical::modifierKey(event->key()) && !keys_.contains(event->key())) keys_.append(event->key());
                break;
            }
        }
        event->accept();
    }

    void keyReleaseEvent(QKeyEvent* event) override {
        if (!event->isAutoRepeat()) {
            heldKeys_.remove(EmulationHostKeyboard::physicalId(*event));
            modifierState_.observe(*event, EmulationHostKeyboard::keyFor(*event));
        }
        finishIfReleased();
        event->accept();
    }

private:
    QString machineKeyboard() const {
        namespace Physical = EmulationPhysicalShortcuts;
        using Key = Emulation::EmulationKey;
        const auto meta = QMetaEnum::fromType<Key>();
        auto keys = machineKeys_;
        std::ranges::sort(keys);
        QStringList parts;
        struct Modifier { Qt::KeyboardModifier flag; Key left, right; QLatin1StringView name; };
        constexpr Modifier groups[]{
            {Qt::ControlModifier, Key::LeftControl, Key::RightControl, QLatin1String("Ctrl")},
            {Qt::AltModifier, Key::LeftAlt, Key::RightAlt, QLatin1String("Alt")},
            {Qt::ShiftModifier, Key::LeftShift, Key::RightShift, QLatin1String("Shift")},
            {Qt::MetaModifier, Key::LeftWindows, Key::RightWindows, QLatin1String("LeftWindows")},
        };
        const auto appendKey = [&](Key key) { parts.append(QString::fromLatin1(meta.valueToKey(int(key)))); };
        for (const auto& group : groups) {
            const bool left = keys.removeOne(group.left);
            const bool right = keys.removeOne(group.right);
            if (left) appendKey(group.left);
            if (right) appendKey(group.right);
            if (!left && !right && modifiers_.testFlag(group.flag)) parts.append(group.name.toString());
        }
        for (const auto key : keys) appendKey(key);
        return parts.join(Physical::ChordSeparator);
    }

    void finishIfReleased() {
        namespace Physical = EmulationPhysicalShortcuts;
        const bool hasKeyboard = escapeCancels_
            ? !keys_.isEmpty() || (modifiers_ & Physical::ChordModifiers) != Qt::NoModifier
            : !machineKeys_.isEmpty();
        if (!heldKeys_.isEmpty() || !heldSources_.isEmpty() || (!hasKeyboard && capturedSources_.isEmpty())) return;
        QStringList parts(capturedSources_.values());
        std::ranges::sort(parts, [](const QString& left, const QString& right) {
            const bool mouseLeft = left.startsWith(QLatin1String(Physical::MousePrefix));
            const bool mouseRight = right.startsWith(QLatin1String(Physical::MousePrefix));
            if (mouseLeft != mouseRight) return mouseLeft;
            return left < right;
        });
        if (!escapeCancels_) {
            const auto keyboard = machineKeyboard();
            if (!keyboard.isEmpty()) {
                parts.prepend(prefixKeyboardSource_ || !parts.isEmpty()
                    ? QLatin1String(Physical::KeyboardPrefix) + keyboard : keyboard);
            }
            finish(parts.join(Physical::ChordSeparator));
            return;
        }
        if (!keys_.isEmpty() || modifiers_ != Qt::NoModifier) {
            if (escapeCancels_ && parts.isEmpty() && keys_.size() == 1) {
                finish(QKeySequence(QKeyCombination(modifiers_, Qt::Key(keys_.front()))).toString(QKeySequence::PortableText));
                return;
            }
            parts.prepend(QLatin1String(Physical::KeyboardPrefix) + Physical::formatKeyboard(modifiers_, keys_));
        }
        finish(parts.join(Physical::ChordSeparator));
    }

    void finish(const QString& source) {
        binding_ = source;
        timer_.stop();
        accept();
    }

    static constexpr auto PollInterval = std::chrono::milliseconds(16);
    static constexpr auto CaptureTimeout = std::chrono::seconds(15);
    Platform::GamepadSource source_;
    std::optional<Platform::MidiInput> midi_;
    QList<Platform::GamepadState> midiSamples_;
    QList<Platform::GamepadState> samples_;
    const QList<Platform::GamepadState> initialSamples_;
    QTimer timer_;
    QSet<quint64> heldKeys_;
    EmulationKeyboardModifiers modifierState_;
    QList<int> keys_;
    QList<Emulation::EmulationKey> machineKeys_;
    Qt::KeyboardModifiers modifiers_{};
    QSet<QString> heldSources_, capturedSources_;
    QString binding_;
    bool escapeCancels_;
    bool prefixKeyboardSource_;
};
}
