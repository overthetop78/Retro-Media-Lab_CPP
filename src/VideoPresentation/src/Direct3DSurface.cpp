#include "Direct3DSurface.h"
#include "HardwareSamplingShader.h"
#include "SamplingFilters.h"
#include "ShaderDiskCache.h"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFutureWatcher>
#include <QPointer>
#include <QPromise>
#include <QThreadPool>
#include <QTimer>
#include <algorithm>
#include <cstring>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

namespace RetroMediaLab::VideoPresentation {
namespace {
using Microsoft::WRL::ComPtr;
using namespace HardwareSamplingShader;
using namespace ShaderDiskCache;
constexpr UINT SwapBufferCount = 2;
constexpr DXGI_FORMAT PixelFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
constexpr float Black[]{0.f, 0.f, 0.f, 1.f};
constexpr char VertexEntry[] = "vertexMain";
constexpr char PixelEntry[] = "pixelMain";
constexpr char VertexProfile[] = "vs_4_0";
constexpr char PixelProfile[] = "ps_4_0";
constexpr char SamplingVertexProfile[] = "vs_5_0";
constexpr char SamplingPixelProfile[] = "ps_5_0";
constexpr char CacheRenderer[] = "Direct3D11";
constexpr char CacheVersion[] = "native-dxbc-v1";
constexpr char CacheExtension[] = ".bin";
constexpr UINT CompileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
constexpr UINT DisplayFallbackCompileFlags = D3DCOMPILE_SKIP_OPTIMIZATION;
constexpr char DisplayShaderBytecode[] = ":/video-presentation/crt.frag.dxbc";
constexpr char PresentationShader[] = R"hlsl(
Texture2D Image : register(t0);
SamplerState ImageSampler : register(s0);

struct Vertex {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

Vertex vertexMain(uint index : SV_VertexID) {
    Vertex vertex;
    vertex.uv = float2((index << 1) & 2, index & 2);
    vertex.position = float4(vertex.uv * float2(2, -2) + float2(-1, 1), 0, 1);
    return vertex;
}

float4 pixelMain(Vertex vertex) : SV_TARGET {
    return Image.Sample(ImageSampler, vertex.uv);
}
)hlsl";

QByteArray compile(QByteArrayView source, const char* entry, const char* profile,
    UINT flags = CompileFlags, const QString& cacheDirectory = shaderCacheDirectory(QLatin1String(CacheRenderer))) {
    QString path;
    if (!cacheDirectory.isEmpty()) {
        QCryptographicHash hash(QCryptographicHash::Sha256);
        const auto add = [&](QByteArrayView input) {
            hash.addData(input);
            hash.addData(QByteArrayView("\0", 1));
        };
        add(CacheVersion);
        add(QByteArray::number(D3D_COMPILER_VERSION));
        add(QByteArray::number(flags));
        add(entry);
        add(profile);
        hash.addData(source);
        path = QDir(cacheDirectory).filePath(QString::fromLatin1(hash.result().toHex()) + QLatin1String(CacheExtension));
        if (const auto cached = readShaderBinary(path); !cached.isEmpty()) return cached;
    }
    ComPtr<ID3DBlob> bytecode;
    if (FAILED(D3DCompile(source.data(), size_t(source.size()), nullptr, nullptr, nullptr,
        entry, profile, flags, 0, bytecode.GetAddressOf(), nullptr))) return {};
    const QByteArray binary(static_cast<const char*>(bytecode->GetBufferPointer()), qsizetype(bytecode->GetBufferSize()));
    saveShaderBinary(cacheDirectory, path, binary);
    return binary;
}

struct SamplingProgram {
    QByteArray vertex, pixel;
    UINT vertexParameters = 0, pixelParameters = 0;
    UINT pointTexture = 0, pointSampler = 0, linearTexture = 0, linearSampler = 0;
};

QShaderKey samplingShaderKey(const QShader& shader) {
    const QShaderVersion version(50);
    const QShaderKey dxbc(QShader::DxbcShader, version, QShader::StandardShader);
    return shader.shader(dxbc).shader().isEmpty()
        ? QShaderKey(QShader::HlslShader, version, QShader::StandardShader) : dxbc;
}

QByteArray samplingShaderBytes(const QShader& shader, const QShaderKey& key, const char* profile,
    UINT flags = CompileFlags) {
    const auto code = shader.shader(key);
    if (key.source() == QShader::DxbcShader || code.shader().isEmpty()) return code.shader();
    return compile(code.shader(), code.entryPoint().constData(), profile, flags);
}

SamplingProgram loadSamplingProgram(const char* path, bool needsLinearSampler, const char* pixelBytecode = nullptr) {
    const auto vertexPack = readShader(VertexShader), fragmentPack = readShader(path);
    const auto vertexKey = samplingShaderKey(vertexPack), fragmentKey = samplingShaderKey(fragmentPack);
    const auto vertexMap = vertexPack.nativeResourceBindingMap(vertexKey);
    const auto fragmentMap = fragmentPack.nativeResourceBindingMap(fragmentKey);
    const auto vertexParameters = vertexMap.value(ParametersBinding, {-1, -1}).first;
    const auto pixelParameters = fragmentMap.value(ParametersBinding, {-1, -1}).first;
    const auto [pointTexture, pointSampler] = fragmentMap.value(PointSamplerBinding, {-1, -1});
    const auto [linearTexture, linearSampler] = fragmentMap.value(LinearSamplerBinding, {-1, -1});
    if ((std::min)({vertexParameters, pixelParameters, pointTexture, pointSampler}) < 0 ||
        (needsLinearSampler && (linearTexture < 0 || linearSampler < 0)))
        return {};
    QByteArray pixel;
    if (pixelBytecode) {
        QFile file(QString::fromLatin1(pixelBytecode));
        if (file.open(QIODevice::ReadOnly)) pixel = file.readAll();
    }
    if (pixel.isEmpty()) {
        const auto flags = pixelBytecode ? DisplayFallbackCompileFlags : CompileFlags;
        pixel = samplingShaderBytes(fragmentPack, fragmentKey, SamplingPixelProfile, flags);
    }
    return {
        samplingShaderBytes(vertexPack, vertexKey, SamplingVertexProfile),
        std::move(pixel),
        UINT(vertexParameters), UINT(pixelParameters), UINT(pointTexture), UINT(pointSampler), UINT(linearTexture), UINT(linearSampler)
    };
}

struct ShaderPrograms {
    QByteArray vertex, pixel;
    SamplingProgram sampling;
    SamplingProgram crt;
};
using SharedShaderPrograms = std::shared_ptr<const ShaderPrograms>;

class ShaderCompilation final : public QObject {
public:
    explicit ShaderCompilation(QObject* parent) : QObject(parent) {
        QPromise<SharedShaderPrograms> result;
        future = result.future();
        result.start();
        worker_.start([result = std::move(result)]() mutable {
            SharedShaderPrograms programs;
            try {
                programs = std::make_shared<const ShaderPrograms>(ShaderPrograms{
                    compile(PresentationShader, VertexEntry, VertexProfile),
                    compile(PresentationShader, PixelEntry, PixelProfile),
                    loadSamplingProgram(FragmentShader, true),
                    loadSamplingProgram(CrtFragmentShader, false, DisplayShaderBytecode)
                });
            } catch (...) {
                // A failed preparation leaves the ordinary software fallback available.
            }
            result.addResult(std::move(programs));
            result.finish();
        });
    }

