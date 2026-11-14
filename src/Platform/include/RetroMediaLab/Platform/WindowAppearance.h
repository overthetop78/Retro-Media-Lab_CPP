#pragma once
#include <cstdint>
namespace RetroMediaLab::Platform {
// The caller owns the window. This operation creates no DWM object or DLL reference.
// Colors are 0xAARRGGBB; the platform adapter converts them to native values.
bool applyWindowAppearance(std::uintptr_t nativeWindow, bool dark, std::uint32_t caption, std::uint32_t text) noexcept;
}
