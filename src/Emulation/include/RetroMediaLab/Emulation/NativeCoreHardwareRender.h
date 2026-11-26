#pragma once

#include "EmulationExport.h"
#include <QByteArray>
#include <QSize>
#include <cstdint>
#include <memory>
#include <optional>

namespace RetroMediaLab::Emulation {
inline constexpr int NativeCoreOpenGlContext = 1;
inline constexpr int NativeCoreOpenGlCoreContext = 3;

struct NativeCoreHardwareRenderCallback {
    using ContextCallback = void (*)();
    using FramebufferCallback = std::uintptr_t (*)();
    using Symbol = void (*)();
    using GetProcAddressCallback = Symbol (*)(const char*);

    int contextType;
    ContextCallback contextReset;
    FramebufferCallback getCurrentFramebuffer;
    GetProcAddressCallback getProcAddress;
    bool depth, stencil, bottomLeftOrigin;
    unsigned versionMajor, versionMinor;
    bool cacheContext;
    ContextCallback contextDestroy;
    bool debugContext;
};

// The module owns reset/destroy callbacks and stops its core before releasing the context.
// Create, use and destroy the context on the same graphics thread.
class RETROMEDIALAB_EMULATION_API NativeCoreHardwareRender final {
public:
    static std::unique_ptr<NativeCoreHardwareRender> create(NativeCoreHardwareRenderCallback&);
    ~NativeCoreHardwareRender();
    NativeCoreHardwareRender(const NativeCoreHardwareRender&) = delete;
    NativeCoreHardwareRender& operator=(const NativeCoreHardwareRender&) = delete;

    bool makeCurrent() noexcept;
    bool ensureFramebuffer(QSize) noexcept;
    // The result owns a tightly packed BGRA copy with the origin negotiated by the core.
    std::optional<QByteArray> readFramebuffer(QSize);

private:
    struct State;
    explicit NativeCoreHardwareRender(std::unique_ptr<State>);
    std::unique_ptr<State> state_;
};
}
