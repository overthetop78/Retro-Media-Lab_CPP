#include "VulkanSurface.h"
#include <QWindow>
#include "HardwareSamplingShader.h"
#include <QPainter>
#include <QScopedValueRollback>
#include <QTimer>
#include <rhi/qrhi.h>

#if QT_CONFIG(vulkan)
#include "ShaderDiskCache.h"
#include <QCoreApplication>
#include <QFutureWatcher>
#include <QPointer>
#include <QPromise>
#include <QThreadPool>
#include <rhi/qrhi_platform.h>
#include <vector>
#endif

namespace RetroMediaLab::VideoPresentation {
namespace {
using namespace HardwareSamplingShader;
constexpr auto ShaderStages = QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;

#if QT_CONFIG(vulkan)
constexpr auto VulkanCacheRenderer = "Vulkan";
constexpr auto VulkanCacheVersion = "qrhi-pipeline-v1";
constexpr auto VulkanCacheExtension = ".bin";

struct PipelineTarget {
    QByteArray deviceName;
    quint64 deviceId;
    quint64 vendorId;
    QVersionNumber apiVersion;
    QRhiTexture::Format format;
    bool depthStencil;

    bool operator==(const PipelineTarget&) const = default;
    bool matches(const QRhiDriverInfo& driver) const {
        return deviceId == driver.deviceId && vendorId == driver.vendorId && deviceName == driver.deviceName;
    }
};

QString pipelineCachePath(const PipelineTarget& target, const QString& directory) {
    if (directory.isEmpty()) return {};
    QCryptographicHash hash(QCryptographicHash::Sha256);
    const auto add = [&](QByteArrayView component) {
        hash.addData(component);
        hash.addData(QByteArrayView("\0", 1));
    };
    add(VulkanCacheVersion);
    add(qVersion());
    add(target.deviceName);
    add(QByteArray::number(target.deviceId));
    add(QByteArray::number(target.vendorId));
    add(target.apiVersion.toString().toLatin1());
    add(QByteArray::number(int(target.format)));
    add(QByteArray::number(target.depthStencil));
    for (const auto path : {VertexShader, FragmentShader, CrtFragmentShader}) {
        QFile shader(QString::fromLatin1(path));
        if (!shader.open(QIODevice::ReadOnly)) return {};
        // The digest preserves shader boundaries when hashing the three artifacts.
        QCryptographicHash shaderHash(QCryptographicHash::Sha256);
        if (!shaderHash.addData(&shader)) return {};
        hash.addData(shaderHash.result());
    }
    return QDir(directory).filePath(QString::fromLatin1(hash.result().toHex()) +
                                    QLatin1String(VulkanCacheExtension));
}
#endif

struct Layer {
    std::unique_ptr<QRhiBuffer> parameters;
    std::unique_ptr<QRhiTexture> texture;
    std::unique_ptr<QRhiShaderResourceBindings> bindings;
    qint64 cacheKey = 0;
};
}

class VulkanSurface::Resources {
public:
    std::unique_ptr<QRhiSampler> point;
    std::unique_ptr<QRhiSampler> linear;
    Layer frame;
    Layer overlay;
    std::unique_ptr<QRhiShaderResourceBindings> layout;
    std::unique_ptr<QRhiRenderPassDescriptor> pass;
    std::unique_ptr<QRhiGraphicsPipeline> pipeline, crtPipeline;
    QRhi* device = nullptr;

    bool compatible(QRhi* rhi, QRhiRenderTarget& target) const {
        const auto* targetPass = target.renderPassDescriptor();
        return device == rhi && targetPass && pipeline->sampleCount() == target.sampleCount() &&
            pass->isCompatible(targetPass);
    }

    std::unique_ptr<QRhiShaderResourceBindings> bind(QRhiBuffer* parameters, QRhiTexture* texture) {
        auto bindings = std::unique_ptr<QRhiShaderResourceBindings>(device->newShaderResourceBindings());
        bindings->setBindings({
            QRhiShaderResourceBinding::uniformBuffer(ParametersBinding, ShaderStages, parameters),
            QRhiShaderResourceBinding::sampledTexture(PointSamplerBinding, QRhiShaderResourceBinding::FragmentStage, texture, point.get()),
            QRhiShaderResourceBinding::sampledTexture(LinearSamplerBinding, QRhiShaderResourceBinding::FragmentStage, texture, linear.get()),
        });
        if (!bindings->create()) return {};
        return bindings;
    }

