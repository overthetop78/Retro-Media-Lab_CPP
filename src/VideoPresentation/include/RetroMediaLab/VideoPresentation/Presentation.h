#pragma once
#include <RetroMediaLab/Emulation/Sdk.h>
#include <QHash>
#include <QImage>
#include <QWidget>
#include <array>
#include <functional>
#include <span>

#if defined(RETROMEDIALAB_VIDEO_PRESENTATION_BUILD)
#define RETROMEDIALAB_VIDEO_PRESENTATION_API Q_DECL_EXPORT
#else
#define RETROMEDIALAB_VIDEO_PRESENTATION_API Q_DECL_IMPORT
#endif

namespace RetroMediaLab::VideoPresentation {
class OpenGlSurface;
class VulkanSurface;
class Direct3DSurface;
enum class Renderer { Direct3D11, Vulkan, OpenGL, Software };
enum class AspectRatio { Original, Square, FourThree, FiveFour, SixteenNine, FourteenNine, TwentyOneNine };
enum class Sampling {
    Nearest = 0, Linear = 1, SharpBilinear = 2, Bicubic = 3, Xbr = 4, Xbrz = 5,
    Hqx = 6, ScaleFx = 7, ScaleNx = 8, Sabr = 9, Hq2x = 10, Hq3x = 11, Hq4x = 12,
    TwoXSai = 13, SuperTwoXSai = 14, SuperEagle = 15, EpxScale2x = 16, Jinc2 = 17, Lanczos = 18
};
enum class Deinterlacing { Off, BobEvenLines, BobOddLines, Blend, Bob, Weave, Adaptive };
enum class SignalConnection { None, RgbScart, Component, SVideo, Composite, Rf };
enum class SignalStandard { Automatic, Pal, Ntsc, Secam };

struct SignalSimulation {
    static constexpr int MaximumIntensity = 100;
    SignalConnection connection = SignalConnection::None;
    int connectionIntensity = 0;
    SignalStandard standard = SignalStandard::Automatic;
    int standardIntensity = 0;
    bool operator==(const SignalSimulation&) const = default;
};

struct ImageAdjustments {
    static constexpr int Minimum = -20;
    static constexpr int Maximum = 20;
    static constexpr int Bt1886 = 1886;
    static constexpr std::array GammaCurves{0, 180, 190, 220, 240, Bt1886};
    int brightness = 0, contrast = 0, gamma = 0, saturation = 0, sharpness = 0;
    bool operator==(const ImageAdjustments&) const = default;
};

enum class CrtMask { None, ApertureGrille, ShadowMask, SlotMask, StaggeredSlotMask, Monochrome, HitachiEdp };
enum class CrtColorStandards { Pal = 1, Secam = 2, Ntsc = 4, PalSecam = 3, PalNtsc = 5, PalSecamNtsc = 7 };
enum class CrtFieldRate { Hz50, Hz60, Hz50And60 };
enum class CrtPhosphor { Gray, Green, Amber };
enum class CrtConnection { Digital, Scart, Composite, Component, SVideo, AnalogRf, LegacyMonitor };
enum class CrtMonitorPalette { Mda, Cga, Ega };

enum class PanelDefinition { Native, Vga, Svga, Xga, Hd, Sxga, HdReady, FullHd, Qhd, Uhd4K };

struct DisplaySignalProfile {
    static constexpr int MaximumConnectionDegradation = 20;
    CrtColorStandards colorStandards = CrtColorStandards::PalSecamNtsc;
    CrtFieldRate fieldRate = CrtFieldRate::Hz50And60;
    CrtConnection connection = CrtConnection::Digital;
    CrtMonitorPalette monitorPalette = CrtMonitorPalette::Mda;
    int connectionDegradation = 0;
    PanelDefinition definition = PanelDefinition::Native;
    RETROMEDIALAB_VIDEO_PRESENTATION_API QSize panelSize(QSize source) const;
    bool canDegrade() const { return connection >= CrtConnection::Scart && connection <= CrtConnection::AnalogRf; }
    bool mergesDithering() const { return connection == CrtConnection::Composite || connection == CrtConnection::AnalogRf; }
    bool operator==(const DisplaySignalProfile&) const = default;
};

struct CrtProfile : DisplaySignalProfile {
    static constexpr int MaximumCurvature = 20;
    bool enabled = false;
    CrtMask mask = CrtMask::None;
    CrtPhosphor phosphor = CrtPhosphor::Gray;
    int beamWidth = 0;
    int curvature = 0;
    bool operator==(const CrtProfile&) const = default;
};

enum class FixedPixelTechnology { Lcd, LedBacklitLcd, Oled };
enum class SubpixelLayout { Monochrome, Rgb, Bgr };
enum class MonochromePalette { Green, Gray, Amber, Blue, White };

struct FixedPixelProfile {
    static constexpr int PercentMaximum = 100;
    static constexpr int ResponseMaximumMilliseconds = 1000;
    int defaultBacklight() const { return technology == FixedPixelTechnology::LedBacklitLcd ? 80 : 65; }
    int defaultBlackDepth() const {
        constexpr std::array defaults{35, 55, 100};
        return defaults[size_t(technology)];
    }

