#pragma once

#include <RetroMediaLab/Platform/Gamepads.h>

namespace RetroMediaLab::Platform {
class MidiInput final {
public:
    MidiInput();
    QList<GamepadState> read() const;

private:
    struct State;
    std::shared_ptr<State> state_;
};
}