    bool initialize(QRhi* rhi, QRhiRenderTarget* target) {
        device = rhi;
        const auto* targetPass = target->renderPassDescriptor();
        if (!targetPass) return false;
        pass.reset(targetPass->newCompatibleRenderPassDescriptor());
        static const auto vertex = readShader(VertexShader);
        static const auto fragment = readShader(FragmentShader);
        static const auto crt = readShader(CrtFragmentShader);
        if (!pass || !vertex.isValid() || !fragment.isValid()) return false;
        const auto sampler = [this](QRhiSampler::Filter filter) {
            auto result = std::unique_ptr<QRhiSampler>(device->newSampler(filter, filter, QRhiSampler::None,
                QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge));
            return result->create() ? std::move(result) : std::unique_ptr<QRhiSampler>{};
        };
        point = sampler(QRhiSampler::Nearest);
        linear = sampler(QRhiSampler::Linear);
        if (!point || !linear) return false;
        layout = bind(nullptr, nullptr);
        if (!layout) return false;
        const auto build = [&](const QShader& shader) -> std::unique_ptr<QRhiGraphicsPipeline> {
            if (!shader.isValid()) return {};
            auto result = std::unique_ptr<QRhiGraphicsPipeline>(device->newGraphicsPipeline());
            QRhiGraphicsPipeline::TargetBlend alpha;
            alpha.enable = true;
            alpha.srcColor = QRhiGraphicsPipeline::SrcAlpha;
            alpha.dstColor = QRhiGraphicsPipeline::OneMinusSrcAlpha;
            result->setTargetBlends({alpha});
            result->setShaderStages({{QRhiShaderStage::Vertex, vertex}, {QRhiShaderStage::Fragment, shader}});
            result->setShaderResourceBindings(layout.get());
            result->setSampleCount(target->sampleCount());
            result->setRenderPassDescriptor(pass.get());
            return result->create() ? std::move(result) : nullptr;
        };
        pipeline = build(fragment);
        if (!pipeline) return false;
        crtPipeline = build(crt);
        return true;
    }

#if QT_CONFIG(vulkan)
    static QByteArray buildCache(QVulkanInstance& instance, const PipelineTarget& target, const QByteArray& seed) {
        QRhiVulkanInitParams params;
        params.inst = &instance;
        std::unique_ptr<QRhi> device;
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
        std::vector<std::unique_ptr<QRhiAdapter>> adapters;
        for (auto* adapter : QRhi::enumerateAdapters(QRhi::Vulkan, &params)) adapters.emplace_back(adapter);
        for (const auto& adapter : adapters) {
            if (!target.matches(adapter->info())) continue;
            device.reset(QRhi::create(QRhi::Vulkan, &params, QRhi::EnablePipelineCacheDataSave, nullptr, adapter.get()));
            if (device) break;
        }
#else
        device.reset(QRhi::create(QRhi::Vulkan, &params, QRhi::EnablePipelineCacheDataSave));
#endif
        if (!device || !target.matches(device->driverInfo())) return {};
        if (!seed.isEmpty()) device->setPipelineCacheData(seed);

        constexpr QSize Extent(16, 16);
        constexpr int Samples = 1;
        std::unique_ptr<QRhiTexture> color(device->newTexture(target.format, Extent, Samples, QRhiTexture::RenderTarget));
        if (!color->create()) return {};
        std::unique_ptr<QRhiRenderBuffer> depth;
        if (target.depthStencil) {
            depth.reset(device->newRenderBuffer(QRhiRenderBuffer::DepthStencil, Extent, Samples));
            if (!depth->create()) return {};
        }
        const QRhiTextureRenderTargetDescription description(QRhiColorAttachment(color.get()), depth.get());
        std::unique_ptr<QRhiRenderPassDescriptor> pass;
        std::unique_ptr<QRhiTextureRenderTarget> renderTarget(device->newTextureRenderTarget(description));
        pass.reset(renderTarget->newCompatibleRenderPassDescriptor());
        if (!pass) return {};
        renderTarget->setRenderPassDescriptor(pass.get());
        if (!renderTarget->create()) return {};
        Resources sampler;
        return sampler.initialize(device.get(), renderTarget.get()) ? device->pipelineCacheData() : QByteArray{};
    }
#endif

