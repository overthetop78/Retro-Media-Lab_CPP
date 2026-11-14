#include <RetroMediaLab/Platform/WindowAppearance.h>
namespace RetroMediaLab::Platform {
void applyWindowAppearance(std::uintptr_t, bool) noexcept {
    // Native decorations belong to the window manager. No Windows API dependency.
}
}