    QFuture<SharedShaderPrograms> future;

private:
    QThreadPool worker_;
};

QFuture<SharedShaderPrograms> compilePrograms() {
    static QPointer<ShaderCompilation> compilation;
    if (!compilation) compilation = new ShaderCompilation(QCoreApplication::instance());
    return compilation->future;
}

struct Texture {
    ComPtr<ID3D11Texture2D> image;
    ComPtr<ID3D11ShaderResourceView> view;
    QSize size;
    qint64 cacheKey = 0;
};

class RendererContext {
public:
    HWND window = nullptr;
    QSize backBufferSize;
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<IDXGISwapChain> swapChain;
    ComPtr<ID3D11RenderTargetView> target;
    ComPtr<ID3D11VertexShader> vertex;
    ComPtr<ID3D11PixelShader> pixel;
    ComPtr<ID3D11SamplerState> nearest;
    ComPtr<ID3D11SamplerState> linear;
    ComPtr<ID3D11BlendState> alpha;
    ComPtr<ID3D11VertexShader> samplingVertex;
    ComPtr<ID3D11PixelShader> crtPixel;
    ComPtr<ID3D11PixelShader> samplingPixel;
    ComPtr<ID3D11Buffer> samplingParameters;
    const SamplingProgram* samplingProgram = nullptr;
    const SamplingProgram* crtProgram = nullptr;
    Texture frame;
    Texture overlay;