    bool prepare(Layer& layer, const QImage& image, QRect area, Sampling sampling, bool isOverlay, ImageAdjustments adjustments, ImageRestoration restoration,
        const SoftwareProcessing::DisplayShaderParameters& crt, QRhiResourceUpdateBatch& updates) {
        if (image.isNull() || area.isEmpty()) return true;
        if (!layer.parameters) {
            auto parameters = std::unique_ptr<QRhiBuffer>(device->newBuffer(QRhiBuffer::Dynamic,
                QRhiBuffer::UniformBuffer, sizeof(Parameters)));
            if (!parameters->create()) return false;
            layer.parameters = std::move(parameters);
        }
        if (!layer.texture || layer.texture->pixelSize() != image.size()) {
            auto texture = std::unique_ptr<QRhiTexture>(device->newTexture(QRhiTexture::RGBA8, image.size()));
            if (!texture->create()) return false;
            auto bindings = bind(layer.parameters.get(), texture.get());
            if (!bindings) return false;
            layer.bindings = std::move(bindings);
            layer.texture = std::move(texture);
            layer.cacheKey = 0;
        }
        if (layer.cacheKey != image.cacheKey()) {
            const auto pixels = image.convertToFormat(QImage::Format_RGBA8888);
            if (pixels.isNull()) return false;
            updates.uploadTexture(layer.texture.get(), pixels);
            layer.cacheKey = image.cacheKey();
        }
        const auto parameters = makeParameters(area.size(), sampling, device->isYUpInNDC() ? 1 : -1, isOverlay, adjustments, restoration, crt);
        updates.updateDynamicBuffer(layer.parameters.get(), 0, sizeof(parameters), &parameters);
        return true;
    }

    void draw(QRhiCommandBuffer& commands, const Layer& layer, const QRect& area, int height) {
        commands.setViewport(QRhiViewport(float(area.x()), float(height - area.y() - area.height()),
            float(area.width()), float(area.height())));
        commands.setShaderResources(layer.bindings.get());
        commands.draw(3);
    }
};

#if QT_CONFIG(vulkan)
class VulkanSurface::Preparation {
public:
    explicit Preparation(PipelineTarget target) : target(std::move(target)) {}

    const PipelineTarget target;
    QFutureWatcher<QByteArray> watcher;
    QByteArray cache;
    bool ready = false;

    static QFuture<QByteArray> request(const PipelineTarget& target) {
        static QPointer<Cache> shared;
        if (!shared) shared = new Cache(target.apiVersion, QCoreApplication::instance());
        return shared->request(target);
    }

private:
    class Cache final : public QObject {
    public:
        Cache(const QVersionNumber& apiVersion, QObject* parent) : QObject(parent) {
            instance_.setApiVersion(apiVersion);
            instance_.setExtensions(QRhiVulkanInitParams::preferredInstanceExtensions());
            instance_.create();
            worker_.setMaxThreadCount(1);
        }

        QFuture<QByteArray> request(const PipelineTarget& target) {
            if (!instance_.isValid() || instance_.apiVersion() != target.apiVersion) return {};
            for (const auto& [key, future] : entries_)
                if (key == target) return future;

            QPromise<QByteArray> promise;
            auto future = promise.future();
            entries_.emplace_back(target, future);
            promise.start();
            worker_.start([this, target, promise = std::move(promise)]() mutable {
                QByteArray data;
                try {
                    using namespace ShaderDiskCache;
                    const auto directory = shaderCacheDirectory(QLatin1String(VulkanCacheRenderer));
                    const auto path = pipelineCachePath(target, directory);
                    const auto seed = readShaderBinary(path);
                    data = Resources::buildCache(instance_, target, seed);
                    if (data != seed) saveShaderBinary(directory, path, data);
                }
                catch (...) { /* An empty cache requests ordinary pipeline creation. */ }
                promise.addResult(std::move(data));
                promise.finish();
            });
            return future;
        }

