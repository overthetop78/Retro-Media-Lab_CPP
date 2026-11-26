#pragma once

#include "Shared/Controls/MachineInputBindings.h"

namespace RetroMediaLab {
namespace Platform { class GamepadSource; }
class MachineControllerPorts final : public QWidget {
public:
    MachineControllerPorts(QList<Emulation::ControllerPortSettings>&, Platform::GamepadSource&, MachineInputBindings::Translate,
        MachineInputBindings::Translate moduleText, std::function<void()> changed, QWidget* parent);
    bool valid() const;
    bool changed() const;

private:
    QList<std::function<bool()>> validators_;
    QList<std::function<bool()>> changes_;
};
}