    bool initialize(HWND handle, QSize size, const ShaderPrograms& programs) {
        window = handle;
        DXGI_SWAP_CHAIN_DESC description{};
        description.BufferDesc.Width = UINT(size.width());
        description.BufferDesc.Height = UINT(size.height());
        description.BufferDesc.Format = PixelFormat;
        description.SampleDesc.Count = 1;
        description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        description.BufferCount = SwapBufferCount;
        description.OutputWindow = window;
        description.Windowed = TRUE;
        description.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        const auto create = [&](D3D_DRIVER_TYPE driver) {
            return SUCCEEDED(D3D11CreateDeviceAndSwapChain(nullptr, driver, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                nullptr, 0, D3D11_SDK_VERSION, &description, swapChain.ReleaseAndGetAddressOf(),
                device.ReleaseAndGetAddressOf(), nullptr, context.ReleaseAndGetAddressOf()));
        };
        if (!create(D3D_DRIVER_TYPE_HARDWARE) && !create(D3D_DRIVER_TYPE_WARP)) return false;
        const auto& vs = programs.vertex;
        const auto& ps = programs.pixel;
        if (vs.isEmpty() || ps.isEmpty() ||
            FAILED(device->CreateVertexShader(vs.constData(), size_t(vs.size()), nullptr, vertex.GetAddressOf())) ||
            FAILED(device->CreatePixelShader(ps.constData(), size_t(ps.size()), nullptr, pixel.GetAddressOf()))) return false;
        D3D11_SAMPLER_DESC sampler{};
        sampler.AddressU = sampler.AddressV = sampler.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampler.MaxAnisotropy = 1;
        sampler.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sampler.MaxLOD = D3D11_FLOAT32_MAX;
        sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        if (FAILED(device->CreateSamplerState(&sampler, nearest.GetAddressOf()))) return false;
        sampler.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        if (FAILED(device->CreateSamplerState(&sampler, linear.GetAddressOf()))) return false;
        D3D11_BLEND_DESC blend{};
        auto& output = blend.RenderTarget[0];
        output.BlendEnable = TRUE;
        output.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        output.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        output.BlendOp = D3D11_BLEND_OP_ADD;
        output.SrcBlendAlpha = D3D11_BLEND_ONE;
        output.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        output.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        output.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        if (FAILED(device->CreateBlendState(&blend, alpha.GetAddressOf()))) return false;
        backBufferSize = size;
        initializeSampling(programs.sampling, samplingPixel, samplingProgram);
        initializeSampling(programs.crt, crtPixel, crtProgram);
        return true;
    }

    void initializeSampling(const SamplingProgram& program, ComPtr<ID3D11PixelShader>& pixel,
        const SamplingProgram*& activeProgram) {
        if (device->GetFeatureLevel() < D3D_FEATURE_LEVEL_11_0) return;
        if (program.vertex.isEmpty() || program.pixel.isEmpty()) return;
        D3D11_BUFFER_DESC constants{};
        constants.ByteWidth = sizeof(Parameters);
        constants.Usage = D3D11_USAGE_DEFAULT;
        constants.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        if (!samplingVertex && FAILED(device->CreateVertexShader(program.vertex.constData(),
                size_t(program.vertex.size()), nullptr, samplingVertex.GetAddressOf()))) return;
        if (!samplingParameters && FAILED(device->CreateBuffer(&constants, nullptr, samplingParameters.GetAddressOf()))) return;
        if (SUCCEEDED(device->CreatePixelShader(program.pixel.constData(), size_t(program.pixel.size()),
                nullptr, pixel.GetAddressOf())))
            activeProgram = &program;
    }