    FixedPixelTechnology technology = FixedPixelTechnology::Lcd;
    SubpixelLayout subpixels = SubpixelLayout::Rgb;
    MonochromePalette palette = MonochromePalette::Green;
    int gridIntensity = 0;
    int pixelGap = 0;
    int responseTimeMilliseconds = 0;
    int persistenceIntensity = 0;
    std::optional<int> backlightIntensity;
    std::optional<int> blackDepth;
    int backlightBleedIntensity = 25;
    int oledEmissionIntensity = 100;
    bool operator==(const FixedPixelProfile&) const = default;
};

enum class LcdPanelType { Tn, Ips, Va };
enum class LedBacklightType { Edge, Direct, FullArray, MiniLed };

enum class LcdPixelStructure { None, Rgb, Bgr, Monochrome };
enum class LcdPixelOrientation { Vertical, Horizontal };

struct LcdProfile {
    LcdPanelType panelType = LcdPanelType::Ips;
    int intensity = 65;
    LcdPixelStructure pixelStructure = LcdPixelStructure::None;
    LcdPixelOrientation orientation = LcdPixelOrientation::Vertical;
    bool operator==(const LcdProfile&) const = default;
};

struct LedBacklightProfile : LcdProfile {
    static constexpr int DefaultIntensity = 80;
    LedBacklightType type = LedBacklightType::Edge;
    LedBacklightProfile() { intensity = DefaultIntensity; }
    bool operator==(const LedBacklightProfile&) const = default;
};

enum class OledPanelType { RgbOled, Woled, QdOled };
enum class OledPixelStructure { None, RgbVertical, BgrVertical, Wrgb, TriangularRgb, PenTileDiamond, Monochrome };

struct OledProfile {
    OledPanelType panelType = OledPanelType::RgbOled;
    OledPixelStructure pixelStructure = OledPixelStructure::RgbVertical;
    bool operator==(const OledProfile&) const = default;
};

struct ImageRestoration {
    static constexpr int MaximumIntensity = 100;
    static constexpr std::array DeditheringLevels{0, 33, 67, MaximumIntensity};
    int dedithering = 0;
    int debanding = 0;
    int detailRecovery = 0;
    bool operator==(const ImageRestoration&) const = default;
};

enum class PlasmaCellStructure { Stripe, Waffle, Delta, Monochrome };

struct PlasmaProfile {
    bool enabled = false;
    PlasmaCellStructure cellStructure = PlasmaCellStructure::Stripe;
    bool operator==(const PlasmaProfile&) const = default;
};

enum class VectorPhosphor { Color, Green, Amber, White };

struct VectorProfile {
    static constexpr int MinimumBeamWidth = -20;
    static constexpr int MaximumBeamWidth = 20;
    bool enabled = false;
    int beamWidth = 0;
    VectorPhosphor phosphorColor = VectorPhosphor::Color;
    int curvature = 0;
    bool operator==(const VectorProfile&) const = default;
};

enum class MatrixDefinition { Native, Size16x16, Size32x16, Size32x32, Size64x32, Size64x64, Size128x64, Size128x128, Size256x128, Size256x256 };
RETROMEDIALAB_VIDEO_PRESENTATION_API QSize matrixSize(MatrixDefinition definition, QSize source);

enum class DotMatrixShape { Round, Square, Rectangle };
enum class VfdColor { BlueGreen, Green, Blue, White, Amber, Red };

struct VfdProfile {
    bool enabled = false;
    VfdColor color = VfdColor::BlueGreen;
    MatrixDefinition definition = MatrixDefinition::Native;
    DotMatrixShape shape = DotMatrixShape::Round;
    bool operator==(const VfdProfile&) const = default;
};

enum class LedMatrixColor { Rgb, Red, Green, Amber, Blue, White, RedGreen };
enum class LedMatrixShape { Round, Square, RoundedSquare };
enum class DisplayTechnology { Normal, Crt, FixedPixel, Plasma, Vector, Vfd, LedMatrix, DotMatrix, SegmentDisplay, EPaper, Projection };
enum class ScreenType { Host, Crt, Lcd, LedBacklitLcd, Oled, Plasma, Projection, Vector, LedMatrix, DotMatrix, Vfd,
    SegmentDisplay, EPaper };
RETROMEDIALAB_VIDEO_PRESENTATION_API std::span<const AspectRatio> aspectRatioChoices(ScreenType screen);
RETROMEDIALAB_VIDEO_PRESENTATION_API QSize aspectRatioDimensions(AspectRatio ratio);
RETROMEDIALAB_VIDEO_PRESENTATION_API std::span<const CrtConnection> displayConnectionChoices(ScreenType screen);
RETROMEDIALAB_VIDEO_PRESENTATION_API std::span<const PanelDefinition> plasmaDefinitionChoices();

enum class ImageAdjustmentControl { Brightness, Contrast, Gamma, Saturation, Sharpness };
enum class DotMatrixPalette { Black, Gray, Green, Amber, Red, Blue, Rgb };
enum class DotMatrixTechnology { Reflective, Backlit, Emissive };

struct DotMatrixProfile {
    bool enabled = false;
    DotMatrixPalette palette = DotMatrixPalette::Green;
    DotMatrixShape shape = DotMatrixShape::Round;
    MatrixDefinition definition = MatrixDefinition::Native;
    DotMatrixTechnology technology = DotMatrixTechnology::Reflective;
    bool operator==(const DotMatrixProfile&) const = default;
};

struct LedMatrixProfile {
    bool enabled = false;
    LedMatrixColor color = LedMatrixColor::Rgb;
    LedMatrixShape shape = LedMatrixShape::Round;
    MatrixDefinition definition = MatrixDefinition::Native;
    bool operator==(const LedMatrixProfile&) const = default;
};

enum class SegmentLayout { Seven, Fourteen, Sixteen };
enum class SegmentColor { Red, Green, Amber, Blue, White, Black, Gray, BlueGreen };
enum class SegmentDisplayTechnology { Led, Lcd, Vfd };
RETROMEDIALAB_VIDEO_PRESENTATION_API std::span<const SegmentColor> segmentColors(SegmentDisplayTechnology);
enum class SegmentEndShape { Beveled, Rounded, Straight };

struct SegmentDisplayProfile {
    static constexpr int MaximumIntensity = 100;
    static constexpr int MaximumDurationMilliseconds = FixedPixelProfile::ResponseMaximumMilliseconds;
    bool enabled = false;
    SegmentLayout layout = SegmentLayout::Seven;
    SegmentColor color = SegmentColor::Red;
    SegmentEndShape endShape = SegmentEndShape::Beveled;
    bool decimalPoint = false;
    bool colon = false;
    int thickness = 55;
    int contrast = 80;
    int glow = 20;
    int responseTimeMilliseconds = 30;
    int cellSize = 45;
    int horizontalGap = 15;
    int verticalGap = 20;
    int segmentGap = 12;
    int brightness = 85;
    int activationThreshold = 45;
    int offSegmentVisibility = 8;
    int blackDepth = MaximumIntensity;
    int haloRadius = 25;
    int persistenceMilliseconds = 60;
    SegmentDisplayTechnology technology = SegmentDisplayTechnology::Led;
    bool operator==(const SegmentDisplayProfile&) const = default;
};

enum class EPaperColorMode { Monochrome, Grayscale16, Color4096, TricolorRed, TricolorYellow, Gallery };

struct EPaperProfile {
    EPaperColorMode colorMode = EPaperColorMode::Monochrome;
    bool enabled = false;
    PanelDefinition definition = PanelDefinition::Native;
    bool operator==(const EPaperProfile&) const = default;
};

enum class ProjectionTechnology { Crt, ThreeLcd, SingleChipDlp, ThreeChipDlp, Lcos };

enum class ProjectionSurface { WhiteScreen, GrayScreen, WhiteWall };

struct ProjectionProfile {
    static constexpr int MaximumAmbientLight = 20;
    bool enabled = false;
    ProjectionTechnology technology = ProjectionTechnology::ThreeLcd;
    ProjectionSurface surface = ProjectionSurface::WhiteScreen;
    int ambientLight = 0;
    bool operator==(const ProjectionProfile&) const = default;
};

struct Profile {
#ifdef Q_OS_WIN
    Renderer renderer = Renderer::Direct3D11;
#else
    Renderer renderer = Renderer::Software;
#endif
    AspectRatio aspectRatio = AspectRatio::Original;
    Sampling sampling = Sampling::Nearest;
    Deinterlacing deinterlacing = Deinterlacing::Off;
    ImageAdjustments adjustments;
    QHash<ScreenType, ImageAdjustments> screenAdjustments;
    SignalSimulation signal;
    ImageRestoration restoration;
    PlasmaProfile plasma;
    VectorProfile vector;
    VfdProfile vfd;
    LedMatrixProfile ledMatrix;
    DotMatrixProfile dotMatrix;
    SegmentDisplayProfile segmentDisplay;
    EPaperProfile ePaper;
    ProjectionProfile projection;
    CrtProfile crt;
    DisplaySignalProfile lcdSignal, ledSignal, oledSignal, plasmaSignal, projectionSignal;
    LcdProfile lcd;
    LedBacklightProfile ledBacklight;
    OledProfile oled;
    bool blackFrameInsertion = false;
    bool fixedPixelEnabled = false;
    FixedPixelProfile fixedPixel;
    std::array<std::optional<FixedPixelProfile>, 3> fixedPixelProfiles;
    QJsonObject settings;

