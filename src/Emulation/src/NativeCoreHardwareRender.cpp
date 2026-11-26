#include <RetroMediaLab/Emulation/NativeCoreHardwareRender.h>
#include <limits>
#include <utility>

#ifdef Q_OS_WIN
#include <QtCore/qt_windows.h>
#include <GL/gl.h>
#include <QScopeGuard>
#include <QVersionNumber>
#include <algorithm>
#endif

namespace RetroMediaLab::Emulation {
#ifdef Q_OS_WIN
namespace {
constexpr auto WindowClass = L"STATIC";
constexpr auto OpenGlLibrary = L"opengl32.dll";
constexpr int WindowSize = 1, InitialFramebufferSize = 2048;
constexpr BYTE ColorBits = 32, DepthBits = 24, StencilBits = 8;
constexpr int ContextMajor = 0x2091, ContextMinor = 0x2092, ContextProfile = 0x9126, CoreProfileBit = 1;
constexpr GLenum FramebufferTarget = 0x8D40, RenderbufferTarget = 0x8D41;
constexpr GLenum ReadFramebuffer = 0x8CA8, ReadFramebufferBinding = 0x8CAA;
constexpr GLenum FramebufferComplete = 0x8CD5, ColorAttachment = 0x8CE0, DepthStencilAttachment = 0x821A;
constexpr GLenum Depth24Stencil8 = 0x88F0, Bgra = 0x80E1, PackBuffer = 0x88EB, PackBufferBinding = 0x88ED;
constexpr GLenum ContextProfileMask = 0x9126;
constexpr GLint TextureLevel = 0, TextureBorder = 0, PackAlignment = 1;
constexpr GLsizei OneObject = 1;
constexpr int BytesPerPixel = 4;

PROC procAddress(const char* name) noexcept {
    if (!name || !*name) return nullptr;
    const auto proc = wglGetProcAddress(name);
    const auto value = reinterpret_cast<std::uintptr_t>(proc);
    if (value > 3 && value != std::numeric_limits<std::uintptr_t>::max()) return proc;
    const auto library = GetModuleHandleW(OpenGlLibrary);
    return library ? GetProcAddress(library, name) : nullptr;
}

template<class Function>
Function glFunction(const char* name) noexcept {
    return reinterpret_cast<Function>(procAddress(name));
}

struct GlFunctions {
    using Objects = void (APIENTRY*)(GLsizei, GLuint*);
    using DeleteObjects = void (APIENTRY*)(GLsizei, const GLuint*);
    using Bind = void (APIENTRY*)(GLenum, GLuint);
    using AttachTexture = void (APIENTRY*)(GLenum, GLenum, GLenum, GLuint, GLint);
    using AttachBuffer = void (APIENTRY*)(GLenum, GLenum, GLenum, GLuint);
    using AllocateStorage = void (APIENTRY*)(GLenum, GLenum, GLsizei, GLsizei);
    using CheckFramebuffer = GLenum (APIENTRY*)(GLenum);

    Objects genFramebuffers = glFunction<Objects>("glGenFramebuffers");
    Objects genRenderbuffers = glFunction<Objects>("glGenRenderbuffers");
    DeleteObjects deleteFramebuffers = glFunction<DeleteObjects>("glDeleteFramebuffers");
    DeleteObjects deleteRenderbuffers = glFunction<DeleteObjects>("glDeleteRenderbuffers");
    Bind bindFramebuffer = glFunction<Bind>("glBindFramebuffer");
    Bind bindRenderbuffer = glFunction<Bind>("glBindRenderbuffer");
    Bind bindBuffer = glFunction<Bind>("glBindBuffer");
    AttachTexture framebufferTexture = glFunction<AttachTexture>("glFramebufferTexture2D");
    AttachBuffer framebufferRenderbuffer = glFunction<AttachBuffer>("glFramebufferRenderbuffer");
    AllocateStorage renderbufferStorage = glFunction<AllocateStorage>("glRenderbufferStorage");
    CheckFramebuffer checkFramebuffer = glFunction<CheckFramebuffer>("glCheckFramebufferStatus");

    bool complete() const noexcept {
        return genFramebuffers && genRenderbuffers && deleteFramebuffers && deleteRenderbuffers &&
            bindFramebuffer && bindRenderbuffer && framebufferTexture && framebufferRenderbuffer &&
            renderbufferStorage && checkFramebuffer;
    }
};

struct Framebuffer {
    QSize size;
    GLuint id = 0, texture = 0, depth = 0;

    void destroy(const GlFunctions& gl) noexcept {
        if (depth) gl.deleteRenderbuffers(OneObject, &depth);
        if (texture) glDeleteTextures(OneObject, &texture);
        if (id) gl.deleteFramebuffers(OneObject, &id);
        *this = {};
    }
};
}

struct NativeCoreHardwareRender::State {
    HWND window = nullptr;
    HDC dc = nullptr;
    HGLRC renderingContext = nullptr;
    std::optional<GlFunctions> gl;
    Framebuffer framebuffer;
    bool bottomLeftOrigin = false;
    static inline thread_local State* current = nullptr;

