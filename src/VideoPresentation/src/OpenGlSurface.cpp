#include "OpenGlSurface.h"
#include "OpenGlSamplingShader.h"
#include "SoftwareProcessing.h"
#include "RestorationProcessing.h"
#include <QCoreApplication>
#include <QFile>
#include <QOffscreenSurface>
#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QPainter>
#include <QThread>
#include <QTimer>

namespace RetroMediaLab::VideoPresentation {
namespace {
constexpr int PositionAttribute = 0;
constexpr int PositionComponents = 2;
constexpr GLfloat Triangle[]{-1.f, -1.f, 3.f, -1.f, -1.f, 3.f};
constexpr const char* PositionName = "Position";
constexpr const char* SourceName = "Source";
constexpr const char* ProcessingName = "Processing";
constexpr const char* OutputName = "Output";
constexpr auto LegacyVertexInterface =
    "#define VERTEX_IN attribute\n#define VERTEX_OUT varying\n";
constexpr auto LegacyFragmentInterface =
    "#define FRAGMENT_IN varying\n#define DECLARE_COLOR\n#define FRAGMENT_COLOR gl_FragColor\n";
constexpr auto CoreVertexInterface =
    "#define VERTEX_IN in\n#define VERTEX_OUT out\n";
constexpr auto CoreFragmentInterface =
    "#define FRAGMENT_IN in\n#define DECLARE_COLOR out vec4 RenderedColor;\n"
    "#define FRAGMENT_COLOR RenderedColor\n#define texture2D texture\n";
constexpr auto GlesPrecision =
    "#ifdef GL_FRAGMENT_PRECISION_HIGH\nprecision highp float;\n#else\nprecision mediump float;\n#endif\n";
constexpr auto CrtFragmentInterface = R"glsl(
FRAGMENT_IN vec2 TextureCoordinate;
DECLARE_COLOR
struct CrtParameters {
    vec4 Output;
    vec4 Adjustments;
    vec4 Corrections;
    vec4 CrtRaster;
    vec4 CrtTuning;
    vec4 CrtPhosphor;
    vec4 CrtGeometry;
};
uniform CrtParameters Parameters;
uniform sampler2D Source;
uniform vec2 InputSize;

vec2 crtInputSize() { return InputSize; }

vec3 sourcePoint(vec2 coordinates)
{
    vec2 texel = clamp(floor(coordinates * InputSize), vec2(0.0), InputSize - 1.0);
    return texture2D(Source, (texel + .5) / InputSize).rgb;
}

vec3 sourceColor(vec2 coordinates) { return sourcePoint(coordinates); }
)glsl";
constexpr auto CrtFragmentMain = "void main() { FRAGMENT_COLOR = vec4(crtPixel(TextureCoordinate), 1.0); }";

std::unique_ptr<QOpenGLShaderProgram> linkProgram(QOpenGLContext* gl, bool crt = false) {
    const bool core = gl->format().profile() == QSurfaceFormat::CoreProfile;
    const auto* version = gl->isOpenGLES() ? "#version 100\n" : core ? "#version 150\n" : "#version 120\n";
    const QByteArray vertex = QByteArray(version) + (core ? CoreVertexInterface : LegacyVertexInterface) +
        OpenGlSamplingShader::Vertex;
    QByteArray fragment = QByteArray(version) + (gl->isOpenGLES() ? GlesPrecision : "") +
        (core ? CoreFragmentInterface : LegacyFragmentInterface);
    if (crt) fragment += CrtFragmentInterface;
    else for (const auto* part : OpenGlSamplingShader::FragmentParts) fragment += part;
    QFile adjustments(QStringLiteral(":/video-presentation/image-adjustments.glsl"));
    if (adjustments.open(QIODevice::ReadOnly)) fragment += adjustments.readAll();
    QFile restoration(QStringLiteral(":/video-presentation/image-restoration.glsl"));
    if (restoration.open(QIODevice::ReadOnly)) fragment += restoration.readAll();
    if (crt) {
        QFile shader(QStringLiteral(":/video-presentation/crt.glsl"));
        if (!shader.open(QIODevice::ReadOnly)) return {};
        fragment += shader.readAll();
        fragment += CrtFragmentMain;
    } else fragment += OpenGlSamplingShader::FragmentMain;
    if (!adjustments.isOpen() || !restoration.isOpen()) return {};
    auto program = std::make_unique<QOpenGLShaderProgram>();
    if (!program->addCacheableShaderFromSourceCode(QOpenGLShader::Vertex, vertex) ||
        !program->addCacheableShaderFromSourceCode(QOpenGLShader::Fragment, fragment)) return {};
    program->bindAttributeLocation(PositionName, PositionAttribute);
    return program->link() ? std::move(program) : nullptr;
}

class SharedShaderPreparation final : public QThread {
public:
    explicit SharedShaderPreparation(QOpenGLContext& display)
        : QThread(QCoreApplication::instance()), surface_(display.screen()), ownerThread_(display.thread()) {
        context_.setScreen(display.screen());
        context_.setFormat(display.format());
        context_.setShareContext(&display);
        if (!context_.create() || !QOpenGLContext::areSharing(&context_, &display)) return;
        surface_.setFormat(context_.format());
        surface_.create();
        available_ = surface_.isValid() && context_.moveToThread(this);
    }
    ~SharedShaderPreparation() override { wait(); }