    QRect physicalBounds(QRect viewport, QSize logicalSize) const {
        if (logicalSize.isEmpty()) return {};
        const auto scaleX = double(backBufferSize.width()) / logicalSize.width();
        const auto scaleY = double(backBufferSize.height()) / logicalSize.height();
        const auto left = qRound(viewport.x()*scaleX), top = qRound(viewport.y()*scaleY);
        return {left, top, qRound((viewport.x()+viewport.width())*scaleX)-left,
            qRound((viewport.y()+viewport.height())*scaleY)-top};
    }

    bool resize(QSize size) {
        if (size != backBufferSize) {
            context->OMSetRenderTargets(0, nullptr, nullptr);
            target.Reset();
            if (FAILED(swapChain->ResizeBuffers(SwapBufferCount, UINT(size.width()), UINT(size.height()), PixelFormat, 0)))
                return false;
            backBufferSize = size;
        }
        if (target) return true;
        ComPtr<ID3D11Texture2D> buffer;
        return SUCCEEDED(swapChain->GetBuffer(0, IID_PPV_ARGS(buffer.GetAddressOf()))) &&
            SUCCEEDED(device->CreateRenderTargetView(buffer.Get(), nullptr, target.GetAddressOf()));
    }

    bool upload(const QImage& source, Texture& destination) {
        if (destination.view && source.cacheKey() == destination.cacheKey) return true;
        if (destination.size != source.size()) {
            destination = {};
            D3D11_TEXTURE2D_DESC description{};
            description.Width = UINT(source.width());
            description.Height = UINT(source.height());
            description.MipLevels = description.ArraySize = description.SampleDesc.Count = 1;
            description.Format = PixelFormat;
            description.Usage = D3D11_USAGE_DYNAMIC;
            description.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            if (FAILED(device->CreateTexture2D(&description, nullptr, destination.image.GetAddressOf())) ||
                FAILED(device->CreateShaderResourceView(destination.image.Get(), nullptr, destination.view.GetAddressOf()))) return false;
            destination.size = source.size();
        }
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (FAILED(context->Map(destination.image.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) return false;
        for (int line = 0; line < source.height(); ++line) {
            auto* destinationLine = static_cast<char*>(mapped.pData) + size_t(line) * mapped.RowPitch;
            std::memcpy(destinationLine, source.constScanLine(line), size_t(source.width()) * sizeof(QRgb));
        }
        context->Unmap(destination.image.Get(), 0);
        destination.cacheKey = source.cacheKey();
        return true;
    }

    void draw(const QRectF& bounds, Texture& texture, ID3D11SamplerState* sampler, ID3D11BlendState* blend) {
        context->VSSetShader(vertex.Get(), nullptr, 0);
        context->PSSetShader(pixel.Get(), nullptr, 0);
        D3D11_VIEWPORT viewport{float(bounds.x()), float(bounds.y()), float(bounds.width()), float(bounds.height()), 0.f, 1.f};
        context->RSSetViewports(1, &viewport);
        context->OMSetBlendState(blend, nullptr, UINT_MAX);
        context->PSSetShaderResources(0, 1, texture.view.GetAddressOf());
        context->PSSetSamplers(0, 1, &sampler);
        context->Draw(3, 0);
        ID3D11ShaderResourceView* empty = nullptr;
        context->PSSetShaderResources(0, 1, &empty);
    }

    void drawSampled(const QRect& bounds, Sampling sampling, ImageAdjustments adjustments, ImageRestoration restoration, const SoftwareProcessing::DisplayShaderParameters& crt) {
        const auto& program = *(crt.enabled() ? crtProgram : samplingProgram);
        const D3D11_VIEWPORT viewport{float(bounds.x()), float(bounds.y()), float(bounds.width()), float(bounds.height()), 0.f, 1.f};
        const auto parameters = makeParameters(bounds.size(), sampling, 1, false, adjustments, restoration, crt);
        context->RSSetViewports(1, &viewport);
        context->OMSetBlendState(nullptr, nullptr, UINT_MAX);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->VSSetShader(samplingVertex.Get(), nullptr, 0);
        context->PSSetShader(crt.enabled() ? crtPixel.Get() : samplingPixel.Get(), nullptr, 0);
        context->UpdateSubresource(samplingParameters.Get(), 0, nullptr, &parameters, 0, 0);
        context->VSSetConstantBuffers(program.vertexParameters, 1, samplingParameters.GetAddressOf());
        context->PSSetConstantBuffers(program.pixelParameters, 1, samplingParameters.GetAddressOf());
        context->PSSetShaderResources(program.pointTexture, 1, frame.view.GetAddressOf());
        if (!crt.enabled()) context->PSSetShaderResources(program.linearTexture, 1, frame.view.GetAddressOf());
        context->PSSetSamplers(program.pointSampler, 1, nearest.GetAddressOf());
        if (!crt.enabled()) context->PSSetSamplers(program.linearSampler, 1, linear.GetAddressOf());
        context->Draw(3, 0);
        ID3D11ShaderResourceView* empty = nullptr;
        context->PSSetShaderResources(program.pointTexture, 1, &empty);
        if (!crt.enabled()) context->PSSetShaderResources(program.linearTexture, 1, &empty);
    }

    QImage capture(const QImage& video, QSize size, Sampling sampling, ImageAdjustments adjustments, ImageRestoration restoration, const SoftwareProcessing::DisplayShaderParameters& crt) {
        if (!(crt.enabled() ? crtProgram : samplingProgram) || size.isEmpty() || video.isNull() || !upload(video, frame)) return {};
        D3D11_TEXTURE2D_DESC description{};
        description.Width = UINT(size.width());
        description.Height = UINT(size.height());
        description.MipLevels = description.ArraySize = description.SampleDesc.Count = 1;
        description.Format = PixelFormat;
        description.Usage = D3D11_USAGE_DEFAULT;
        description.BindFlags = D3D11_BIND_RENDER_TARGET;
        ComPtr<ID3D11Texture2D> output, readback;
        ComPtr<ID3D11RenderTargetView> view;
        if (FAILED(device->CreateTexture2D(&description, nullptr, output.GetAddressOf())) ||
            FAILED(device->CreateRenderTargetView(output.Get(), nullptr, view.GetAddressOf()))) return {};
        description.Usage = D3D11_USAGE_STAGING;
        description.BindFlags = 0;
        description.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        if (FAILED(device->CreateTexture2D(&description, nullptr, readback.GetAddressOf()))) return {};
        QImage capture(size, QImage::Format_RGB32);
        if (capture.isNull()) return {};
        context->OMSetRenderTargets(1, view.GetAddressOf(), nullptr);
        drawSampled(QRect(QPoint{}, size), sampling, adjustments, restoration, crt);
        context->OMSetRenderTargets(0, nullptr, nullptr);
        context->CopyResource(readback.Get(), output.Get());
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (FAILED(context->Map(readback.Get(), 0, D3D11_MAP_READ, 0, &mapped))) return {};
        for (int row = 0; row < size.height(); ++row) {
            const auto* begin = static_cast<const char*>(mapped.pData) + size_t(row) * mapped.RowPitch;
            std::memcpy(capture.scanLine(row), begin, size_t(capture.bytesPerLine()));
        }
        context->Unmap(readback.Get(), 0);
        return capture;
    }

    bool present(const QImage& video, QRect viewport, Sampling sampling, const QImage& hud, QSize logicalSize, ImageAdjustments adjustments, ImageRestoration restoration, const SoftwareProcessing::DisplayShaderParameters& crt) {
        if (!video.isNull() && !upload(video, frame)) return false;
        if (!hud.isNull() && !upload(hud, overlay)) return false;
        context->OMSetRenderTargets(1, target.GetAddressOf(), nullptr);
        context->ClearRenderTargetView(target.Get(), Black);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        const auto bounds = physicalBounds(viewport, logicalSize);
        if (!video.isNull() && !bounds.isEmpty()) {
            if (crt.enabled() ? crtProgram != nullptr : samplingProgram &&
                (usesSoftwareSampling(sampling) || adjustments != ImageAdjustments{} || restoration != ImageRestoration{}))
                drawSampled(bounds, sampling, adjustments, restoration, crt);
            else
                draw(bounds, frame, sampling == Sampling::Linear ? linear.Get() : nearest.Get(), nullptr);
        }
        if (!hud.isNull()) draw(QRectF(QPointF{}, QSizeF(backBufferSize)), overlay, linear.Get(), alpha.Get());
        return SUCCEEDED(swapChain->Present(0, 0));
    }
};
}

struct Direct3DSurface::Data {
    QFutureWatcher<SharedShaderPrograms> preparation;
    SharedShaderPrograms programs;
    std::unique_ptr<RendererContext> renderer;
    QImage frame, overlay;
    QRect viewport;
    Sampling sampling = Sampling::Nearest;
    ImageAdjustments adjustments;
    ImageRestoration restoration;
    SoftwareProcessing::DisplayShaderParameters crt;
    bool presenting = false;
    bool failed = false;
    bool preparing = false;
    bool prepared = false;
};

Direct3DSurface::Direct3DSurface(std::function<void()> samplingStarted, std::function<void()> samplingChanged,
    std::function<void()> rendererFailed, QWidget* parent)
    : QWidget(parent), data_(std::make_unique<Data>()), samplingStarted_(std::move(samplingStarted)), samplingChanged_(std::move(samplingChanged)),
      rendererFailed_(std::move(rendererFailed)) {
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_DontCreateNativeAncestors);
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    connect(&data_->preparation, &QFutureWatcher<SharedShaderPrograms>::finished, this, [this] {
        data_->programs = data_->preparation.result();
        data_->prepared = true;
        if (isVisible()) presentNow();
    });
}

Direct3DSurface::~Direct3DSurface() = default;

void Direct3DSurface::setScene(QImage frame, QRect viewport, Sampling sampling, QImage overlay, ImageAdjustments adjustments, ImageRestoration restoration, const SoftwareProcessing::DisplayShaderParameters& crt) {
    data_->frame = std::move(frame);
    data_->overlay = std::move(overlay);
    data_->viewport = viewport;
    data_->sampling = sampling;
    data_->adjustments = adjustments;
    data_->restoration = restoration;
    data_->crt = crt;
    if (isVisible()) presentNow();
}

bool Direct3DSurface::isPresenting() const noexcept { return isVisible() && data_->presenting; }
bool Direct3DSurface::canSample() const noexcept { return data_->renderer && data_->renderer->samplingProgram; }

bool Direct3DSurface::canSampleDisplay() const noexcept { return data_->renderer && data_->renderer->crtProgram; }

QImage Direct3DSurface::captureSampled(const QImage& image, QRect viewport, Sampling sampling, ImageAdjustments adjustments, ImageRestoration restoration, const SoftwareProcessing::DisplayShaderParameters& crt) {
    if (crt.enabled() ? !canSampleDisplay() : !canSample()) return {};
    const auto physical = data_->renderer->physicalBounds(viewport, size());
    return data_->renderer->capture(image, physical.size(), sampling, adjustments, restoration, crt);
}
QPaintEngine* Direct3DSurface::paintEngine() const { return nullptr; }

void Direct3DSurface::paintEvent(QPaintEvent*) { presentNow(); }

void Direct3DSurface::presentNow() {
    if (data_->failed) return;
    const auto handle = reinterpret_cast<HWND>(winId());
    RECT bounds{};
    if (!GetClientRect(handle, &bounds)) return;
    const QSize dimensions(int(bounds.right - bounds.left), int(bounds.bottom - bounds.top));
    if (dimensions.isEmpty()) return;
    const bool initializing = !data_->renderer || data_->renderer->window != handle;
    if (initializing) {
        if (!data_->preparing) {
            data_->preparing = true;
            samplingStarted_();
            if (!data_->prepared) {
                data_->preparation.setFuture(compilePrograms());
                return;
            }
        }
        if (!data_->prepared) return;
        if (data_->programs) {
            data_->renderer = std::make_unique<RendererContext>();
            if (!data_->renderer->initialize(handle, dimensions, *data_->programs)) data_->renderer.reset();
        }
        data_->preparing = false;
    }
    data_->presenting = data_->renderer && data_->renderer->resize(dimensions) &&
        data_->renderer->present(data_->frame, data_->viewport, data_->sampling, data_->overlay, size(), data_->adjustments, data_->restoration, data_->crt);
    if (data_->presenting) {
        if (initializing) QTimer::singleShot(0, this, [this] { samplingChanged_(); });
        return;
    }
    data_->renderer.reset();
    data_->failed = true;
    QTimer::singleShot(0, this, [this] {
        if (data_->presenting) return;
        hide();
        rendererFailed_();
    });
}
}