    ~State() {
        if (renderingContext) {
            const auto previous = wglGetCurrentContext();
            const auto previousDc = wglGetCurrentDC();
            if (gl && gl->complete() && (previous == renderingContext || wglMakeCurrent(dc, renderingContext)))
                framebuffer.destroy(*gl);
            if (wglGetCurrentContext() == renderingContext) {
                wglMakeCurrent(nullptr, nullptr);
                if (previous && previous != renderingContext) wglMakeCurrent(previousDc, previous);
            }
            wglDeleteContext(renderingContext);
        }
        if (current == this) current = nullptr;
        if (dc) ReleaseDC(window, dc);
        if (window) DestroyWindow(window);
    }

    static std::uintptr_t currentFramebuffer() noexcept {
        return current && current->renderingContext == wglGetCurrentContext() ? current->framebuffer.id : 0;
    }

    static NativeCoreHardwareRenderCallback::Symbol resolve(const char* name) noexcept {
        return current && current->renderingContext == wglGetCurrentContext()
            ? reinterpret_cast<NativeCoreHardwareRenderCallback::Symbol>(procAddress(name)) : nullptr;
    }

    bool makeCurrent() noexcept {
        if (!wglMakeCurrent(dc, renderingContext)) return false;
        current = this;
        return true;
    }