    private:
        QVulkanInstance instance_;
        QThreadPool worker_;
        std::vector<std::pair<PipelineTarget, QFuture<QByteArray>>> entries_;
    };
};
#endif

VulkanSurface::VulkanSurface(std::function<void(QPainter&)> draw, std::function<void()> samplingStarted, std::function<void()> samplingChanged)
    : draw_(std::move(draw)), samplingStarted_(std::move(samplingStarted)), samplingChanged_(std::move(samplingChanged)) {
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowTransparentForInput | Qt::WindowDoesNotAcceptFocus);
    setApi(Api::Vulkan);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    connect(this, &QRhiWidget::frameSubmitted, this, [this] { submitted_ = true; });
    connect(this, &QRhiWidget::renderFailed, this, [this] {
        submitted_ = false;
        failed_ = true;
        QTimer::singleShot(0, this, [this] {
            hide();
            samplingChanged_();
        });
    });
}

VulkanSurface::~VulkanSurface() { releaseResources(); }

void VulkanSurface::attachTo(QWidget& host) {
    // Keep a separate QWidget backing store, with a native child window clipped to the host.
    const auto hostId = host.winId();
    const auto surfaceId = winId();
    if (!hostId || !surfaceId) return;
    windowHandle()->setParent(host.windowHandle());
    setGeometry(host.rect());
    if (host.isVisible()) showEmbedded();
}

void VulkanSurface::showEmbedded() {
    if (!failed_ && windowHandle() && windowHandle()->parent()) show();
}

bool VulkanSurface::isPresenting() const noexcept { return isVisible() && submitted_; }
bool VulkanSurface::canSample() const noexcept { return bool(resources_); }
bool VulkanSurface::canSampleDisplay() const noexcept { return resources_ && resources_->crtPipeline; }

void VulkanSurface::initialize(QRhiCommandBuffer*) {
    if (!rhi() || !renderTarget()) return;
    if (resources_ && resources_->compatible(rhi(), *renderTarget())) return;
#if QT_CONFIG(vulkan)
    const auto* color = colorTexture();
    if (color && renderTarget()->sampleCount() == 1 && rhi()->isFeatureSupported(QRhi::PipelineCacheDataLoadSave)) {
        const auto driver = rhi()->driverInfo();
        const auto* native = static_cast<const QRhiVulkanNativeHandles*>(rhi()->nativeHandles());
        const auto apiVersion = native && native->inst ? native->inst->apiVersion() : QVersionNumber{};
        const PipelineTarget target{driver.deviceName, driver.deviceId, driver.vendorId, apiVersion,
            color->format(), depthStencilBuffer() != nullptr};
        if (!preparation_ || preparation_->target != target) {
            resources_.reset();
            preparation_ = std::make_unique<Preparation>(target);
            auto* pending = preparation_.get();
            QTimer::singleShot(0, &pending->watcher, [this, pending] {
                samplingStarted_();
                auto future = Preparation::request(pending->target);
                if (!future.isValid()) {
                    pending->ready = true;
                    update();
                    return;
                }
                connect(&pending->watcher, &QFutureWatcher<QByteArray>::finished, &pending->watcher, [this, pending] {
                    pending->cache = pending->watcher.result();
                    pending->ready = true;
                    update();
                });
                pending->watcher.setFuture(future);
            });
            return;
        }
        if (!preparation_->ready) return;
        if (!preparation_->cache.isEmpty()) rhi()->setPipelineCacheData(preparation_->cache);
    }
    preparation_.reset();
#endif
    samplingStarted_();
    auto resources = std::make_unique<Resources>();
    resources_ = resources->initialize(rhi(), renderTarget()) ? std::move(resources) : nullptr;
    QTimer::singleShot(0, this, [this] { samplingChanged_(); });
}

