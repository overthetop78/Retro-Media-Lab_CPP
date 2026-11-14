#include <RetroMediaLab/Platform/WindowAppearance.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <dwmapi.h>
namespace RetroMediaLab::Platform {
void applyWindowAppearance(std::uintptr_t nativeWindow, bool dark) noexcept {
    if (!nativeWindow) return;
    const BOOL enabled = dark ? TRUE : FALSE;
    const auto window = reinterpret_cast<HWND>(nativeWindow);
    // Older Windows versions may not support this optional appearance attribute.
    DwmSetWindowAttribute(window, 20, &enabled, sizeof(enabled));
}
}