    bool initialize(const NativeCoreHardwareRenderCallback& requested) {
        bottomLeftOrigin = requested.bottomLeftOrigin;
        window = CreateWindowExW(0, WindowClass, L"", WS_POPUP, 0, 0, WindowSize, WindowSize,
            nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
        if (!window || !(dc = GetDC(window))) return false;
        PIXELFORMATDESCRIPTOR format{};
        format.nSize = sizeof(format);
        format.nVersion = 1;
        format.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        format.iPixelType = PFD_TYPE_RGBA;
        format.cColorBits = ColorBits;
        format.cDepthBits = DepthBits;
        format.cStencilBits = StencilBits;
        format.iLayerType = PFD_MAIN_PLANE;
        const int pixelFormat = ChoosePixelFormat(dc, &format);
        if (!pixelFormat || !SetPixelFormat(dc, pixelFormat, &format)) return false;
        renderingContext = wglCreateContext(dc);
        if (!renderingContext || !makeCurrent()) return false;
        if (requested.contextType == NativeCoreOpenGlCoreContext) {
            using CreateContext = HGLRC (WINAPI*)(HDC, HGLRC, const int*);
            const auto create = glFunction<CreateContext>("wglCreateContextAttribsARB");
            const auto major = requested.versionMajor, minor = requested.versionMinor;
            if (!create || major > std::numeric_limits<int>::max() || minor > std::numeric_limits<int>::max())
                return false;
            const int attributes[]{ContextMajor, int(major), ContextMinor, int(minor), ContextProfile, CoreProfileBit, 0};
            const auto modern = create(dc, nullptr, attributes);
            if (!modern) return false;
            if (!wglMakeCurrent(dc, modern)) {
                wglDeleteContext(modern);
                return false;
            }
            wglDeleteContext(std::exchange(renderingContext, modern));
        }
        gl.emplace();
        return gl->complete();
    }
};
#else
struct NativeCoreHardwareRender::State {};
#endif

NativeCoreHardwareRender::NativeCoreHardwareRender(std::unique_ptr<State> state) : state_(std::move(state)) {}
NativeCoreHardwareRender::~NativeCoreHardwareRender() = default;

std::unique_ptr<NativeCoreHardwareRender> NativeCoreHardwareRender::create(NativeCoreHardwareRenderCallback& callback) {
#ifdef Q_OS_WIN
    if (callback.contextType != NativeCoreOpenGlContext && callback.contextType != NativeCoreOpenGlCoreContext) return {};
    const auto previous = wglGetCurrentContext();
    const auto previousDc = wglGetCurrentDC();
    const auto previousOwner = State::current;
    auto restore = qScopeGuard([=] {
        if (previous && wglMakeCurrent(previousDc, previous)) State::current = previousOwner;
    });
    auto state = std::make_unique<State>();
    if (!state->initialize(callback)) return {};
    const auto* versionString = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    if (!versionString) return {};
    const auto version = QVersionNumber::fromString(QByteArrayView(versionString));
    if (version.isNull()) return {};
    GLint profile = 0;
    if (version >= QVersionNumber(3, 2)) glGetIntegerv(ContextProfileMask, &profile);
    auto context = std::unique_ptr<NativeCoreHardwareRender>(new NativeCoreHardwareRender(std::move(state)));
    if (!context->ensureFramebuffer({InitialFramebufferSize, InitialFramebufferSize})) return {};
    callback.contextType = profile & CoreProfileBit ? NativeCoreOpenGlCoreContext : NativeCoreOpenGlContext;
    callback.versionMajor = unsigned(version.majorVersion());
    callback.versionMinor = unsigned(version.minorVersion());
    callback.getCurrentFramebuffer = &State::currentFramebuffer;
    callback.getProcAddress = &State::resolve;
    restore.dismiss();
    return context;
#else
    Q_UNUSED(callback)
    return {};
#endif
}

bool NativeCoreHardwareRender::makeCurrent() noexcept {
#ifdef Q_OS_WIN
    return state_->makeCurrent();
#else
    return false;
#endif
}

bool NativeCoreHardwareRender::ensureFramebuffer(QSize size) noexcept {
#ifdef Q_OS_WIN
    if (size.isEmpty() || !makeCurrent()) return false;
    auto& current = state_->framebuffer;
    if (current.id && current.size == size) return true;
    const auto& gl = *state_->gl;
    Framebuffer next{size};
    gl.genFramebuffers(OneObject, &next.id);
    glGenTextures(OneObject, &next.texture);
    gl.genRenderbuffers(OneObject, &next.depth);
    if (next.id && next.texture && next.depth) {
        gl.bindFramebuffer(FramebufferTarget, next.id);
        glBindTexture(GL_TEXTURE_2D, next.texture);
        glTexImage2D(GL_TEXTURE_2D, TextureLevel, GL_RGBA8, size.width(), size.height(), TextureBorder,
            GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        gl.framebufferTexture(FramebufferTarget, ColorAttachment, GL_TEXTURE_2D, next.texture, TextureLevel);
        gl.bindRenderbuffer(RenderbufferTarget, next.depth);
        gl.renderbufferStorage(RenderbufferTarget, Depth24Stencil8, size.width(), size.height());
        gl.framebufferRenderbuffer(FramebufferTarget, DepthStencilAttachment, RenderbufferTarget, next.depth);
        if (gl.checkFramebuffer(FramebufferTarget) == FramebufferComplete) {
            current.destroy(gl);
            current = next;
            return true;
        }
    }
    next.destroy(gl);
    gl.bindFramebuffer(FramebufferTarget, current.id);
#else
    Q_UNUSED(size)
#endif
    return false;
}

std::optional<QByteArray> NativeCoreHardwareRender::readFramebuffer(QSize size) {
#ifdef Q_OS_WIN
    const auto& framebuffer = state_->framebuffer;
    if (size.isEmpty() || size.width() > framebuffer.size.width() || size.height() > framebuffer.size.height() ||
        !makeCurrent()) return std::nullopt;
    const auto stride = qsizetype(size.width()) * BytesPerPixel;
    if (stride > std::numeric_limits<qsizetype>::max() / size.height()) return std::nullopt;
    QByteArray pixels(stride * size.height(), Qt::Uninitialized);
    const auto& gl = *state_->gl;
    GLint previousFramebuffer = 0, previousAlignment = 0, previousPackBuffer = 0;
    GLint previousRowLength = 0, previousRows = 0, previousPixels = 0;
    glGetIntegerv(ReadFramebufferBinding, &previousFramebuffer);
    glGetIntegerv(GL_PACK_ALIGNMENT, &previousAlignment);
    glGetIntegerv(GL_PACK_ROW_LENGTH, &previousRowLength);
    glGetIntegerv(GL_PACK_SKIP_ROWS, &previousRows);
    glGetIntegerv(GL_PACK_SKIP_PIXELS, &previousPixels);
    if (gl.bindBuffer) {
        glGetIntegerv(PackBufferBinding, &previousPackBuffer);
        if (previousPackBuffer) gl.bindBuffer(PackBuffer, 0);
    }
    gl.bindFramebuffer(ReadFramebuffer, framebuffer.id);
    glReadBuffer(ColorAttachment);
    glPixelStorei(GL_PACK_ALIGNMENT, PackAlignment);
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
    glPixelStorei(GL_PACK_SKIP_ROWS, 0);
    glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
    glReadPixels(0, 0, size.width(), size.height(), Bgra, GL_UNSIGNED_BYTE, pixels.data());
    glFinish();
    glPixelStorei(GL_PACK_ALIGNMENT, previousAlignment);
    glPixelStorei(GL_PACK_ROW_LENGTH, previousRowLength);
    glPixelStorei(GL_PACK_SKIP_ROWS, previousRows);
    glPixelStorei(GL_PACK_SKIP_PIXELS, previousPixels);
    if (previousPackBuffer) gl.bindBuffer(PackBuffer, GLuint(previousPackBuffer));
    gl.bindFramebuffer(ReadFramebuffer, GLuint(previousFramebuffer));
    if (state_->bottomLeftOrigin)
        for (int row = 0; row < size.height() / 2; ++row) {
            auto* top = pixels.data() + row * stride;
            auto* bottom = pixels.data() + (size.height() - row - 1) * stride;
            std::swap_ranges(top, top + stride, bottom);
        }
    return pixels;
#else
    Q_UNUSED(size)
    return std::nullopt;
#endif
}
}