    RETROMEDIALAB_VIDEO_PRESENTATION_API ScreenType screenType() const;
    RETROMEDIALAB_VIDEO_PRESENTATION_API void selectScreen(ScreenType);
    RETROMEDIALAB_VIDEO_PRESENTATION_API bool supportsDedithering() const;
    RETROMEDIALAB_VIDEO_PRESENTATION_API const DisplaySignalProfile* panelSignal() const;
    RETROMEDIALAB_VIDEO_PRESENTATION_API DisplaySignalProfile* panelSignal();
    RETROMEDIALAB_VIDEO_PRESENTATION_API bool supportsDeinterlacing() const;

    bool usesHostDisplay() const { return displayTechnology() == DisplayTechnology::Normal; }
    Sampling displaySampling() const { return usesHostDisplay() ? sampling : Sampling::Nearest; }
    Deinterlacing displayDeinterlacing() const { return supportsDeinterlacing() ? deinterlacing : Deinterlacing::Off; }

    RETROMEDIALAB_VIDEO_PRESENTATION_API ImageAdjustments imageAdjustments(ScreenType) const;
    RETROMEDIALAB_VIDEO_PRESENTATION_API void setImageAdjustments(ScreenType, ImageAdjustments);
    RETROMEDIALAB_VIDEO_PRESENTATION_API bool showsImageAdjustment(ImageAdjustmentControl) const;
    RETROMEDIALAB_VIDEO_PRESENTATION_API ImageAdjustments effectiveImageAdjustments() const;