    bool available() const noexcept { return available_; }
    bool acquiredContext() const noexcept { return acquiredContext_; }
    std::unique_ptr<QOpenGLShaderProgram> takeProgram() { return std::move(program_); }
    std::unique_ptr<QOpenGLShaderProgram> takeCrt() { return std::move(crt_); }

private:
    void run() override {
        acquiredContext_ = context_.makeCurrent(&surface_);
        if (acquiredContext_) {
            try {
                program_ = linkProgram(&context_);
                crt_ = linkProgram(&context_, true);
                if (program_ || crt_) {
                    context_.functions()->glFinish();
                    if (program_) program_->moveToThread(ownerThread_);
                    if (crt_) crt_->moveToThread(ownerThread_);
                }
            } catch (...) {
                program_.reset();
                crt_.reset();
            }
            context_.doneCurrent();
        }
        context_.moveToThread(ownerThread_);
    }

    QOpenGLContext context_;
    QOffscreenSurface surface_;
    QThread* ownerThread_;
    std::unique_ptr<QOpenGLShaderProgram> program_, crt_;
    bool available_ = false;
    bool acquiredContext_ = false;
};
}

class OpenGlSurface::Resources {
public:
    std::unique_ptr<QOpenGLShaderProgram> program, crt;
    QOpenGLVertexArrayObject vertexArray;
    QOpenGLBuffer vertices;
    std::unique_ptr<QOpenGLTexture> texture;
    QSize textureSize;
    qint64 cacheKey = 0;
    bool ready = false;
};

