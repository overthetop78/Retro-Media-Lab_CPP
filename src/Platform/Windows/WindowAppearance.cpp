#include "PlatformConstants.h"
#include <RetroMediaLab/Platform/WindowAppearance.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <dwmapi.h>
namespace RetroMediaLab::Platform {
bool applyWindowAppearance(std::uintptr_t nativeWindow, bool dark, std::uint32_t caption, std::uint32_t text, bool systemColors) noexcept {
    if (!nativeWindow) return false;
    const BOOL enabled = dark ? TRUE : FALSE;
    const auto window = reinterpret_cast<HWND>(nativeWindow);
    // Older Windows versions may not support this optional appearance attribute.
    DwmSetWindowAttribute(window, DWMWA_USE_IMMERSIVE_DARK_MODE, &enabled, sizeof(enabled));
    const COLORREF captionColor = systemColors ? DWMWA_COLOR_DEFAULT : RGB((caption >> PlatformConstants::WindowAppearanceValues::RedShift) & PlatformConstants::WindowAppearanceValues::ChannelMask, (caption >> PlatformConstants::WindowAppearanceValues::GreenShift) & PlatformConstants::WindowAppearanceValues::ChannelMask, caption & PlatformConstants::WindowAppearanceValues::ChannelMask);
    const COLORREF textColor = systemColors ? DWMWA_COLOR_DEFAULT : RGB((text >> PlatformConstants::WindowAppearanceValues::RedShift) & PlatformConstants::WindowAppearanceValues::ChannelMask, (text >> PlatformConstants::WindowAppearanceValues::GreenShift) & PlatformConstants::WindowAppearanceValues::ChannelMask, text & PlatformConstants::WindowAppearanceValues::ChannelMask);
    const auto captionResult = DwmSetWindowAttribute(window, DWMWA_CAPTION_COLOR, &captionColor, sizeof(captionColor));
    const auto textResult = DwmSetWindowAttribute(window, DWMWA_TEXT_COLOR, &textColor, sizeof(textColor));
    return SUCCEEDED(captionResult) && SUCCEEDED(textResult);
}
}