void VulkanSurface::setSampledScene(QImage frame, QRect viewport, Sampling sampling, QImage overlay, ImageAdjustments adjustments, ImageRestoration restoration, const SoftwareProcessing::DisplayShaderParameters& crt) {
    sampledFrame_ = std::move(frame);
    viewport_ = viewport;
    sampling_ = sampling;
    adjustments_ = adjustments;
    restoration_ = restoration;
    crt_ = crt;
    overlay_ = std::move(overlay);
    update();
}

QRect VulkanSurface::physicalBounds(QRect bounds) const {
    const double ratio = devicePixelRatioF();
    const int left = qRound(bounds.x()*ratio), top = qRound(bounds.y()*ratio);
    return {left, top, qRound((bounds.x()+bounds.width())*ratio)-left,
        qRound((bounds.y()+bounds.height())*ratio)-top};
}

QImage VulkanSurface::captureSampled(const QImage& image, QRect viewport, Sampling sampling, ImageAdjustments adjustments, ImageRestoration restoration, const SoftwareProcessing::DisplayShaderParameters& crt) {
    if ((crt.enabled() ? !canSampleDisplay() : !canSample()) || image.isNull() || viewport.isEmpty()) return {};
    QScopedValueRollback frame(sampledFrame_, image);
    QScopedValueRollback bounds(viewport_, viewport);
    QScopedValueRollback mode(sampling_, sampling);
    QScopedValueRollback settings(adjustments_, adjustments);
    QScopedValueRollback recovery(restoration_, restoration);
    QScopedValueRollback display(crt_, crt);
    QScopedValueRollback capturing(capturing_, true);
    const auto capture = grabFramebuffer();
    return capture.isNull() || (crt.enabled() ? !canSampleDisplay() : !canSample()) ? QImage{} : capture.copy(physicalBounds(viewport));
}

void VulkanSurface::render(QRhiCommandBuffer* commands) {
#if QT_CONFIG(vulkan)
    if (preparation_ && preparation_->ready) initialize(commands);
#endif
    auto* texture = colorTexture();
    if (!texture || texture->pixelSize().isEmpty()) return;
    if (resources_ && !viewport_.isEmpty() && (!crt_.enabled() || canSampleDisplay())) {
        auto& r = *resources_;
        const QRect area = physicalBounds(viewport_);
        const QRect full(QPoint{}, texture->pixelSize());
        const auto& hud = capturing_ ? QImage{} : overlay_;
        auto* updates = rhi()->nextResourceUpdateBatch();
        if (r.prepare(r.frame, sampledFrame_, area, sampling_, false, adjustments_, restoration_, crt_, *updates) &&
            r.prepare(r.overlay, hud, full, Sampling::Nearest, true, {}, {}, {}, *updates)) {
            commands->beginPass(renderTarget(), Qt::black, {1.f, 0}, updates);
            if (!sampledFrame_.isNull() && !area.isEmpty()) {
                commands->setGraphicsPipeline(crt_.enabled() ? r.crtPipeline.get() : r.pipeline.get());
                r.draw(*commands, r.frame, area, full.height());
            }
            if (!hud.isNull()) {
                commands->setGraphicsPipeline(r.pipeline.get());
                r.draw(*commands, r.overlay, full, full.height());
            }
            commands->endPass();
            return;
        }
        updates->release();
        resources_.reset();
        QTimer::singleShot(0, this, [this] { samplingChanged_(); });
    }
    if (scene_.size() != texture->pixelSize())
        scene_ = QImage(texture->pixelSize(), QImage::Format_RGBA8888);
    if (scene_.isNull()) return;
    scene_.setDevicePixelRatio(devicePixelRatioF());
    {
        QPainter painter(&scene_);
        draw_(painter);
    }
    auto* updates = rhi()->nextResourceUpdateBatch();
    updates->uploadTexture(texture, scene_);
    commands->resourceUpdate(updates);
}

void VulkanSurface::releaseResources() {
#if QT_CONFIG(vulkan)
    preparation_.reset();
#endif
    submitted_ = false;
    resources_.reset();
    scene_ = {};
}
}
