#pragma once
#include "AppApi.h"

#include "Shared/Controls/MachineInputBindings.h"

class QFormLayout;

namespace RetroMediaLab {
namespace Platform { class GamepadSource; }

RETROMEDIALAB_APP_API void addControllerOutputs(QFormLayout&, Emulation::ControllerPortFeedback&, Platform::GamepadSource&,
    MachineInputBindings::Translate, MachineInputBindings::Translate moduleText, std::function<void()> changed);
}
