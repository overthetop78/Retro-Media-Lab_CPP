#include "NativeRefreshClock.h"

#include <windows.h>
#include <d3dkmthk.h>
#include <chrono>
#include <condition_variable>
#include <mutex>

namespace RetroMediaLab::VideoPresentation {
namespace {
using namespace std::chrono_literals;
constexpr auto ModeCheckInterval = 1s;
constexpr auto RetryInterval = 250ms;
constexpr auto DisplayDriver = L"DISPLAY";

struct DisplayMode {
    MONITORINFOEXW monitor{};
    double rate = 0;
};

DisplayMode displayMode(HWND window) {
    DisplayMode result;
    if (!IsWindow(window) || IsIconic(window)) return result;
    const auto monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
    result.monitor.cbSize = sizeof(result.monitor);
    if (!monitor || !GetMonitorInfoW(monitor, reinterpret_cast<MONITORINFO*>(&result.monitor))) return result;

    DEVMODEW mode{};
    mode.dmSize = sizeof(mode);
    if (EnumDisplaySettingsW(result.monitor.szDevice, ENUM_CURRENT_SETTINGS, &mode) && mode.dmDisplayFrequency > 1)
        result.rate = mode.dmDisplayFrequency;
    return result;
}

struct DisplayContext {
    HDC value = nullptr;
    ~DisplayContext() { if (value) DeleteDC(value); }
};

struct Adapter {
    D3DKMT_OPENADAPTERFROMHDC value{};
    ~Adapter() {
        if (!value.hAdapter) return;
        D3DKMT_CLOSEADAPTER close{value.hAdapter};
        D3DKMTCloseAdapter(&close);
    }
};

void runMonitor(HWND window, DisplayMode mode, std::stop_token token,
        const std::function<void(std::uint64_t, double)>& present) {
    const DisplayContext context{CreateDCW(DisplayDriver, mode.monitor.szDevice, nullptr, nullptr)};
    if (!context.value) return;
    Adapter adapter;
    adapter.value.hDc = context.value;
    if (D3DKMTOpenAdapterFromHdc(&adapter.value) != 0) return;
    D3DKMT_WAITFORVERTICALBLANKEVENT blank{};
    blank.hAdapter = adapter.value.hAdapter;
    blank.VidPnSourceId = adapter.value.VidPnSourceId;

    auto checkedAt = std::chrono::steady_clock::now();
    std::uint64_t refresh = 0;
    while (!token.stop_requested() && D3DKMTWaitForVerticalBlankEvent(&blank) == 0) {
        if (token.stop_requested()) return;
        const auto now = std::chrono::steady_clock::now();
        if (now - checkedAt >= ModeCheckInterval) {
            const auto current = displayMode(window);
            if (current.rate <= 0 || lstrcmpW(current.monitor.szDevice, mode.monitor.szDevice) != 0) return;
            mode.rate = current.rate;
            checkedAt = now;
        }
        present(++refresh, mode.rate);
    }
}
}

void runNativeRefreshClock(std::uintptr_t handle, std::stop_token token,
        const std::function<void(std::uint64_t, double)>& present) {
    const auto window = reinterpret_cast<HWND>(handle);
    std::condition_variable_any retry;
    std::mutex mutex;
    while (!token.stop_requested()) {
        const auto mode = displayMode(window);
        if (mode.rate > 0) runMonitor(window, mode, token, present);
        if (token.stop_requested()) return;
        present(0, 0);
        std::unique_lock lock(mutex);
        retry.wait_for(lock, token, RetryInterval, [] { return false; });
    }
}
}
