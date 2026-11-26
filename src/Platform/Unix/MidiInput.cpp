#include <RetroMediaLab/Platform/MidiInput.h>

namespace RetroMediaLab::Platform {
MidiInput::MidiInput() = default;
QList<GamepadState> MidiInput::read() const { return {}; }
}
