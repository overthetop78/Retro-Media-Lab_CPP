#pragma once
#include "AppApi.h"

#include <RetroMediaLab/Emulation/EmulationKey.h>
#include <RetroMediaLab/Emulation/Sdk.h>

#include <functional>
#include <optional>

class QKeyEvent;
class QPoint;

namespace RetroMediaLab {
class EmulationKeyboardModifiers final {
public:
    RETROMEDIALAB_APP_API void observe(const QKeyEvent&, Emulation::EmulationKey);
    RETROMEDIALAB_APP_API Qt::KeyboardModifiers normalize(Qt::KeyboardModifiers) const;
    RETROMEDIALAB_APP_API void clear();
    bool syntheticLeftControl() const { return syntheticLeftControl_; }

private:
    std::optional<quint32> leftControlTimestamp_;
    bool syntheticLeftControl_ = false;
    bool rightControl_ = false;
    bool rightAlt_ = false;
};

class EmulationHostKeyboard final {
public:
    static RETROMEDIALAB_APP_API Emulation::EmulationKey keyFor(const QKeyEvent&);
    static RETROMEDIALAB_APP_API quint64 physicalId(const QKeyEvent&);
    static RETROMEDIALAB_APP_API QKeyCombination hostCombination(QStringView bindingKey);
    static RETROMEDIALAB_APP_API void applyMachineBindings(Emulation::InputSnapshot&,
        const QList<Emulation::RawControllerState>&, Qt::MouseButtons, const QPoint&,
        const QHash<QString, QString>&, const std::function<bool(const QString&)>& reserved);
    RETROMEDIALAB_APP_API bool update(const QKeyEvent&, Emulation::InputSnapshot&);
    RETROMEDIALAB_APP_API void clear();

private:
    struct HeldKey {
        Emulation::EmulationKey key;
        quint32 virtualKey;
        char32_t character;
    };
    QHash<quint64, HeldKey> held_;
    EmulationKeyboardModifiers modifiers_;
};
}