OpenGlSurface::OpenGlSurface(std::function<void(QPainter&)> draw, std::function<void()> samplingStarted, std::function<void()> samplingChanged, QWidget* parent)
    : QOpenGLWidget(parent), draw_(std::move(draw)), samplingStarted_(std::move(samplingStarted)), samplingChanged_(std::move(samplingChanged)) {
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

OpenGlSurface::~OpenGlSurface() {
    QObject::disconnect(contextCleanup_);
    releaseResources();
}

void OpenGlSurface::releaseResources() {
    ++preparationGeneration_;
    if (!resources_) return;
    makeCurrent();
    resources_.reset();
    doneCurrent();
}

void OpenGlSurface::initializeGL() {
    auto* gl = context();
    QObject::disconnect(contextCleanup_);
    contextCleanup_ = connect(gl, &QOpenGLContext::aboutToBeDestroyed, this,
        [this] { releaseResources(); }, Qt::DirectConnection);
    samplingStarted_();
    resources_ = std::make_unique<Resources>();
    const auto generation = ++preparationGeneration_;
    if (QOpenGLContext::supportsThreadedOpenGL()) {
        auto loading = std::make_unique<SharedShaderPreparation>(*gl);
        if (loading->available()) {
            auto* task = loading.release();
            connect(task, &QThread::finished, this, [this, task, generation] {
                if (generation != preparationGeneration_ || !isValid()) return;
                makeCurrent();
                if (task->acquiredContext()) installPrograms(task->takeProgram(), task->takeCrt());
                else installPrograms(linkProgram(context()), linkProgram(context(), true));
                doneCurrent();
                samplingChanged_();
            });
            connect(task, &QThread::finished, task, &QObject::deleteLater);
            task->start();
            return;
        }
    }
    installPrograms(linkProgram(gl), linkProgram(gl, true));
    QTimer::singleShot(0, this, [this, generation] {
        if (generation == preparationGeneration_) samplingChanged_();
    });
}

void OpenGlSurface::installPrograms(std::unique_ptr<QOpenGLShaderProgram> program, std::unique_ptr<QOpenGLShaderProgram> crt) {
    auto& r = *resources_;
    if ((!program && !crt) || !r.vertices.create() || !r.vertices.bind()) return;
    r.vertices.allocate(Triangle, sizeof(Triangle));
    r.vertices.release();
    const bool core = context()->format().profile() == QSurfaceFormat::CoreProfile;
    r.ready = r.vertexArray.create() || !core;
    r.program = std::move(program);
    r.crt = std::move(crt);
}

bool OpenGlSurface::canSample() const { return resources_ && resources_->ready && resources_->program; }
bool OpenGlSurface::canSampleCrt() const { return resources_ && resources_->ready && resources_->crt; }

bool OpenGlSurface::render(const QImage& image, QRect physicalViewport, Sampling sampling, ImageAdjustments settings, ImageRestoration restoration, const SoftwareProcessing::CrtShaderParameters& crt) {
    if ((crt.enabled() ? !canSampleCrt() : !canSample()) || image.isNull() || physicalViewport.isEmpty()) return false;
    auto& r = *resources_;
    if (r.textureSize != image.size()) {
        auto texture = std::make_unique<QOpenGLTexture>(QOpenGLTexture::Target2D);
        texture->setFormat(QOpenGLTexture::RGBA8_UNorm);
        texture->setSize(image.width(), image.height());
        texture->setMipLevels(1);
        texture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
        texture->setWrapMode(QOpenGLTexture::ClampToEdge);
        texture->allocateStorage();
        if (!texture->isStorageAllocated()) return false;
        r.texture = std::move(texture);
        r.textureSize = image.size();
        r.cacheKey = 0;
    }
    if (r.cacheKey != image.cacheKey()) {
        const auto pixels = image.convertToFormat(QImage::Format_RGBA8888);
        if (pixels.isNull()) return false;
        r.texture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, pixels.constBits());
        r.cacheKey = image.cacheKey();
    }
    auto* f = context()->functions();
    f->glViewport(physicalViewport.x(), physicalViewport.y(), physicalViewport.width(), physicalViewport.height());
    f->glDisable(GL_DEPTH_TEST);
    f->glDisable(GL_BLEND);
    f->glDisable(GL_CULL_FACE);
    f->glDisable(GL_SCISSOR_TEST);
    QOpenGLVertexArrayObject::Binder vertexArray(&r.vertexArray);
    auto& program = *(crt.enabled() ? r.crt : r.program);
    if (!program.bind()) return false;
    if (!r.vertices.bind()) {
        program.release();
        return false;
    }
    program.setUniformValue(SourceName, 0);
    if (crt.enabled()) settings.brightness = 0;
    const auto parameters = SoftwareProcessing::adjustmentParameters(settings);
    if (crt.enabled()) {
        const auto setCrtUniform = [&](const char* name, const std::array<float, 4>& values) {
            program.setUniformValue(name, values[0], values[1], values[2], values[3]);
        };
        setCrtUniform("Parameters.CrtRaster", crt.raster);
        setCrtUniform("Parameters.CrtTuning", crt.tuning);
        setCrtUniform("Parameters.CrtPhosphor", crt.phosphor);
        setCrtUniform("Parameters.CrtGeometry", crt.geometry);
        program.setUniformValue("Parameters.Corrections", parameters.sharpness, 0.f, 0.f, 0.f);
        program.setUniformValue("Parameters.Adjustments", parameters.brightness, parameters.contrast, parameters.gamma, parameters.saturation);
        program.setUniformValue("Parameters.Output", float(physicalViewport.width()), float(physicalViewport.height()), 0.f, 0.f);
        program.setUniformValue("InputSize", QVector2D(image.width(), image.height()));
    } else {
        program.setUniformValue("Adjustments", parameters.brightness, parameters.contrast, parameters.gamma, parameters.saturation);
        program.setUniformValue(ProcessingName, parameters.sharpness, float(sampling), float(image.width()), float(image.height()));
        const auto restorationValues = SoftwareProcessing::normalized(restoration);
        const auto maximum = float(ImageRestoration::MaximumIntensity);
        program.setUniformValue("Restoration", QVector2D(restorationValues.debanding / maximum,
            restorationValues.detailRecovery / maximum));
        program.setUniformValue(OutputName, float(physicalViewport.width()), float(physicalViewport.height()), 0.f, 0.f);
    }
    r.texture->bind(0);
    program.enableAttributeArray(PositionAttribute);
    program.setAttributeBuffer(PositionAttribute, GL_FLOAT, 0, PositionComponents);
    f->glDrawArrays(GL_TRIANGLES, 0, 3);
    program.disableAttributeArray(PositionAttribute);
    r.texture->release(0);
    r.vertices.release();
    program.release();
    return true;
}

