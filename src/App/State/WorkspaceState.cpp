#include "WorkspaceState.h"
namespace RetroMediaLab {
bool WorkspaceState::hasGeometry() const noexcept { return !mainWindowGeometry.isEmpty(); }
}
