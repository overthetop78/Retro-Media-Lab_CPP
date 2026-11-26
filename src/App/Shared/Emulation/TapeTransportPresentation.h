#pragma once

#include <RetroMediaLab/Emulation/Sdk.h>

namespace RetroMediaLab {
struct TapeButtonState {
    bool supported;
    bool enabled;
    bool active;
    bool blinking;
    bool operator==(const TapeButtonState&) const = default;
};

inline TapeButtonState tapeButtonState(const Emulation::TapeTransportStatus& transport,
    Emulation::TapeCommand command, bool powered, bool canRecord) {
    using Emulation::TapeCommand;
    using Emulation::TapeState;

    auto operation = transport.activeOperation;
    if (!operation && transport.state == TapeState::Playing) operation = TapeCommand::Play;
    if (!operation && transport.state == TapeState::Recording) operation = TapeCommand::Record;
    const bool paused = transport.state == TapeState::Paused;
    const bool play = command == TapeCommand::Play;
    const bool record = command == TapeCommand::Record;
    const bool running = operation == command &&
        ((play && (paused || transport.state == TapeState::Playing)) ||
         (record && (paused || transport.state == TapeState::Recording)));
    const bool active = running || (paused && command == TapeCommand::Pause) ||
        (operation == command && (command == TapeCommand::Rewind || command == TapeCommand::FastForward));
    const bool supported = transport.availableCommands.contains(command) && (!record || canRecord);
    return {supported, powered && supported && (transport.state != TapeState::Empty || record) && !running,
        active, paused && running};
}
}
