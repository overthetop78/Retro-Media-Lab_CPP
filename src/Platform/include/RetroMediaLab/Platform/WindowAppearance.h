#pragma once
#include <cstdint>
namespace RetroMediaLab::Platform {
// The caller owns the window. This operation creates no DWM object or DLL reference.
void applyWindowAppearance(std::uintptr_t nativeWindow, bool dark) noexcept;
}
