#include <RetroMediaLab/Platform/Gamepads.h>

namespace RetroMediaLab::Platform {
bool canOfferGameInputInstaller() { return false; }
GameInputInstallResult launchGameInputInstaller() { return GameInputInstallResult::Failed; }

struct GamepadSource::State {};
GamepadSource::GamepadSource() = default;
GamepadSource::~GamepadSource() = default;
QList<GamepadState> GamepadSource::read() { return {}; }
QList<GamepadRumbleDevice> GamepadSource::rumbleDevices() const { return {}; }
bool GamepadSource::updateRumble(const QHash<QString, GamepadRumble>&) { return false; }
void GamepadSource::stopRumble() noexcept {}
}