    DisplayTechnology displayTechnology() const {
        if (crt.enabled) return DisplayTechnology::Crt;
        if (fixedPixelEnabled) return DisplayTechnology::FixedPixel;
        if (plasma.enabled) return DisplayTechnology::Plasma;
        if (vector.enabled) return DisplayTechnology::Vector;
        if (vfd.enabled) return DisplayTechnology::Vfd;
        if (ledMatrix.enabled) return DisplayTechnology::LedMatrix;
        if (dotMatrix.enabled) return DisplayTechnology::DotMatrix;
        if (segmentDisplay.enabled) return DisplayTechnology::SegmentDisplay;
        if (ePaper.enabled) return DisplayTechnology::EPaper;
        if (projection.enabled) return DisplayTechnology::Projection;
        return DisplayTechnology::Normal;
    }

    void setDisplayTechnology(DisplayTechnology value) {
        crt.enabled = value == DisplayTechnology::Crt;
        fixedPixelEnabled = value == DisplayTechnology::FixedPixel;
        plasma.enabled = value == DisplayTechnology::Plasma;
        vector.enabled = value == DisplayTechnology::Vector;
        vfd.enabled = value == DisplayTechnology::Vfd;
        ledMatrix.enabled = value == DisplayTechnology::LedMatrix;
        dotMatrix.enabled = value == DisplayTechnology::DotMatrix;
        segmentDisplay.enabled = value == DisplayTechnology::SegmentDisplay;
        ePaper.enabled = value == DisplayTechnology::EPaper;
        projection.enabled = value == DisplayTechnology::Projection;
    }
};

class RETROMEDIALAB_VIDEO_PRESENTATION_API ProfileStore {
public:
    explicit ProfileStore(QString directory, QString legacyDirectory = {});
    Profile load(const QString& module, const QUuid&) const;
    bool save(const QString& module, const QUuid&, const Profile&) const;
    bool copy(const QString& module, const QUuid& source, const QUuid& destination) const;
    bool remove(const QString& module, const QUuid&) const;
private:
    QString path(const QString& module, const QUuid&) const;
    QString legacyPath(const QString& module, const QUuid&) const;
    QString existingPath(const QString& module, const QUuid&) const;
    static Profile decode(QJsonObject);
    QString directory_;
    QString legacyDirectory_;
};

namespace SoftwareProcessing { class SoftwareFrameWorker; }

class RETROMEDIALAB_VIDEO_PRESENTATION_API VideoSurface : public QWidget {
public:
    explicit VideoSurface(QWidget* parent = nullptr);
    ~VideoSurface() override;
    void setProfile(Profile);
    void setEmulationRunning(bool);
    void submit(std::shared_ptr<const Emulation::VideoFrame>);
    void submit(std::shared_ptr<const Emulation::VideoFrame>, std::optional<Emulation::VideoSegmentLevels>);
    void submit(std::shared_ptr<const Emulation::VideoFrame>, std::optional<Emulation::VideoSegmentLevels>,
        Emulation::VideoVectorSegments);
    void submit(std::shared_ptr<const Emulation::VideoFrame>, std::optional<Emulation::VideoSegmentLevels>,
        Emulation::VideoVectorSegments, const QList<std::shared_ptr<const Emulation::VideoFrame>>& previousFields);
    void clear();
    void setStatistics(QString);
    void setOsdTransparency(int percent);
    void setIndicator(QString);
    void setShaderLoadingObserver(std::function<void(bool)>);
    bool isShaderLoading() const noexcept;
    QImage currentImage() const;
    static QImage softwareSnapshot(const Emulation::VideoFrame&, const Profile&, QSize viewport,
        const QList<std::shared_ptr<const Emulation::VideoFrame>>& previousFields,
        std::optional<Emulation::VideoSegmentLevels> = {}, Emulation::VideoVectorSegments = {});
    bool usesOpenGl() const;
    bool usesDirect3D11() const;
    bool usesVulkan() const;
protected:
    bool event(QEvent*) override;
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void showEvent(QShowEvent*) override;
    void hideEvent(QHideEvent*) override;
private:
    class RefreshState;
    std::unique_ptr<RefreshState> refresh_;
    void updateRefreshClock(bool enable);
    void presentAtRefresh(quint64 refresh, double hostHz);
    bool blackFrame_ = false;
    class TemporalState;
    std::unique_ptr<TemporalState> temporal_;
    std::unique_ptr<SoftwareProcessing::SoftwareFrameWorker> processing_;
    void acceptFrame(std::shared_ptr<const Emulation::VideoFrame>);
    void compose(bool advance = false);
    void refreshViewport();
    void presentImage(QImage, QSize sourceSize, double aspectRatio, std::optional<Sampling> hardwareSampling = {}, ImageAdjustments hardwareAdjustments = {},
        ImageRestoration hardwareRestoration = {}, DisplayTechnology hardwareDisplay = DisplayTechnology::Normal);
    void draw(QPainter&, bool openGl = false);
    void drawOverlay(QPainter&);
    QRect imageViewport() const;
    QRect imageViewport(const QSize& source, double aspectRatio) const;
    void scheduleUpdate();
    void setShaderLoading(bool);
    void samplingFinished(Renderer);
    OpenGlSurface* glSurface_ = nullptr;
    VulkanSurface* vulkanSurface_ = nullptr;
#ifdef Q_OS_WIN
    Direct3DSurface* d3dSurface_ = nullptr;
#endif
    QImage overlay_;
    int osdTransparency_ = 0;
    Profile profile_;
    std::shared_ptr<const Emulation::VideoFrame> frame_;
    QImage source_, previousField_, sameParity_, previousComplement_, image_;
    QSize imageSourceSize_;
    double imageAspectRatio_ = 0;
    std::optional<Sampling> hardwareSampling_;
    ImageAdjustments hardwareAdjustments_;
    QList<float> motion_;
    std::optional<quint64> motionSequence_;
    SignalStandard detectedSignal_ = SignalStandard::Ntsc;
    QString statistics_, indicator_;
};
}
