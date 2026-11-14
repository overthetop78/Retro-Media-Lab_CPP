#include <RetroMediaLab/Platform/WindowAppearance.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <dwmapi.h>
namespace RetroMediaLab::Platform {
bool applyWindowAppearance(std::uintptr_t nativeWindow, bool dark, std::uint32_t caption, std::uint32_t text) noexcept {
    if (!nativeWindow) return false;
    const BOOL enabled = dark ? TRUE : FALSE;
    const auto window = reinterpret_cast<HWND>(nativeWindow);
    // Older Windows versions may not support this optional appearance attribute.
    DwmSetWindowAttribute(window, DWMWA_USE_IMMERSIVE_DARK_MODE, &enabled, sizeof(enabled));
    const COLORREF captionColor = RGB((caption >> 16) & 255, (caption >> 8) & 255, caption & 255);
    const COLORREF textColor = RGB((text >> 16) & 255, (text >> 8) & 255, text & 255);
    const auto captionResult = DwmSetWindowAttribute(window, DWMWA_CAPTION_COLOR, &captionColor, sizeof(captionColor));
    const auto textResult = DwmSetWindowAttribute(window, DWMWA_TEXT_COLOR, &textColor, sizeof(textColor));
    return SUCCEEDED(captionResult) && SUCCEEDED(textResult);
}
}
