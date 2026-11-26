#include <RetroMediaLab/Emulation/InputSettings.h>
#include <algorithm>
#include <cmath>
#include <limits>

namespace RetroMediaLab::Emulation {
namespace {
bool validMotor(ControllerMotor motor) {
    switch (motor) {
    case ControllerMotor::LowFrequency:
    case ControllerMotor::HighFrequency:
    case ControllerMotor::LeftTrigger:
    case ControllerMotor::RightTrigger:
        return true;
    }
    return false;
}
}

void assignControllerRumbleDevice(ControllerPortFeedback& port, const QString& deviceId) {
    constexpr std::pair<QLatin1StringView, ControllerMotor> StandardMotors[]{
        {ControllerFeedbackCommands::Strong, ControllerMotor::LowFrequency},
        {ControllerFeedbackCommands::Weak, ControllerMotor::HighFrequency},
    };
    const bool disabled = deviceId.trimmed().isEmpty();
    for (const auto& [command, motor] : StandardMotors) {
        const QString id(command);
        if (!port.availableCommands.contains(id)) continue;
        if (disabled) port.outputs.remove(id);
        else port.outputs.insert(id, {deviceId, motor});
    }
}

std::optional<ControllerPortFeedback> controllerModelFeedback(const ControllerPortSettings& port) {
    if (std::ranges::none_of(port.controllerChoices, [](const auto& choice) { return !choice.visualOutputDefinitions.isEmpty(); }))
        return std::nullopt;
    ControllerPortFeedback feedback{port.number, {}, {}, port.displayName, {}};
    const auto choice = std::ranges::find(port.controllerChoices, port.selectedControllerId, &ControllerChoice::id);
    if (choice == port.controllerChoices.end()) return feedback;
    const auto visual = port.visualId.isEmpty() ? choice->defaultVisualId : port.visualId;
    const auto definitions = choice->visualOutputDefinitions.constFind(visual);
    if (definitions == choice->visualOutputDefinitions.cend()) return feedback;
    for (const auto& definition : *definitions) {
        feedback.availableCommands.insert(definition.id);
        feedback.commandResourceKeys.insert(definition.id, definition.displayResourceKey);
        const auto binding = port.outputBindings.constFind(definition.id);
        if (binding != port.outputBindings.cend()) feedback.outputs.insert(definition.id, *binding);
    }
    return feedback;
}

ControllerFeedbackBindings createControllerFeedbackBindings(const QList<ControllerPortFeedback>& ports) {
    ControllerFeedbackBindings bindings;
    for (const auto& port : ports) {
        if (port.number <= 0 || static_cast<unsigned>(port.number) > ControllerFeedbackCommands::MaximumPortCount)
            continue;
        for (auto output = port.outputs.cbegin(); output != port.outputs.cend(); ++output) {
            if (port.availableCommands.contains(output.key()) && !output->deviceId.trimmed().isEmpty() && validMotor(output->motor))
                bindings.insert({port.number - 1, output.key()}, output.value());
        }
    }
    return bindings;
}

QList<ControllerOutput> resolveControllerFeedback(const QList<ControllerFeedbackState>& states,
    const ControllerFeedbackBindings& bindings) {
    QList<ControllerOutput> outputs;
    if (bindings.isEmpty()) return outputs;
    for (const auto& state : states) {
        if (!std::isfinite(state.strength) || state.strength < 0.0f || state.strength > 1.0f) continue;
        const auto binding = bindings.constFind({state.port, state.commandId});
        if (binding != bindings.constEnd())
            outputs.append({binding->deviceId, binding->motor, state.strength});
    }
    return outputs;
}

bool ControllerFeedbackAccumulator::setState(unsigned port, unsigned effect, quint16 strength) {
    if (port >= strengths_.size() || effect >= EffectCount) return false;
    const std::lock_guard lock(mutex_);
    strengths_[port][effect] = strength;
    return true;
}

QList<ControllerFeedbackState> ControllerFeedbackAccumulator::snapshot() const {
    const std::lock_guard lock(mutex_);
    constexpr auto MaximumStrength = static_cast<float>(std::numeric_limits<quint16>::max());
    QList<ControllerFeedbackState> states;
    for (unsigned port = 0; port < strengths_.size(); ++port) {
        const auto& effects = strengths_[port];
        if (effects[StrongEffect] != 0)
            states.append({static_cast<int>(port), QString(ControllerFeedbackCommands::Strong), effects[StrongEffect] / MaximumStrength});
        if (effects[WeakEffect] != 0)
            states.append({static_cast<int>(port), QString(ControllerFeedbackCommands::Weak), effects[WeakEffect] / MaximumStrength});
    }
    return states;
}

void ControllerFeedbackAccumulator::resetPort(int port) {
    if (port < 0 || static_cast<unsigned>(port) >= strengths_.size()) return;
    const std::lock_guard lock(mutex_);
    strengths_[static_cast<unsigned>(port)] = {};
}

void ControllerFeedbackAccumulator::reset() {
    const std::lock_guard lock(mutex_);
    strengths_ = {};
}
}
