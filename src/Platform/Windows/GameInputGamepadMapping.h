#pragma once

#include <GameInput.h>
#include <RetroMediaLab/Platform/Gamepads.h>
#include <algorithm>
#include <span>

namespace RetroMediaLab::Platform::GameInputGamepadMapping {
using namespace GameInput::v3;

struct Controls {
    std::span<const float> axes;
    std::span<const bool> buttons;
    std::span<const GameInputSwitchPosition> switches;
};

inline float switchAxis(GameInputSwitchPosition position, GameInputSwitchPosition reference) {
    constexpr int FirstDirection = GameInputSwitchUp;
    constexpr int DirectionCount = GameInputSwitchUpLeft - FirstDirection + 1;
    constexpr int HalfCircle = DirectionCount / 2;
    if (position == GameInputSwitchCenter) return RawGamepad::AxisCenter;
    if (position == reference) return 1.0f;
    const auto opposite = (int(reference) - FirstDirection + HalfCircle) % DirectionCount + FirstDirection;
    return int(position) == opposite ? 0.0f : RawGamepad::AxisCenter;
}

inline float axisValue(const GameInputAxisMapping& mapping, const Controls& controls) {
    const auto index = mapping.controllerIndex;
    float value = 0.0f;
    switch (mapping.controllerElementKind) {
    case GameInputElementKindAxis:
        if (index < controls.axes.size()) value = controls.axes[index];
        break;
    case GameInputElementKindButton:
        if (index < controls.buttons.size()) {
            const auto high = controls.buttons[index];
            if (mapping.fromTwoButtons && mapping.buttonMinIndexValue < controls.buttons.size()) {
                const auto low = controls.buttons[mapping.buttonMinIndexValue];
                value = high == low ? RawGamepad::AxisCenter : float(high);
            } else {
                value = float(high);
            }
        }
        break;
    case GameInputElementKindSwitch:
        if (index < controls.switches.size()) value = switchAxis(controls.switches[index], mapping.referenceDirection);
        break;
    default:
        break;
    }
    return mapping.isInverted ? 1.0f - value : value;
}

inline bool buttonValue(const GameInputButtonMapping& mapping, const Controls& controls) {
    const auto index = mapping.controllerIndex;
    switch (mapping.controllerElementKind) {
    case GameInputElementKindButton:
        return index < controls.buttons.size() && controls.buttons[index];
    case GameInputElementKindAxis:
        return index < controls.axes.size() && (mapping.isInverted
            ? controls.axes[index] < RawGamepad::AxisCenter : controls.axes[index] > RawGamepad::AxisCenter);
    case GameInputElementKindSwitch:
        return index < controls.switches.size() && controls.switches[index] == mapping.switchPosition;
    default:
        return false;
    }
}

inline GameInputGamepadState map(IGameInputMapper& mapper, const Controls& controls) {
    const auto axis = [&](GameInputGamepadAxes element, float unmapped) {
        GameInputAxisMapping mapping{};
        return mapper.GetGamepadAxisMappingInfo(element, &mapping) ? axisValue(mapping, controls) : unmapped;
    };
    const auto stick = [&](GameInputGamepadAxes element) {
        return std::clamp(axis(element, RawGamepad::AxisCenter) * 2.0f - 1.0f, -1.0f, 1.0f);
    };

    GameInputGamepadState state{};
    state.leftTrigger = axis(GameInputGamepadLeftTrigger, 0.0f);
    state.rightTrigger = axis(GameInputGamepadRightTrigger, 0.0f);
    state.leftThumbstickX = stick(GameInputGamepadLeftThumbstickX);
    state.leftThumbstickY = stick(GameInputGamepadLeftThumbstickY);
    state.rightThumbstickX = stick(GameInputGamepadRightThumbstickX);
    state.rightThumbstickY = stick(GameInputGamepadRightThumbstickY);
    for (uint32_t bit = GameInputGamepadMenu; bit <= GameInputGamepadPaddleRight2; bit <<= 1) {
        const auto button = static_cast<GameInputGamepadButtons>(bit);
        GameInputButtonMapping mapping{};
        if (mapper.GetGamepadButtonMappingInfo(button, &mapping) && buttonValue(mapping, controls))
            state.buttons |= button;
    }
    return state;
}
}
