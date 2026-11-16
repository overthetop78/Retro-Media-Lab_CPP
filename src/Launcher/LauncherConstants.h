#pragma once
// Technical constants; displayed text is resolved through translation catalogs.
namespace RetroMediaLab::LauncherConstants {
inline constexpr auto NullCharacter = '\0';
inline constexpr auto WideNullCharacter = L'\0';
inline constexpr auto WideEmpty = L"";
inline constexpr auto WideLib = L"lib";
inline constexpr auto WideLibQt6 = L"lib/Qt6";
inline constexpr auto WideLibDirectX = L"lib/DirectX";
inline constexpr auto WideLibOpenGL = L"lib/OpenGL";
inline constexpr auto WideLibFFmpeg = L"lib/FFmpeg";
inline constexpr auto WideLibAppDll = L"lib/App.dll";
inline constexpr auto WideLibInfrastructureDll = L"lib/Infrastructure.dll";
inline constexpr auto ProbeEntry = "RetroMediaLabProbe";
inline constexpr auto WideProbeEntry = L"RetroMediaLabProbe";
inline constexpr auto RetroMediaLabRun = "RetroMediaLabRun";
inline constexpr auto WideRetroMediaLabRun = L"RetroMediaLabRun";
inline constexpr auto WideRetroMediaLab = L"Retro Media Lab";
namespace Process {
inline constexpr auto ExecutablePathCapacity = 32768;
}
}
