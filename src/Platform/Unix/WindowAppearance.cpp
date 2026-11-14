#include <RetroMediaLab/Platform/WindowAppearance.h>
namespace RetroMediaLab::Platform {
bool applyWindowAppearance(std::uintptr_t, bool, std::uint32_t, std::uint32_t) noexcept {
    // Native decorations belong to the window manager. No Windows API dependency.
    return false;
}
}