QRect OpenGlSurface::physicalBounds(QRect bounds) const {
    const double ratio = devicePixelRatioF();
    const int left = qRound(bounds.x() * ratio), top = qRound(bounds.y() * ratio);
    const int right = qRound((bounds.x() + bounds.width()) * ratio);
    const int bottom = qRound((bounds.y() + bounds.height()) * ratio);
    return {left, top, right - left, bottom - top};
}

bool OpenGlSurface::drawSampled(QPainter& painter, const QImage& image, QRect bounds, Sampling sampling, ImageAdjustments settings, ImageRestoration restoration, const SoftwareProcessing::CrtShaderParameters& crt) {
    const auto framebuffer = size() * devicePixelRatioF();
    const auto viewport = physicalBounds(bounds);
    painter.beginNativePainting();
    const bool drawn = render(image, {viewport.x(), framebuffer.height() - viewport.y() - viewport.height(),
        viewport.width(), viewport.height()}, sampling, settings, restoration, crt);
    context()->functions()->glViewport(0, 0, framebuffer.width(), framebuffer.height());
    painter.endNativePainting();
    return drawn;
}

QImage OpenGlSurface::captureSampled(const QImage& image, QRect bounds, Sampling sampling, ImageAdjustments settings, ImageRestoration restoration, const SoftwareProcessing::CrtShaderParameters& crt) {
    const auto size = physicalBounds(bounds).size();
    if ((crt.enabled() ? !canSampleCrt() : !canSample()) || size.isEmpty() || !isValid()) return {};
    makeCurrent();
    QImage capture;
    {
        QOpenGLFramebufferObject target(size);
        if (target.isValid() && target.bind() && render(image, QRect(QPoint{}, size), sampling, settings, restoration, crt))
            capture = target.toImage();
    }
    context()->functions()->glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    doneCurrent();
    return capture;
}

void OpenGlSurface::paintGL() {
    QPainter painter(this);
    draw_(painter);
}

void OpenGlSurface::showEvent(QShowEvent* event) {
    QOpenGLWidget::showEvent(event);
    QTimer::singleShot(0, this, [this] {
        if (!isVisible() || isValid()) return;
        hide();
        parentWidget()->update();
    });
}
}
