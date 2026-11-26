#include <RetroMediaLab/Platform/Gamepads.h>

namespace RetroMediaLab::Platform {
struct GamepadSource::State {};
GamepadSource::GamepadSource() = default;
GamepadSource::~GamepadSource() = default;
QList<GamepadState> GamepadSource::read() { return {}; }
QList<GamepadRumbleDevice> GamepadSource::rumbleDevices() const { return {}; }
void GamepadSource::updateRumble(const QHash<QString, GamepadRumble>&) {}
void GamepadSource::stopRumble() noexcept {}
}
