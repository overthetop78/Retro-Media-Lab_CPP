#include "SoftwareProcessing.h"
#include "SoftwareFrameWorker.h"
#include "SamplingFilters.h"
#include "BlackFrameCadence.h"
#include "CrtProcessing.h"
#include "CrtConnection.h"
#include "FixedPixelProcessing.h"
#include "StylisticProcessing.h"
#include "RestorationProcessing.h"
#include "LedMatrixProcessing.h"
#include "PlasmaProcessing.h"
#include "VectorProcessing.h"
#include "OpenGlSurface.h"
#include "VulkanSurface.h"
#ifdef Q_OS_WIN
#include "Direct3DSurface.h"
#include "NativeRefreshClock.h"
#endif
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QPainter>
#include <QRegularExpression>
#include <QSaveFile>
#include <QtEndian>
#include <algorithm>
#include <atomic>
#include <thread>
#include <array>
#include <cmath>
#include <limits>
#include <tuple>

namespace RetroMediaLab::VideoPresentation {
namespace {
constexpr auto RendererKey = "renderer";
constexpr std::array<const char*, 4> RendererNames{"Direct3D11", "Vulkan", "OpenGL", "Wpf"};
constexpr auto Processing = "processing";
constexpr auto Aspect = "aspectRatio";
constexpr auto Sample = "sampling";
constexpr auto BlackFrameInsertion = "blackFrameInsertion";
constexpr auto LegacyTemporal = "temporal";
constexpr auto Restoration = "restoration";
constexpr std::array RestorationFields{
    std::pair{"dedithering", &ImageRestoration::dedithering},
    std::pair{"debanding", &ImageRestoration::debanding},
    std::pair{"detailRecovery", &ImageRestoration::detailRecovery}
};
constexpr auto DeinterlacingKey = "deinterlacing";
constexpr auto TechnologyKey = "displayTechnology";
constexpr std::array<const char*, 11> TechnologyNames{"Normal", "Crt", "FixedPixel", "Plasma", "Vector", "Vfd", "LedMatrix", "DotMatrix", "SegmentDisplay", "EPaper", "Projection"};
constexpr auto EPaperColorModeKey = "colorMode";
constexpr std::array EPaperColorNames{"Monochrome", "Grayscale16", "Color4096"};
constexpr auto EPaperKey = "ePaper";
constexpr std::array EPaperFields{
    std::pair{"contrast", &EPaperProfile::contrast},
    std::pair{"dithering", &EPaperProfile::dithering},
    std::pair{"refreshTimeMilliseconds", &EPaperProfile::refreshTimeMilliseconds},
    std::pair{"ghosting", &EPaperProfile::ghosting},
    std::pair{"inkDensity", &EPaperProfile::inkDensity},
    std::pair{"paperBrightness", &EPaperProfile::paperBrightness},
    std::pair{"paperWarmth", &EPaperProfile::paperWarmth},
    std::pair{"colorSaturation", &EPaperProfile::colorSaturation},
    std::pair{"surfaceTexture", &EPaperProfile::surfaceTexture},
    std::pair{"edgeSoftness", &EPaperProfile::edgeSoftness}
};
constexpr auto ProjectionKey = "projection";
constexpr auto ProjectionTechnologyKey = "technology";
constexpr std::array<const char*, 5> ProjectionTechnologies{"Crt", "ThreeLcd", "SingleChipDlp", "ThreeChipDlp", "Lcos"};
constexpr auto ProjectionSurfaceKey = "surface";
constexpr auto ProjectionAmbientLightKey = "ambientLight";
constexpr std::array<const char*, 3> ProjectionSurfaces{"WhiteScreen", "GrayScreen", "WhiteWall"};
constexpr auto SegmentKey = "segmentDisplay";
constexpr auto SegmentLayoutKey = "layout";
constexpr auto SegmentEndShapeKey = "endShape";
constexpr std::array SegmentFlags{
    std::pair{"decimalPoint", &SegmentDisplayProfile::decimalPoint}, std::pair{"colon", &SegmentDisplayProfile::colon}
};
constexpr std::array<const char*, 3> SegmentLayoutNames{"Seven", "Fourteen", "Sixteen"};
constexpr std::array<const char*, 3> SegmentEndShapeNames{"Beveled", "Rounded", "Straight"};
constexpr std::array<const char*, 5> SegmentColorNames{"Red", "Green", "Amber", "Blue", "White"};
constexpr std::array SegmentFields{
    std::pair{"cellSize", &SegmentDisplayProfile::cellSize},
    std::pair{"horizontalGap", &SegmentDisplayProfile::horizontalGap},
    std::pair{"verticalGap", &SegmentDisplayProfile::verticalGap},
    std::pair{"thickness", &SegmentDisplayProfile::thickness},
    std::pair{"segmentGap", &SegmentDisplayProfile::segmentGap},
    std::pair{"brightness", &SegmentDisplayProfile::brightness},
    std::pair{"activationThreshold", &SegmentDisplayProfile::activationThreshold},
    std::pair{"contrast", &SegmentDisplayProfile::contrast},
    std::pair{"offSegmentVisibility", &SegmentDisplayProfile::offSegmentVisibility},
    std::pair{"blackDepth", &SegmentDisplayProfile::blackDepth},
    std::pair{"glow", &SegmentDisplayProfile::glow},
    std::pair{"haloRadius", &SegmentDisplayProfile::haloRadius},
    std::pair{"responseTimeMilliseconds", &SegmentDisplayProfile::responseTimeMilliseconds},
    std::pair{"persistenceMilliseconds", &SegmentDisplayProfile::persistenceMilliseconds}
};
constexpr auto DotMatrixKey = "dotMatrix";
constexpr auto DotPaletteKey = "palette";
constexpr auto DisplayShapeKey = "shape";
constexpr std::array<const char*, 7> DotPalettes{"Green", "Gray", "Amber", "Blue", "Rgb", "Red", "White"};
constexpr std::array<const char*, 3> DotShapes{"Round", "Square", "Rectangle"};
constexpr std::array DotFields{
    std::pair{"cellSize", &DotMatrixProfile::cellSize},
    std::pair{"dotSize", &DotMatrixProfile::dotSize},
    std::pair{"cellGap", &DotMatrixProfile::cellGap},
    std::pair{"brightness", &DotMatrixProfile::brightness},
    std::pair{"contrast", &DotMatrixProfile::contrast},
    std::pair{"haloIntensity", &DotMatrixProfile::haloIntensity},
    std::pair{"responseTimeMilliseconds", &DotMatrixProfile::responseTimeMilliseconds},
    std::pair{"persistenceMilliseconds", &DotMatrixProfile::persistenceMilliseconds}
};
constexpr auto UnknownTechnology = static_cast<DisplayTechnology>(-1);
constexpr auto LedMatrixKey = "ledMatrix";
constexpr std::array<const char*, 6> LedMatrixColors{"Rgb", "Red", "Green", "Amber", "Blue", "White"};
constexpr std::array<const char*, 2> LedMatrixShapes{"Round", "Square"};
constexpr std::array LedMatrixFields{
    std::pair{"cellSize", &LedMatrixProfile::cellSize},
    std::pair{"cellGap", &LedMatrixProfile::cellGap},
    std::pair{"diffusion", &LedMatrixProfile::diffusion},
    std::pair{"brightness", &LedMatrixProfile::brightness},
    std::pair{"haloRadius", &LedMatrixProfile::haloRadius},
    std::pair{"blackDepth", &LedMatrixProfile::blackDepth}
};
constexpr auto CrtKey = "crt";
constexpr auto CrtMaskKey = "mask";
constexpr auto CrtStandardsKey = "colorStandards";
constexpr auto CrtFieldRateKey = "fieldRate";
constexpr auto CrtPhosphorKey = "phosphorColor";
constexpr auto CrtDegradationKey = "connectionDegradation";
constexpr auto CrtPaletteKey = "monitorPalette";
constexpr std::array<const char*, 3> CrtPalettes{"Mda", "Cga", "Ega"};
constexpr auto CrtBeamKey = "beamWidth";
constexpr auto CrtCurvatureKey = "curvature";
constexpr auto PlasmaKey = "plasma";
constexpr auto CellStructureKey = "cellStructure";
constexpr auto VectorKey = "vector";
constexpr auto VfdKey = "vfd";
constexpr auto DisplayColorKey = "color";
constexpr auto VfdStructureKey = "structure";
constexpr std::array<const char*, 4> VfdColors{"Blue", "Green", "Amber", "Red"};
constexpr std::array<const char*, 2> VfdStructures{"Graphic", "DotMatrix"};
constexpr std::array VfdFields{
    std::pair{"phosphorIntensity", &VfdProfile::phosphorIntensity},
    std::pair{"emissionThreshold", &VfdProfile::emissionThreshold},
    std::pair{"glassDarkening", &VfdProfile::glassDarkening},
    std::pair{"cellSize", &VfdProfile::cellSize},
    std::pair{"cellGap", &VfdProfile::cellGap},
    std::pair{"haloIntensity", &VfdProfile::haloIntensity},
    std::pair{"haloRadius", &VfdProfile::haloRadius},
    std::pair{"persistenceMilliseconds", &VfdProfile::persistenceMilliseconds}
};
constexpr std::array<const char*, 5> VectorPhosphorNames{"Color", "Green", "Amber", "White", "Gray"};
constexpr std::array VectorFields{
    std::pair{"lineThreshold", &VectorProfile::lineThreshold},
    std::pair{"lineIntensity", &VectorProfile::lineIntensity},
    std::pair{"beamWidth", &VectorProfile::beamWidth},
    std::pair{"beamFocus", &VectorProfile::beamFocus},
    std::pair{"haloIntensity", &VectorProfile::haloIntensity},
    std::pair{"haloRadius", &VectorProfile::haloRadius},
    std::pair{"persistenceIntensity", &VectorProfile::persistenceIntensity}
};
constexpr std::array PlasmaCellStructures{"Stripe", "Waffle", "Delta", "Monochrome"};
constexpr auto FixedPixelKey = "fixedPixel";
constexpr std::array<const char*, 3> FixedPixelProfileKeys{"lcdProfile", "ledBacklitLcdProfile", "oledProfile"};
constexpr std::array<const char*, 3> FixedPixelNames{"Lcd", "LedBacklitLcd", "Oled"};
constexpr std::array<const char*, 3> SubpixelNames{"Monochrome", "Rgb", "Bgr"};
constexpr std::array<const char*, 5> PaletteNames{"Green", "Gray", "Amber", "Blue", "White"};
constexpr auto SubpixelsKey = "subpixels";
constexpr auto MonochromePaletteKey = "monochromePalette";
constexpr auto BacklightKey = "backlightIntensity";
constexpr auto BlackDepthKey = "blackDepth";
constexpr auto FixedPixelTypeKey = "technology";
constexpr std::array FixedPixelFields{
    std::pair{"gridIntensity", &FixedPixelProfile::gridIntensity},
    std::pair{"pixelGap", &FixedPixelProfile::pixelGap},
    std::pair{"responseTimeMilliseconds", &FixedPixelProfile::responseTimeMilliseconds},
    std::pair{"persistenceIntensity", &FixedPixelProfile::persistenceIntensity},
    std::pair{"backlightBleedIntensity", &FixedPixelProfile::backlightBleedIntensity},
    std::pair{"oledEmissionIntensity", &FixedPixelProfile::oledEmissionIntensity}
};
constexpr std::array<const char*, 7> MaskNames{"None","ApertureGrille","ShadowMask","SlotMask","StaggeredSlotMask","Monochrome","HitachiEdp"};
constexpr std::array<const char*, 8> CrtStandardNames{"","Pal","Secam","PalSecam","Ntsc","PalNtsc","","PalSecamNtsc"};
constexpr std::array<const char*, 3> CrtRateNames{"Hz50","Hz60","Hz50And60"};
constexpr std::array<const char*, 3> CrtPhosphorNames{"Gray","Green","Amber"};
constexpr std::array<const char*, 7> CrtConnections{"Digital","Scart","Composite","Component","SVideo","AnalogRf","LegacyMonitor"};
constexpr auto SignalKey = "signalSimulation";
constexpr auto StylisticKey = "stylistic";
constexpr auto SepiaKey = "sepia";
constexpr std::array StylisticFields{
    std::pair{"grain", &StylisticEffects::grain},
    std::pair{"vhs", &StylisticEffects::vhs},
    std::pair{"chromaticAberration", &StylisticEffects::chromaticAberration},
    std::pair{"bloom", &StylisticEffects::bloom}
};
constexpr auto ConnectionKey = "connection";
constexpr auto ConnectionIntensityKey = "connectionIntensity";
constexpr auto StandardKey = "standard";
constexpr auto StandardIntensityKey = "standardIntensity";
constexpr auto PalDurationThreshold = std::chrono::duration<double, std::milli>(18.2);
constexpr std::array<const char*, 6> ConnectionNames{"None","RgbScart","Component","SVideo","Composite","Rf"};
constexpr std::array<const char*, 4> StandardNames{"Automatic","Pal","Ntsc","Secam"};
constexpr auto LcdPanelKey = "lcd";
constexpr auto LcdOrientationKey = "orientation";
constexpr std::array<const char*, 4> LcdStructures{"None", "Rgb", "Bgr", "Monochrome"};
constexpr std::array<const char*, 2> LcdOrientations{"Vertical", "Horizontal"};
constexpr auto LedPanelKey = "led";
constexpr auto OledPanelKey = "oled";
constexpr auto PixelStructureKey = "pixelStructure";
constexpr std::array<const char*, 3> OledPanelTypes{"RgbOled", "Woled", "QdOled"};
constexpr std::array<const char*, 7> OledPixelStructures{
    "None", "RgbVertical", "BgrVertical", "Wrgb", "TriangularRgb", "PenTileDiamond", "Monochrome"
};
constexpr auto LcdPanelTypeKey = "panelType";
constexpr auto LedBacklightTypeKey = "backlightType";
constexpr std::array<const char*, 3> LcdPanelTypes{"Tn", "Ips", "Va"};
constexpr std::array<const char*, 4> LedBacklightTypes{"Edge", "Direct", "FullArray", "MiniLed"};
constexpr auto PanelDefinitionKey = "definition";
constexpr std::array<const char*, 10> PanelDefinitionNames{
    "Native", "Vga", "Svga", "Xga", "Hd", "Sxga", "HdReady", "FullHd", "Qhd", "Uhd4K"
};
constexpr std::array PanelDefinitionSizes{
    QSize{}, QSize{640, 480}, QSize{800, 600}, QSize{1024, 768}, QSize{1280, 720},
    QSize{1280, 1024}, QSize{1366, 768}, QSize{1920, 1080}, QSize{2560, 1440}, QSize{3840, 2160}
};
static_assert(PanelDefinitionSizes.size() == PanelDefinitionNames.size());
constexpr std::array PanelSignalFields{
    std::tuple{LcdPanelKey, &Profile::lcdSignal, ScreenType::Lcd},
    std::tuple{LedPanelKey, &Profile::ledSignal, ScreenType::LedBacklitLcd},
    std::tuple{OledPanelKey, &Profile::oledSignal, ScreenType::Oled},
    std::tuple{PlasmaKey, &Profile::plasmaSignal, ScreenType::Plasma},
    std::tuple{ProjectionKey, &Profile::projectionSignal, ScreenType::Projection}
};
constexpr auto AdjustmentsKey = "adjustments";
constexpr auto ScreenAdjustmentsKey = "screenAdjustments";
constexpr std::array ScreenAdjustmentNames{
    std::pair{ScreenType::Crt, "Crt"},
    std::pair{ScreenType::Lcd, "Lcd"},
    std::pair{ScreenType::LedBacklitLcd, "LedBacklitLcd"},
    std::pair{ScreenType::Oled, "Oled"},
    std::pair{ScreenType::Plasma, "Plasma"},
    std::pair{ScreenType::Projection, "Projection"},
    std::pair{ScreenType::Vector, "Vector"},
    std::pair{ScreenType::LedMatrix, "LedMatrix"},
    std::pair{ScreenType::DotMatrix, "DotMatrix"},
    std::pair{ScreenType::Vfd, "Vfd"},
    std::pair{ScreenType::SegmentDisplay, "SegmentDisplay"},
    std::pair{ScreenType::EPaper, "EPaper"}
};
constexpr std::array AdjustmentFields{
    std::pair{"brightness", &ImageAdjustments::brightness},
    std::pair{"contrast", &ImageAdjustments::contrast},
    std::pair{"gamma", &ImageAdjustments::gamma},
    std::pair{"saturation", &ImageAdjustments::saturation},
    std::pair{"sharpness", &ImageAdjustments::sharpness}
};
ImageAdjustments readImageAdjustments(const QJsonObject& data) {
    ImageAdjustments adjustments;
    for (const auto& [key, field] : AdjustmentFields)
        adjustments.*field = data.value(QLatin1String(key)).toInt();
    return SoftwareProcessing::normalized(adjustments);
}

QJsonObject writeImageAdjustments(ImageAdjustments adjustments, QJsonObject data) {
    adjustments = SoftwareProcessing::normalized(adjustments);
    for (const auto& [key, field] : AdjustmentFields)
        data.insert(QLatin1String(key), adjustments.*field);
    return data;
}

constexpr auto ProfileExtension = ".json";
constexpr auto MaximumProfileBytes = 1024 * 1024;
constexpr int OsdMargin = 12;
constexpr int OsdHorizontalPadding = 12;
constexpr int OsdVerticalPadding = 6;
constexpr int OsdAlpha = 208;
constexpr int OsdRadius = 5;

constexpr std::array<const char*, 7> AspectNames{"Original","Square","FourThree","FiveFour","SixteenNine","FourteenNine","TwentyOneNine"};
constexpr std::array<const char*, 19> SamplingNames{
    "Nearest", "Bilinear", "SharpBilinear", "Bicubic",
    "Xbr", "Xbrz", "Hqx", "ScaleFx", "ScaleNx", "Sabr", "Hq2x", "Hq3x", "Hq4x",
    "TwoXSai", "SuperTwoXSai", "SuperEagle", "EpxScale2x", "Jinc2", "Lanczos"
};
constexpr std::array<const char*, 7> DeinterlacingNames{"Off","BobEvenLines","BobOddLines","Blend","Bob","Weave","Adaptive"};

template<class Enum, std::size_t Count>
Enum readEnum(const QJsonValue& value, const std::array<const char*, Count>& names, Enum fallback) {
    const auto text = value.toString();
    for (std::size_t i = 0; i < names.size(); ++i)
        if (names[i] && text.compare(QLatin1String(names[i]), Qt::CaseInsensitive) == 0) return static_cast<Enum>(i);
    const int number = value.toInt(-1);
    return number >= 0 && std::size_t(number) < Count && names[std::size_t(number)] ? static_cast<Enum>(number) : fallback;
}

template<class Enum, std::size_t Count>
QString enumName(Enum value, const std::array<const char*, Count>& names) {
    const auto index = static_cast<std::size_t>(value);
    return QString::fromLatin1(names[index < Count && names[index] ? index : 0]);
}

DisplaySignalProfile readDisplaySignal(const QJsonObject& object) {
    DisplaySignalProfile signal;
    signal.colorStandards = readEnum(object.value(QLatin1String(CrtStandardsKey)), CrtStandardNames, signal.colorStandards);
    signal.fieldRate = readEnum(object.value(QLatin1String(CrtFieldRateKey)), CrtRateNames, signal.fieldRate);
    signal.connection = readEnum(object.value(QLatin1String(ConnectionKey)), CrtConnections, signal.connection);
    signal.monitorPalette = readEnum(object.value(QLatin1String(CrtPaletteKey)), CrtPalettes, signal.monitorPalette);
    signal.connectionDegradation = std::clamp(object.value(QLatin1String(CrtDegradationKey)).toInt(),
        0, DisplaySignalProfile::MaximumConnectionDegradation);
    signal.definition = readEnum(object.value(QLatin1String(PanelDefinitionKey)), PanelDefinitionNames, signal.definition);
    return signal;
}

void writeDisplaySignal(QJsonObject& object, const DisplaySignalProfile& signal) {
    object.insert(QLatin1String(PanelDefinitionKey), enumName(signal.definition, PanelDefinitionNames));
    object.insert(QLatin1String(CrtStandardsKey), enumName(signal.colorStandards, CrtStandardNames));
    object.insert(QLatin1String(CrtFieldRateKey), enumName(signal.fieldRate, CrtRateNames));
    object.insert(QLatin1String(ConnectionKey), enumName(signal.connection, CrtConnections));
    object.insert(QLatin1String(CrtPaletteKey), enumName(signal.monitorPalette, CrtPalettes));
    object.insert(QLatin1String(CrtDegradationKey), std::clamp(signal.connectionDegradation,
        0, DisplaySignalProfile::MaximumConnectionDegradation));
}

void readLcd(const QJsonObject& object, const FixedPixelProfile& fallback, LcdProfile& lcd) {
    constexpr std::array legacyStructures{LcdPixelStructure::Monochrome, LcdPixelStructure::Rgb, LcdPixelStructure::Bgr};
    const bool hasLegacyStructure = fallback.gridIntensity != 0 || fallback.subpixels == SubpixelLayout::Monochrome;
    const auto structure = hasLegacyStructure ? legacyStructures[size_t(fallback.subpixels)] : lcd.pixelStructure;
    lcd.pixelStructure = readEnum(object.value(QLatin1String(PixelStructureKey)), LcdStructures, structure);
    lcd.orientation = readEnum(object.value(QLatin1String(LcdOrientationKey)), LcdOrientations,
        hasLegacyStructure ? LcdPixelOrientation::Vertical : lcd.orientation);
    lcd.panelType = readEnum(object.value(QLatin1String(LcdPanelTypeKey)), LcdPanelTypes, lcd.panelType);
    lcd.intensity = std::clamp(object.value(QLatin1String(BacklightKey)).toInt(fallback.backlightIntensity.value_or(lcd.intensity)),
        0, FixedPixelProfile::PercentMaximum);
}

void writeLcd(QJsonObject& object, const LcdProfile& lcd) {
    object.insert(QLatin1String(PixelStructureKey), enumName(lcd.pixelStructure, LcdStructures));
    object.insert(QLatin1String(LcdOrientationKey), enumName(lcd.orientation, LcdOrientations));
    object.insert(QLatin1String(LcdPanelTypeKey), enumName(lcd.panelType, LcdPanelTypes));
    object.insert(QLatin1String(BacklightKey), std::clamp(lcd.intensity, 0, FixedPixelProfile::PercentMaximum));
}

FixedPixelProfile readFixedPixel(const QJsonObject& object, FixedPixelTechnology fallback = FixedPixelTechnology::Lcd) {
    FixedPixelProfile profile;
    profile.technology = readEnum(object.value(QLatin1String(FixedPixelTypeKey)), FixedPixelNames, fallback);
    profile.subpixels = readEnum(object.value(QLatin1String(SubpixelsKey)), SubpixelNames, profile.subpixels);
    profile.palette = readEnum(object.value(QLatin1String(MonochromePaletteKey)), PaletteNames, profile.palette);
    for (const auto& [key, field] : FixedPixelFields)
        profile.*field = object.value(QLatin1String(key)).toInt(profile.*field);
    if (const auto value = object.value(QLatin1String(BacklightKey)); value.isDouble()) profile.backlightIntensity = value.toInt();
    if (const auto value = object.value(QLatin1String(BlackDepthKey)); value.isDouble()) profile.blackDepth = value.toInt();
    return SoftwareProcessing::normalized(profile);
}

QJsonObject writeFixedPixel(const FixedPixelProfile& input, QJsonObject object) {
    const auto profile = SoftwareProcessing::normalized(input);
    object.insert(QLatin1String(FixedPixelTypeKey), enumName(profile.technology, FixedPixelNames));
    object.insert(QLatin1String(SubpixelsKey), enumName(profile.subpixels, SubpixelNames));
    object.insert(QLatin1String(MonochromePaletteKey), enumName(profile.palette, PaletteNames));
    for (const auto& [key, field] : FixedPixelFields) object.insert(QLatin1String(key), profile.*field);
    object.insert(QLatin1String(BacklightKey), profile.backlightIntensity ? QJsonValue(*profile.backlightIntensity) : QJsonValue());
    object.insert(QLatin1String(BlackDepthKey), profile.blackDepth ? QJsonValue(*profile.blackDepth) : QJsonValue());
    return object;
}

int expand5(int value) { return value * 255 / 31; }
int expand6(int value) { return value * 255 / 63; }

QImage decode(const Emulation::VideoFrame& frame) {
    using Emulation::PixelFormat;
    const int pixelBytes = frame.pixelFormat == PixelFormat::Xrgb8888 ? sizeof(quint32) : sizeof(quint16);
    if (frame.width <= 0 || frame.height <= 0 || frame.pitch <= 0 || qint64(frame.width) * pixelBytes > frame.pitch ||
        qint64(frame.height - 1) * frame.pitch + qint64(frame.width) * pixelBytes > frame.pixels.size())
        return {};
    QImage image(frame.width, frame.height, QImage::Format_RGB32);
    if (image.isNull()) return {};
    for (int y = 0; y < frame.height; ++y) {
        const auto* source = frame.pixels.constData() + qsizetype(y) * frame.pitch;
        auto* destination = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < frame.width; ++x, source += pixelBytes) {
            if (frame.pixelFormat == PixelFormat::Xrgb8888) {
                destination[x] = qFromLittleEndian<quint32>(source) | 0xff000000u;
            } else {
                const auto value = qFromLittleEndian<quint16>(source);
                const auto redShift = frame.pixelFormat == PixelFormat::Rgb565 ? 11 : 10;
                const auto green = frame.pixelFormat == PixelFormat::Rgb565 ? expand6((value >> 5) & 63) : expand5((value >> 5) & 31);
                destination[x] = qRgb(expand5((value >> redShift) & 31), green, expand5(value & 31));
            }
        }
    }
    return image;
}

QRect fit(const QSize& viewport, double ratio) {
    if (viewport.isEmpty() || !std::isfinite(ratio) || ratio <= 0.0) return {};
    const double h = std::min<double>(viewport.height(), viewport.width() / ratio);
    const double w = h * ratio;
    return {(viewport.width() - qRound(w)) / 2, (viewport.height() - qRound(h)) / 2, qRound(w), qRound(h)};
}

void paintOsd(QPainter& painter, const QRect& area, const QString& text, Qt::Alignment alignment) {
    if (text.isEmpty()) return;
    const auto bounds = painter.fontMetrics().boundingRect(area.adjusted(OsdMargin, OsdMargin, -OsdMargin, -OsdMargin),
        alignment | Qt::TextWordWrap, text);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, OsdAlpha));
    painter.drawRoundedRect(bounds.adjusted(-OsdHorizontalPadding, -OsdVerticalPadding, OsdHorizontalPadding, OsdVerticalPadding),
        OsdRadius, OsdRadius);
    painter.setPen(Qt::white);
    painter.drawText(bounds, alignment | Qt::TextWordWrap, text);
}
}

ProfileStore::ProfileStore(QString directory, QString legacyDirectory)
    : directory_(std::move(directory)), legacyDirectory_(std::move(legacyDirectory)) {}

namespace {
QString profilePath(const QString& directory, const QString& key, const QUuid& id) {
    if (directory.isEmpty() || key.isEmpty() || id.isNull()) return {};
    return QDir(QDir(directory).filePath(key)).filePath(id.toString(QUuid::Id128) + QLatin1String(ProfileExtension));
}

bool profileExists(const QString& path) {
    if (path.isEmpty()) return false;
    const QFileInfo info(path);
    return info.exists() || info.isSymLink();
}

std::optional<QJsonObject> readProfile(const QString& filename) {
    QFile file(filename);
    if (filename.isEmpty() || !file.open(QIODevice::ReadOnly) || file.size() > MaximumProfileBytes) return {};
    const auto data = file.read(MaximumProfileBytes + 1);
    if (data.size() > MaximumProfileBytes || file.error() != QFileDevice::NoError) return {};
    const auto document = QJsonDocument::fromJson(data);
    if (!document.isObject()) return {};
    return document.object();
}
}

std::span<const CrtConnection> displayConnectionChoices(ScreenType screen) {
    using enum CrtConnection;
    static constexpr std::array standard{Digital, Scart, Composite, Component, SVideo, AnalogRf, LegacyMonitor};
    static constexpr std::array oled{Digital, Composite, Component, AnalogRf, LegacyMonitor};
    static constexpr std::array projection{Digital, Composite, Component, SVideo, Scart};
    static constexpr std::array plasma{Digital, Scart, Composite, Component, SVideo, AnalogRf};
    switch (screen) {
    case ScreenType::Plasma: return plasma;
    case ScreenType::Oled: return oled;
    case ScreenType::Projection: return projection;
    default: return standard;
    }
}

std::span<const PanelDefinition> plasmaDefinitionChoices() {
    using enum PanelDefinition;
    static constexpr std::array definitions{Native, Vga, Xga, Hd, HdReady, FullHd};
    return definitions;
}

namespace {
CrtConnection normalizedDisplayConnection(ScreenType screen, CrtConnection connection) {
    const auto choices = displayConnectionChoices(screen);
    return std::ranges::find(choices, connection) == choices.end() ? CrtConnection::Digital : connection;
}

void normalizePanelInput(ScreenType screen, DisplaySignalProfile& signal) {
    signal.connection = normalizedDisplayConnection(screen, signal.connection);
    if (screen == ScreenType::Plasma) {
        const auto definitions = plasmaDefinitionChoices();
        if (std::ranges::find(definitions, signal.definition) == definitions.end())
            signal.definition = PanelDefinition::Native;
    }
}
}

QSize DisplaySignalProfile::panelSize(QSize source) const {
    const auto index = size_t(definition);
    return index > 0 && index < PanelDefinitionSizes.size() ? PanelDefinitionSizes[index] : source;
}

std::span<const AspectRatio> aspectRatioChoices(ScreenType screen) {
    using enum AspectRatio;
    static constexpr std::array original{Original};
    static constexpr std::array vector{Original, FourThree};
    static constexpr std::array matrix{Original, Square};
    static constexpr std::array paper{Original, FourThree, SixteenNine};
    static constexpr std::array standard{Original, FourThree, FiveFour, SixteenNine, FourteenNine, TwentyOneNine};
    switch (screen) {
    case ScreenType::Vector: return vector;
    case ScreenType::LedMatrix:
    case ScreenType::DotMatrix: return matrix;
    case ScreenType::Vfd:
    case ScreenType::SegmentDisplay: return original;
    case ScreenType::EPaper: return paper;
    default: return standard;
    }
}

QSize aspectRatioDimensions(AspectRatio ratio) {
    using enum AspectRatio;
    switch (ratio) {
    case Square: return {1, 1};
    case FourThree: return {4, 3};
    case FiveFour: return {5, 4};
    case SixteenNine: return {16, 9};
    case FourteenNine: return {14, 9};
    case TwentyOneNine: return {21, 9};
    default: return {};
    }
}

ScreenType Profile::screenType() const {
    switch (displayTechnology()) {
    case DisplayTechnology::Normal: return ScreenType::Host;
    case DisplayTechnology::Crt: return ScreenType::Crt;
    case DisplayTechnology::FixedPixel:
        switch (fixedPixel.technology) {
        case FixedPixelTechnology::LedBacklitLcd: return ScreenType::LedBacklitLcd;
        case FixedPixelTechnology::Oled: return ScreenType::Oled;
        default: return ScreenType::Lcd;
        }
    case DisplayTechnology::Plasma: return ScreenType::Plasma;
    case DisplayTechnology::Projection: return ScreenType::Projection;
    case DisplayTechnology::Vector: return ScreenType::Vector;
    case DisplayTechnology::Vfd: return ScreenType::Vfd;
    case DisplayTechnology::LedMatrix: return ScreenType::LedMatrix;
    case DisplayTechnology::DotMatrix: return ScreenType::DotMatrix;
    case DisplayTechnology::SegmentDisplay: return ScreenType::SegmentDisplay;
    case DisplayTechnology::EPaper: return ScreenType::EPaper;
    }
    return ScreenType::Host;
}

void Profile::selectScreen(ScreenType screen) {
    switch (screen) {
    case ScreenType::Host: setDisplayTechnology(DisplayTechnology::Normal); return;
    case ScreenType::Crt: setDisplayTechnology(DisplayTechnology::Crt); return;
    case ScreenType::Plasma: setDisplayTechnology(DisplayTechnology::Plasma); return;
    case ScreenType::Projection: setDisplayTechnology(DisplayTechnology::Projection); return;
    case ScreenType::Vector: setDisplayTechnology(DisplayTechnology::Vector); return;
    case ScreenType::Vfd: setDisplayTechnology(DisplayTechnology::Vfd); return;
    case ScreenType::LedMatrix: setDisplayTechnology(DisplayTechnology::LedMatrix); return;
    case ScreenType::DotMatrix: setDisplayTechnology(DisplayTechnology::DotMatrix); return;
    case ScreenType::SegmentDisplay: setDisplayTechnology(DisplayTechnology::SegmentDisplay); return;
    case ScreenType::EPaper: setDisplayTechnology(DisplayTechnology::EPaper); return;
    case ScreenType::Lcd:
    case ScreenType::LedBacklitLcd:
    case ScreenType::Oled:
        break;
    }
    auto technology = FixedPixelTechnology::Lcd;
    if (screen == ScreenType::LedBacklitLcd) technology = FixedPixelTechnology::LedBacklitLcd;
    if (screen == ScreenType::Oled) technology = FixedPixelTechnology::Oled;
    if (fixedPixel.technology != technology) {
        fixedPixelProfiles[size_t(fixedPixel.technology)] = fixedPixel;
        fixedPixel = fixedPixelProfiles[size_t(technology)].value_or(FixedPixelProfile{technology});
    }
    setDisplayTechnology(DisplayTechnology::FixedPixel);
}

bool Profile::supportsDeinterlacing() const {
    switch (displayTechnology()) {
    case DisplayTechnology::Normal:
    case DisplayTechnology::FixedPixel:
    case DisplayTechnology::Plasma:
    case DisplayTechnology::Projection:
        return true;
    default:
        return false;
    }
}

const DisplaySignalProfile* Profile::panelSignal() const {
    switch (screenType()) {
    case ScreenType::Lcd: return &lcdSignal;
    case ScreenType::LedBacklitLcd: return &ledSignal;
    case ScreenType::Oled: return &oledSignal;
    case ScreenType::Plasma: return &plasmaSignal;
    case ScreenType::Projection: return &projectionSignal;
    default: return nullptr;
    }
}

DisplaySignalProfile* Profile::panelSignal() {
    return const_cast<DisplaySignalProfile*>(std::as_const(*this).panelSignal());
}

bool Profile::supportsDedithering() const {
    return supportsDeinterlacing() || displayTechnology() == DisplayTechnology::Crt;
}

ImageAdjustments Profile::imageAdjustments(ScreenType screen) const {
    return SoftwareProcessing::normalized(screen == ScreenType::Host ? adjustments : screenAdjustments.value(screen));
}

void Profile::setImageAdjustments(ScreenType screen, ImageAdjustments value) {
    value = SoftwareProcessing::normalized(value);
    if (screen == ScreenType::Host) adjustments = value;
    else if (imageAdjustments(screen) != value) screenAdjustments.insert(screen, value);
}

bool Profile::showsImageAdjustment(ImageAdjustmentControl control) const {
    using Control = ImageAdjustmentControl;
    switch (screenType()) {
    case ScreenType::Vector:
    case ScreenType::Vfd:
        return control == Control::Brightness;
    case ScreenType::LedMatrix:
        return control == Control::Brightness || control == Control::Contrast || control == Control::Gamma ||
            (control == Control::Saturation && ledMatrix.color == LedMatrixColor::Rgb);
    case ScreenType::DotMatrix:
        return control == Control::Brightness || control == Control::Contrast ||
            (control == Control::Gamma && (dotMatrix.palette == DotMatrixPalette::Rgb || dotMatrix.palette == DotMatrixPalette::Gray)) ||
            (control == Control::Saturation && dotMatrix.palette == DotMatrixPalette::Rgb);
    case ScreenType::SegmentDisplay:
        return control == Control::Brightness || control == Control::Contrast;
    case ScreenType::EPaper:
        return control == Control::Contrast;
    default:
        return true;
    }
}

ImageAdjustments Profile::effectiveImageAdjustments() const {
    const auto value = imageAdjustments(screenType());
    using Control = ImageAdjustmentControl;
    return {
        showsImageAdjustment(Control::Brightness) ? value.brightness : 0,
        showsImageAdjustment(Control::Contrast) ? value.contrast : 0,
        showsImageAdjustment(Control::Gamma) ? value.gamma : 0,
        showsImageAdjustment(Control::Saturation) ? value.saturation : 0,
        showsImageAdjustment(Control::Sharpness) ? value.sharpness : 0
    };
}

QString ProfileStore::path(const QString& module, const QUuid& id) const {
    if (module.trimmed().isEmpty()) return {};
    const auto key = QString::fromLatin1(module.toUtf8().toHex().toUpper());
    return profilePath(directory_, key, id);
}

QString ProfileStore::legacyPath(const QString& module, const QUuid& id) const {
    static const QRegularExpression validModule(QStringLiteral("^[a-z0-9][a-z0-9_-]*$"));
    if (!validModule.match(module).hasMatch()) return {};
    return profilePath(legacyDirectory_, module, id);
}

QString ProfileStore::existingPath(const QString& module, const QUuid& id) const {
    const auto canonical = path(module, id);
    if (profileExists(canonical)) return canonical;
    const auto previous = legacyPath(module, id);
    return profileExists(previous) ? previous : QString{};
}

Profile ProfileStore::load(const QString& module, const QUuid& id) const {
    const auto settings = readProfile(existingPath(module, id));
    return settings ? decode(*settings) : Profile{};
}

Profile ProfileStore::decode(QJsonObject settings) {
    Profile profile;
    profile.settings = std::move(settings);
    profile.renderer = readEnum(profile.settings.value(QLatin1String(RendererKey)), RendererNames, profile.renderer);
    const auto processing = profile.settings.value(QLatin1String(Processing)).toObject();
    profile.aspectRatio = readEnum(processing.value(QLatin1String(Aspect)), AspectNames, profile.aspectRatio);
    profile.sampling = readEnum(processing.value(QLatin1String(Sample)), SamplingNames, profile.sampling);
    const auto blackFrame = processing.value(QLatin1String(BlackFrameInsertion));
    profile.blackFrameInsertion = blackFrame.isUndefined()
        ? processing.value(QLatin1String(LegacyTemporal)).toObject().value(QLatin1String(BlackFrameInsertion)).toBool()
        : blackFrame.toBool();
    const auto restoration = processing.value(QLatin1String(Restoration)).toObject();
    profile.deinterlacing = readEnum(restoration.value(QLatin1String(DeinterlacingKey)),
        DeinterlacingNames, profile.deinterlacing);
    for (const auto& [key, field] : RestorationFields) profile.restoration.*field = restoration.value(QLatin1String(key)).toInt();
    profile.restoration = SoftwareProcessing::normalized(profile.restoration);
    profile.adjustments = readImageAdjustments(processing.value(QLatin1String(AdjustmentsKey)).toObject());
    const auto screens = processing.value(QLatin1String(ScreenAdjustmentsKey)).toObject();
    for (const auto& [screen, name] : ScreenAdjustmentNames) {
        const auto values = screens.value(QLatin1String(name));
        if (values.isObject()) profile.screenAdjustments.insert(screen, readImageAdjustments(values.toObject()));
    }
    const auto signal = processing.value(QLatin1String(SignalKey)).toObject();
    profile.signal = SoftwareProcessing::normalized(SignalSimulation{
        readEnum(signal.value(QLatin1String(ConnectionKey)), ConnectionNames, profile.signal.connection),
        signal.value(QLatin1String(ConnectionIntensityKey)).toInt(),
        readEnum(signal.value(QLatin1String(StandardKey)), StandardNames, profile.signal.standard),
        signal.value(QLatin1String(StandardIntensityKey)).toInt()});
    const auto stylistic = processing.value(QLatin1String(StylisticKey)).toObject();
    for (const auto& [key, field] : StylisticFields) profile.stylistic.*field = stylistic.value(QLatin1String(key)).toInt();
    profile.stylistic.sepia = stylistic.value(QLatin1String(SepiaKey)).toBool();
    profile.stylistic = SoftwareProcessing::normalized(profile.stylistic);
    const auto technology = processing.value(QLatin1String(TechnologyKey));
    profile.setDisplayTechnology(readEnum(technology, TechnologyNames, DisplayTechnology::Normal));
    const auto segment = processing.value(QLatin1String(SegmentKey)).toObject();
    for (const auto& [key, field] : SegmentFields)
        profile.segmentDisplay.*field = segment.value(QLatin1String(key)).toInt(profile.segmentDisplay.*field);
    for (const auto& [key, flag] : SegmentFlags)
        profile.segmentDisplay.*flag = segment.value(QLatin1String(key)).toBool(profile.segmentDisplay.*flag);
    profile.segmentDisplay.layout = readEnum(segment.value(QLatin1String(SegmentLayoutKey)), SegmentLayoutNames, profile.segmentDisplay.layout);
    profile.segmentDisplay.endShape = readEnum(segment.value(QLatin1String(SegmentEndShapeKey)), SegmentEndShapeNames, profile.segmentDisplay.endShape);
    profile.segmentDisplay.color = readEnum(segment.value(QLatin1String(DisplayColorKey)), SegmentColorNames, profile.segmentDisplay.color);
    profile.segmentDisplay = SoftwareProcessing::normalized(profile.segmentDisplay);
    const auto ePaper = processing.value(QLatin1String(EPaperKey)).toObject();
    for (const auto& [key, member] : EPaperFields)
        profile.ePaper.*member = ePaper.value(QLatin1String(key)).toInt(profile.ePaper.*member);
    profile.ePaper.colorMode = readEnum(ePaper.value(QLatin1String(EPaperColorModeKey)), EPaperColorNames, profile.ePaper.colorMode);
    profile.ePaper = SoftwareProcessing::normalized(profile.ePaper);
    const auto projection = processing.value(QLatin1String(ProjectionKey)).toObject();
    profile.projection.ambientLight = projection.value(QLatin1String(ProjectionAmbientLightKey)).toInt();
    profile.projection.surface = readEnum(projection.value(QLatin1String(ProjectionSurfaceKey)),
        ProjectionSurfaces, profile.projection.surface);
    profile.projection.technology = readEnum(projection.value(QLatin1String(ProjectionTechnologyKey)),
        ProjectionTechnologies, profile.projection.technology);
    profile.projection = SoftwareProcessing::normalized(profile.projection);
    const auto dot = processing.value(QLatin1String(DotMatrixKey)).toObject();
    for (const auto& [key, field] : DotFields) profile.dotMatrix.*field = dot.value(QLatin1String(key)).toInt(profile.dotMatrix.*field);
    profile.dotMatrix.palette = readEnum(dot.value(QLatin1String(DotPaletteKey)), DotPalettes, profile.dotMatrix.palette);
    profile.dotMatrix.shape = readEnum(dot.value(QLatin1String(DisplayShapeKey)), DotShapes, profile.dotMatrix.shape);
    profile.dotMatrix = SoftwareProcessing::normalized(profile.dotMatrix);
    const auto ledMatrix = processing.value(QLatin1String(LedMatrixKey)).toObject();
    for (const auto& [key, field] : LedMatrixFields) profile.ledMatrix.*field = ledMatrix.value(QLatin1String(key)).toInt(profile.ledMatrix.*field);
    profile.ledMatrix.color = readEnum(ledMatrix.value(QLatin1String(DisplayColorKey)), LedMatrixColors, profile.ledMatrix.color);
    profile.ledMatrix.shape = readEnum(ledMatrix.value(QLatin1String(DisplayShapeKey)), LedMatrixShapes, profile.ledMatrix.shape);
    profile.ledMatrix = SoftwareProcessing::normalized(profile.ledMatrix);
    for (const auto& [key, field, screen] : PanelSignalFields) {
        auto& panel = profile.*field;
        panel = readDisplaySignal(processing.value(QLatin1String(key)).toObject());
        normalizePanelInput(screen, panel);
    }
    profile.fixedPixel = readFixedPixel(processing.value(QLatin1String(FixedPixelKey)).toObject());
    for (size_t index = 0; index < FixedPixelProfileKeys.size(); ++index) {
        const auto value = processing.value(QLatin1String(FixedPixelProfileKeys[index]));
        if (value.isObject()) profile.fixedPixelProfiles[index] = readFixedPixel(value.toObject(), FixedPixelTechnology(index));
    }
    const auto oled = processing.value(QLatin1String(OledPanelKey)).toObject();
    profile.oled.panelType = readEnum(oled.value(QLatin1String(LcdPanelTypeKey)), OledPanelTypes, profile.oled.panelType);
    profile.oled.pixelStructure = readEnum(oled.value(QLatin1String(PixelStructureKey)),
        OledPixelStructures, profile.oled.pixelStructure);
    const auto ledPanel = processing.value(QLatin1String(LedPanelKey)).toObject();
    const auto oldPanel = [&](FixedPixelTechnology technology) {
        return profile.fixedPixel.technology == technology ? profile.fixedPixel :
            profile.fixedPixelProfiles[size_t(technology)].value_or(FixedPixelProfile{technology});
    };
    readLcd(processing.value(QLatin1String(LcdPanelKey)).toObject(), oldPanel(FixedPixelTechnology::Lcd), profile.lcd);
    readLcd(ledPanel, oldPanel(FixedPixelTechnology::LedBacklitLcd), profile.ledBacklight);
    profile.ledBacklight.type = readEnum(ledPanel.value(QLatin1String(LedBacklightTypeKey)), LedBacklightTypes, profile.ledBacklight.type);
    const auto plasma = processing.value(QLatin1String(PlasmaKey)).toObject();
    const auto cellStructure = plasma.value(QLatin1String(CellStructureKey));
    // Earlier C++ profiles used a numeric intensity instead of a cell structure.
    if (cellStructure.isString())
        profile.plasma.cellStructure = readEnum(cellStructure, PlasmaCellStructures, profile.plasma.cellStructure);
    const auto vector = processing.value(QLatin1String(VectorKey)).toObject();
    for (const auto& [key, field] : VectorFields) profile.vector.*field = vector.value(QLatin1String(key)).toInt(profile.vector.*field);
    profile.vector.phosphorColor = readEnum(vector.value(QLatin1String(CrtPhosphorKey)), VectorPhosphorNames, profile.vector.phosphorColor);
    profile.vector = SoftwareProcessing::normalized(profile.vector);
    const auto vfd = processing.value(QLatin1String(VfdKey)).toObject();
    for (const auto& [key, field] : VfdFields) profile.vfd.*field = vfd.value(QLatin1String(key)).toInt(profile.vfd.*field);
    profile.vfd.color = readEnum(vfd.value(QLatin1String(DisplayColorKey)), VfdColors, profile.vfd.color);
    profile.vfd.structure = readEnum(vfd.value(QLatin1String(VfdStructureKey)), VfdStructures, profile.vfd.structure);
    profile.vfd = SoftwareProcessing::normalized(profile.vfd);
    const auto crt = processing.value(QLatin1String(CrtKey)).toObject();
    profile.crt.mask = readEnum(crt.value(QLatin1String(CrtMaskKey)), MaskNames, profile.crt.mask);
    const auto standards = readEnum(crt.value(QLatin1String(CrtStandardsKey)), CrtStandardNames, profile.crt.colorStandards);
    if (int(standards) != 0 && int(standards) != 6) profile.crt.colorStandards = standards;
    profile.crt.fieldRate = readEnum(crt.value(QLatin1String(CrtFieldRateKey)), CrtRateNames, profile.crt.fieldRate);
    profile.crt.phosphor = readEnum(crt.value(QLatin1String(CrtPhosphorKey)), CrtPhosphorNames, profile.crt.phosphor);
    profile.crt.connection = readEnum(crt.value(QLatin1String(ConnectionKey)), CrtConnections, profile.crt.connection);
    profile.crt.monitorPalette = readEnum(crt.value(QLatin1String(CrtPaletteKey)), CrtPalettes, profile.crt.monitorPalette);
    profile.crt.connectionDegradation = crt.value(QLatin1String(CrtDegradationKey)).toInt();
    profile.crt.beamWidth = crt.value(QLatin1String(CrtBeamKey)).toInt();
    profile.crt.curvature = crt.value(QLatin1String(CrtCurvatureKey)).toInt();
    profile.crt = SoftwareProcessing::normalized(profile.crt);
    return profile;
}

bool ProfileStore::save(const QString& module, const QUuid& id, const Profile& profile) const {
    const auto filename = path(module, id);
    if (filename.isEmpty() || !QDir().mkpath(QFileInfo(filename).absolutePath())) return false;
    auto value = profile.settings;
    value.insert(QLatin1String(RendererKey), enumName(profile.renderer, RendererNames));
    auto processing = value.value(QLatin1String(Processing)).toObject();
    auto restoration = processing.value(QLatin1String(Restoration)).toObject();
    restoration.insert(QLatin1String(DeinterlacingKey), enumName(profile.deinterlacing, DeinterlacingNames));
    const auto restored = SoftwareProcessing::normalized(profile.restoration);
    for (const auto& [key, field] : RestorationFields) restoration.insert(QLatin1String(key), restored.*field);
    processing.insert(QLatin1String(Restoration), restoration);
    processing.insert(QLatin1String(Aspect), enumName(profile.aspectRatio, AspectNames));
    processing.insert(QLatin1String(Sample), enumName(profile.sampling, SamplingNames));
    processing.insert(QLatin1String(BlackFrameInsertion), profile.blackFrameInsertion);
    const auto adjustments = QLatin1String(AdjustmentsKey);
    processing.insert(adjustments, writeImageAdjustments(profile.adjustments, processing.value(adjustments).toObject()));
    auto screens = processing.value(QLatin1String(ScreenAdjustmentsKey)).toObject();
    for (const auto& [screen, name] : ScreenAdjustmentNames) {
        const auto settings = profile.screenAdjustments.constFind(screen);
        if (settings == profile.screenAdjustments.cend()) continue;
        const auto key = QLatin1String(name);
        screens.insert(key, writeImageAdjustments(*settings, screens.value(key).toObject()));
    }
    if (!screens.isEmpty()) processing.insert(QLatin1String(ScreenAdjustmentsKey), screens);
    auto signal = processing.value(QLatin1String(SignalKey)).toObject();
    const auto normalized = SoftwareProcessing::normalized(profile.signal);
    signal.insert(QLatin1String(ConnectionKey), enumName(normalized.connection, ConnectionNames));
    signal.insert(QLatin1String(ConnectionIntensityKey), normalized.connectionIntensity);
    signal.insert(QLatin1String(StandardKey), enumName(normalized.standard, StandardNames));
    signal.insert(QLatin1String(StandardIntensityKey), normalized.standardIntensity);
    processing.insert(QLatin1String(SignalKey), signal);
    auto stylistic = processing.value(QLatin1String(StylisticKey)).toObject();
    const auto effects = SoftwareProcessing::normalized(profile.stylistic);
    for (const auto& [key, field] : StylisticFields) stylistic.insert(QLatin1String(key), effects.*field);
    stylistic.insert(QLatin1String(SepiaKey), effects.sepia);
    processing.insert(QLatin1String(StylisticKey), stylistic);
    const auto previousTechnology = processing.value(QLatin1String(TechnologyKey));
    const auto selected = profile.displayTechnology();
    if (selected != DisplayTechnology::Normal || previousTechnology.isUndefined() ||
        readEnum(previousTechnology, TechnologyNames, UnknownTechnology) != UnknownTechnology)
        processing.insert(QLatin1String(TechnologyKey), enumName(selected, TechnologyNames));
    auto segment = processing.value(QLatin1String(SegmentKey)).toObject();
    const auto segmentSettings = SoftwareProcessing::normalized(profile.segmentDisplay);
    for (const auto& [key, field] : SegmentFields) segment.insert(QLatin1String(key), segmentSettings.*field);
    for (const auto& [key, flag] : SegmentFlags) segment.insert(QLatin1String(key), segmentSettings.*flag);
    segment.insert(QLatin1String(SegmentLayoutKey), enumName(segmentSettings.layout, SegmentLayoutNames));
    segment.insert(QLatin1String(SegmentEndShapeKey), enumName(segmentSettings.endShape, SegmentEndShapeNames));
    segment.insert(QLatin1String(DisplayColorKey), enumName(segmentSettings.color, SegmentColorNames));
    processing.insert(QLatin1String(SegmentKey), segment);
    auto ePaper = processing.value(QLatin1String(EPaperKey)).toObject();
    const auto ePaperSettings = SoftwareProcessing::normalized(profile.ePaper);
    for (const auto& [key, member] : EPaperFields) ePaper.insert(QLatin1String(key), ePaperSettings.*member);
    ePaper.insert(QLatin1String(EPaperColorModeKey), enumName(ePaperSettings.colorMode, EPaperColorNames));
    processing.insert(QLatin1String(EPaperKey), ePaper);
    auto projection = processing.value(QLatin1String(ProjectionKey)).toObject();
    const auto projectionSettings = SoftwareProcessing::normalized(profile.projection);
    projection.insert(QLatin1String(ProjectionAmbientLightKey), projectionSettings.ambientLight);
    projection.insert(QLatin1String(ProjectionSurfaceKey), enumName(projectionSettings.surface, ProjectionSurfaces));
    projection.insert(QLatin1String(ProjectionTechnologyKey), enumName(projectionSettings.technology, ProjectionTechnologies));
    processing.insert(QLatin1String(ProjectionKey), projection);
    auto dot = processing.value(QLatin1String(DotMatrixKey)).toObject();
    const auto dotSettings = SoftwareProcessing::normalized(profile.dotMatrix);
    for (const auto& [key, field] : DotFields) dot.insert(QLatin1String(key), dotSettings.*field);
    dot.insert(QLatin1String(DotPaletteKey), enumName(dotSettings.palette, DotPalettes));
    dot.insert(QLatin1String(DisplayShapeKey), enumName(dotSettings.shape, DotShapes));
    processing.insert(QLatin1String(DotMatrixKey), dot);
    auto ledMatrix = processing.value(QLatin1String(LedMatrixKey)).toObject();
    const auto ledSettings = SoftwareProcessing::normalized(profile.ledMatrix);
    for (const auto& [key, field] : LedMatrixFields) ledMatrix.insert(QLatin1String(key), ledSettings.*field);
    ledMatrix.insert(QLatin1String(DisplayColorKey), enumName(ledSettings.color, LedMatrixColors));
    ledMatrix.insert(QLatin1String(DisplayShapeKey), enumName(ledSettings.shape, LedMatrixShapes));
    processing.insert(QLatin1String(LedMatrixKey), ledMatrix);
    auto vfd = processing.value(QLatin1String(VfdKey)).toObject();
    const auto vfdSettings = SoftwareProcessing::normalized(profile.vfd);
    for (const auto& [key, field] : VfdFields) vfd.insert(QLatin1String(key), vfdSettings.*field);
    vfd.insert(QLatin1String(DisplayColorKey), enumName(vfdSettings.color, VfdColors));
    vfd.insert(QLatin1String(VfdStructureKey), enumName(vfdSettings.structure, VfdStructures));
    processing.insert(QLatin1String(VfdKey), vfd);
    auto vector = processing.value(QLatin1String(VectorKey)).toObject();
    const auto vectorSettings = SoftwareProcessing::normalized(profile.vector);
    for (const auto& [key, field] : VectorFields) vector.insert(QLatin1String(key), vectorSettings.*field);
    vector.insert(QLatin1String(CrtPhosphorKey), enumName(vectorSettings.phosphorColor, VectorPhosphorNames));
    processing.insert(QLatin1String(VectorKey), vector);
    auto plasma = processing.value(QLatin1String(PlasmaKey)).toObject();
    plasma.insert(QLatin1String(CellStructureKey), enumName(profile.plasma.cellStructure, PlasmaCellStructures));
    processing.insert(QLatin1String(PlasmaKey), plasma);
    const auto fixedPixel = QLatin1String(FixedPixelKey);
    processing.insert(fixedPixel, writeFixedPixel(profile.fixedPixel, processing.value(fixedPixel).toObject()));
    for (size_t index = 0; index < FixedPixelProfileKeys.size(); ++index) {
        if (!profile.fixedPixelProfiles[index]) continue;
        const auto key = QLatin1String(FixedPixelProfileKeys[index]);
        processing.insert(key, writeFixedPixel(*profile.fixedPixelProfiles[index], processing.value(key).toObject()));
    }
    for (const auto& [name, field, screen] : PanelSignalFields) {
        const auto key = QLatin1String(name);
        auto panel = processing.value(key).toObject();
        auto panelSignal = profile.*field;
        normalizePanelInput(screen, panelSignal);
        writeDisplaySignal(panel, panelSignal);
        processing.insert(key, panel);
    }
    const auto oledKey = QLatin1String(OledPanelKey);
    auto oled = processing.value(oledKey).toObject();
    oled.insert(QLatin1String(LcdPanelTypeKey), enumName(profile.oled.panelType, OledPanelTypes));
    oled.insert(QLatin1String(PixelStructureKey), enumName(profile.oled.pixelStructure, OledPixelStructures));
    processing.insert(oledKey, oled);
    const auto ledKey = QLatin1String(LedPanelKey);
    auto ledPanel = processing.value(ledKey).toObject();
    const auto lcdKey = QLatin1String(LcdPanelKey);
    auto lcdPanel = processing.value(lcdKey).toObject();
    writeLcd(lcdPanel, profile.lcd);
    processing.insert(lcdKey, lcdPanel);
    writeLcd(ledPanel, profile.ledBacklight);
    ledPanel.insert(QLatin1String(LedBacklightTypeKey), enumName(profile.ledBacklight.type, LedBacklightTypes));
    processing.insert(ledKey, ledPanel);
    auto crt = processing.value(QLatin1String(CrtKey)).toObject();
    const auto crtSettings = SoftwareProcessing::normalized(profile.crt);
    crt.insert(QLatin1String(CrtMaskKey), enumName(crtSettings.mask, MaskNames));
    crt.insert(QLatin1String(CrtStandardsKey), enumName(crtSettings.colorStandards, CrtStandardNames));
    crt.insert(QLatin1String(CrtFieldRateKey), enumName(crtSettings.fieldRate, CrtRateNames));
    crt.insert(QLatin1String(CrtPhosphorKey), enumName(crtSettings.phosphor, CrtPhosphorNames));
    crt.insert(QLatin1String(ConnectionKey), enumName(crtSettings.connection, CrtConnections));
    crt.insert(QLatin1String(CrtPaletteKey), enumName(crtSettings.monitorPalette, CrtPalettes));
    crt.insert(QLatin1String(CrtDegradationKey), crtSettings.connectionDegradation);
    crt.insert(QLatin1String(CrtBeamKey), crtSettings.beamWidth);
    crt.insert(QLatin1String(CrtCurvatureKey), crtSettings.curvature);
    processing.insert(QLatin1String(CrtKey), crt);
    value.insert(QLatin1String(Processing), processing);
    const auto bytes = QJsonDocument(value).toJson();
    QSaveFile file(filename);
    file.setDirectWriteFallback(false);
    return bytes.size() <= MaximumProfileBytes && file.open(QIODevice::WriteOnly) &&
        file.write(bytes) == bytes.size() && file.commit();
}

bool ProfileStore::copy(const QString& module, const QUuid& source, const QUuid& destination) const {
    if (path(module, source).isEmpty() || path(module, destination).isEmpty()) return false;
    const auto filename = existingPath(module, source);
    if (filename.isEmpty()) return save(module, destination, {});
    const auto settings = readProfile(filename);
    return settings && save(module, destination, decode(*settings));
}

bool ProfileStore::remove(const QString& module, const QUuid& id) const {
    const auto canonical = path(module, id);
    if (canonical.isEmpty()) return false;
    const auto previous = legacyPath(module, id);
    return (!profileExists(previous) || QFile::remove(previous)) &&
        (!profileExists(canonical) || QFile::remove(canonical));
}

class VideoSurface::TemporalState : public SoftwareProcessing::DisplayHistory {};
class VideoSurface::RefreshState {
public:
    BlackFrameCadence cadence;
    ImageRestoration hardwareRestoration;
    SoftwareProcessing::CrtShaderParameters hardwareCrt;
    QImage crtSource;
    std::unique_ptr<VulkanSurface> vulkanWindow;
    std::optional<quint64> presentedImage;
    quint64 image = 0;
    quint64 generation = 0;
    double machineHz = 0;
    bool running = false;
    bool active = false;
    bool shaderLoading = false;
    std::function<void(bool)> shaderLoadingObserver;
    std::atomic_bool pending = false;
    std::jthread worker;
};

VideoSurface::~VideoSurface() {
    processing_.reset();
    updateRefreshClock(false);
    vulkanSurface_ = nullptr;
    refresh_->vulkanWindow.reset();
}

VideoSurface::VideoSurface(QWidget* parent) : QWidget(parent), refresh_(std::make_unique<RefreshState>()), temporal_(std::make_unique<TemporalState>()) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_OpaquePaintEvent);
}
void VideoSurface::setProfile(Profile profile) {
    for (const auto& entry : PanelSignalFields) {
        auto& panel = profile.*std::get<1>(entry);
        normalizePanelInput(std::get<2>(entry), panel);
    }
    if (profile_.displayDeinterlacing() != profile.displayDeinterlacing()) motionSequence_.reset();
    const bool resetHistory = profile_.fixedPixel != profile.fixedPixel || profile_.fixedPixelEnabled != profile.fixedPixelEnabled ||
        profile_.effectiveImageAdjustments() != profile.effectiveImageAdjustments() || profile_.signal != profile.signal || profile_.sampling != profile.sampling ||
        profile_.crt.enabled != profile.crt.enabled || profile_.stylistic != profile.stylistic ||
        profile_.restoration != profile.restoration || profile_.plasma != profile.plasma ||
        profile_.lcdSignal != profile.lcdSignal || profile_.ledSignal != profile.ledSignal ||
        profile_.oledSignal != profile.oledSignal || profile_.plasmaSignal != profile.plasmaSignal ||
        profile_.projectionSignal != profile.projectionSignal || profile_.ledBacklight != profile.ledBacklight ||
        profile_.lcd != profile.lcd || profile_.oled != profile.oled ||
        profile_.vector != profile.vector || profile_.vfd != profile.vfd || profile_.dotMatrix != profile.dotMatrix ||
        profile_.segmentDisplay != profile.segmentDisplay || profile_.ePaper != profile.ePaper || profile_.projection != profile.projection ||
        profile_.deinterlacing != profile.deinterlacing;
    if (resetHistory) *temporal_ = {};
    if (processing_) processing_->invalidate(resetHistory);
    const bool clockChanged = profile_.blackFrameInsertion != profile.blackFrameInsertion;
    if (profile_.renderer != profile.renderer) setShaderLoading(false);
    profile_ = std::move(profile);
    if (clockChanged) updateRefreshClock(isVisible());
    if (profile_.renderer == Renderer::OpenGL) {
        if (!glSurface_) {
            glSurface_ = new OpenGlSurface([this](QPainter& painter) { draw(painter, true); },
                [this] { setShaderLoading(true); },
                [this] { samplingFinished(Renderer::OpenGL); }, this);
            glSurface_->setGeometry(rect());
            glSurface_->show();
        }
    } else if (glSurface_) {
        delete glSurface_;
        glSurface_ = nullptr;
    }
#ifdef Q_OS_WIN
    if (profile_.renderer == Renderer::Direct3D11) {
        if (!d3dSurface_) {
            d3dSurface_ = new Direct3DSurface([this] { setShaderLoading(true); },
                [this] { samplingFinished(Renderer::Direct3D11); }, this);
            d3dSurface_->setGeometry(rect());
            d3dSurface_->show();
        }
    } else if (d3dSurface_) {
        delete d3dSurface_;
        d3dSurface_ = nullptr;
        overlay_ = {};
    }
#endif
    if (profile_.renderer == Renderer::Vulkan) {
        auto& window = refresh_->vulkanWindow;
        const bool createWindow = !window;
        if (createWindow) {
            // Finish creating the native host before adding the RHI surface to the widget tree.
            setAttribute(Qt::WA_DontCreateNativeAncestors);
            setAttribute(Qt::WA_NativeWindow);
            winId();
            window = std::make_unique<VulkanSurface>([this](QPainter& painter) { draw(painter); },
                [this] {
                    if (profile_.renderer == Renderer::Vulkan) setShaderLoading(true);
                },
                [this] { samplingFinished(Renderer::Vulkan); });
        }
        vulkanSurface_ = window.get();
        if (createWindow) {
            vulkanSurface_->attachTo(*this);
        } else {
            vulkanSurface_->setGeometry(rect());
            if (isVisible()) vulkanSurface_->showEmbedded();
        }
    } else if (vulkanSurface_) {
        vulkanSurface_->hide();
        vulkanSurface_ = nullptr;
    }
    compose(resetHistory);
    scheduleUpdate();
}
void VideoSurface::setEmulationRunning(bool running) {
    if (refresh_->running == running) return;
    refresh_->running = running;
    if (!running) refresh_->machineHz = 0;
    updateRefreshClock(isVisible());
    scheduleUpdate();
}

void VideoSurface::updateRefreshClock(bool enable) {
    auto& state = *refresh_;
#ifdef Q_OS_WIN
    if (enable && state.running && profile_.blackFrameInsertion) {
        if (state.worker.joinable()) return;
        const auto handle = static_cast<std::uintptr_t>(window()->winId());
        const auto generation = state.generation;
        state.worker = std::jthread([this, handle, generation](std::stop_token token) {
            runNativeRefreshClock(handle, token, [this, generation](quint64 tick, double hostHz) {
                if (refresh_->pending.exchange(true)) return;
                QMetaObject::invokeMethod(this, [this, generation, tick, hostHz] {
                    if (generation != refresh_->generation) return;
                    refresh_->pending = false;
                    presentAtRefresh(tick, hostHz);
                }, Qt::QueuedConnection);
            });
        });
        return;
    }
    state.worker = {};
#else
    Q_UNUSED(enable);
#endif
    ++state.generation;
    state.pending = false;
    state.active = false;
    state.cadence = {};
    state.presentedImage.reset();
    blackFrame_ = false;
}

void VideoSurface::presentAtRefresh(quint64 tick, double hostHz) {
    auto& state = *refresh_;
    const bool available = state.running && isVisible() && !image_.isNull() &&
        BlackFrameCadence::available(hostHz, state.machineHz);
    if (!available) {
        if (!state.active) return;
        state.active = false;
        state.cadence = {};
        state.presentedImage.reset();
        blackFrame_ = false;
        scheduleUpdate();
        return;
    }
    state.active = true;
    const bool newImage = state.presentedImage != state.image;
    const bool wasBlack = blackFrame_;
    blackFrame_ = state.cadence.shouldBlank(tick, newImage);
    state.presentedImage = state.image;
    if (newImage || wasBlack != blackFrame_) scheduleUpdate();
}

void VideoSurface::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    if (vulkanSurface_) vulkanSurface_->showEmbedded();
    updateRefreshClock(true);
    if (processing_) compose(true);
    scheduleUpdate();
}

void VideoSurface::hideEvent(QHideEvent* event) {
    setShaderLoading(false);
    if (vulkanSurface_) vulkanSurface_->hide();
    if (processing_) processing_->invalidate(true);
    updateRefreshClock(false);
    QWidget::hideEvent(event);
}

void VideoSurface::submit(std::shared_ptr<const Emulation::VideoFrame> frame) {
    if (frame == frame_) return;
    if (!frame) {
        clear();
        return;
    }
    const auto rate = frame->signalFieldRate;
    const auto maximumGap = rate > 0 && std::isfinite(rate) ? std::clamp(1.75 / rate, .001, .15) : .1;
    const bool consecutive = frame_ && frame->scanMode != Emulation::ScanMode::Progressive &&
        frame_->scanMode != Emulation::ScanMode::Progressive && frame->scanMode != frame_->scanMode &&
        frame->sequence - frame_->sequence == 1 && frame->timestamp > frame_->timestamp &&
        std::chrono::duration<double>(frame->timestamp - frame_->timestamp).count() <= maximumGap &&
        frame->width == frame_->width && frame->height == frame_->height && frame->pitch == frame_->pitch &&
        frame->aspectRatio == frame_->aspectRatio && frame->pixelFormat == frame_->pixelFormat && !source_.isNull();
    if (!consecutive) {
        previousField_ = sameParity_ = previousComplement_ = {};
        motionSequence_.reset();
    } else {
        previousComplement_ = std::move(sameParity_);
        sameParity_ = std::move(previousField_);
        previousField_ = std::move(source_);
    }
    const auto previousSequence = frame_ ? frame_->sequence : 0;
    const auto previousTimestamp = frame_ ? frame_->timestamp : std::chrono::nanoseconds{};
    const auto duration = frame->sequence > previousSequence && frame->timestamp > previousTimestamp
        ? frame->timestamp - previousTimestamp : std::chrono::nanoseconds{};
    detectedSignal_ = duration > PalDurationThreshold ? SignalStandard::Pal : SignalStandard::Ntsc;
    if (frame_ && duration.count() > 0) {
        refresh_->machineHz = double(frame->sequence - previousSequence) / std::chrono::duration<double>(duration).count();
    } else if (rate > 0 && std::isfinite(rate)) {
        refresh_->machineHz = rate;
    } else {
        refresh_->machineHz = 0;
    }
    frame_ = std::move(frame);
    source_ = decode(*frame_);
    compose(true);
}
void VideoSurface::compose(bool advance) {
    if (!frame_) return;
    auto signal = profile_.signal;
    if (signal.standard == SignalStandard::Automatic) signal.standard = detectedSignal_;
    const auto composed = SoftwareProcessing::composeFields(source_, previousField_, sameParity_,
        previousComplement_, *frame_, profile_.displayDeinterlacing(), motion_, motionSequence_);
    const auto imageAspect = SoftwareProcessing::frameAspectRatio(*frame_, composed.size());
    const auto outputSize = imageViewport(composed.size(), imageAspect).size() * devicePixelRatioF();
    bool gpuCanSample = (glSurface_ && glSurface_->canSample()) ||
        (vulkanSurface_ && vulkanSurface_->canSample());
#ifdef Q_OS_WIN
    gpuCanSample |= d3dSurface_ && d3dSurface_->canSample();
#endif
    const auto adjustments = profile_.effectiveImageAdjustments();
    const auto sampling = profile_.displaySampling();
    const bool needsSampling = usesSoftwareSampling(sampling) && outputSize != composed.size();
    auto restoration = SoftwareProcessing::restorationForDisplay(profile_);
    restoration.dedithering = 0;
    if (sampling == Sampling::Nearest || outputSize == composed.size()) restoration.detailRecovery = 0;
    bool gpuCanRenderCrt = (glSurface_ && glSurface_->canSampleCrt()) ||
        (vulkanSurface_ && vulkanSurface_->canSampleCrt());
#ifdef Q_OS_WIN
    gpuCanRenderCrt |= d3dSurface_ && d3dSurface_->canSampleCrt();
#endif
    if (!composed.isNull() && !outputSize.isEmpty() && gpuCanRenderCrt &&
        profile_.displayTechnology() == DisplayTechnology::Crt &&
        SoftwareProcessing::normalized(profile_.stylistic) == StylisticEffects{}) {
        if (processing_) processing_->invalidate(false);
        presentImage(composed, source_.size(), imageAspect, Sampling::Nearest, adjustments, {}, true);
        return;
    }
    if (!composed.isNull() && !outputSize.isEmpty() && gpuCanSample &&
        (needsSampling || adjustments != ImageAdjustments{} || restoration != ImageRestoration{})) {
        auto remainingEffects = profile_;
        remainingEffects.sampling = sampling == Sampling::Nearest ? Sampling::Nearest : Sampling::Linear;
        remainingEffects.setImageAdjustments(remainingEffects.screenType(), {});
        remainingEffects.restoration.debanding = 0;
        remainingEffects.restoration.detailRecovery = 0;
        if (!SoftwareProcessing::requiresProcessing(remainingEffects, signal, composed.size(), outputSize)) {
            presentImage(composed, source_.size(), imageAspect, sampling, adjustments, restoration);
            return;
        }
    }
    const auto requiresProcessing = !composed.isNull() &&
        SoftwareProcessing::requiresProcessing(profile_, signal, composed.size(), outputSize);
    if (!requiresProcessing) {
        if (composed.isNull() && processing_) processing_->invalidate(true);
        presentImage(composed, source_.size(), imageAspect);
        return;
    }
    if (!processing_ && refresh_->running) {
        processing_ = std::make_unique<SoftwareProcessing::SoftwareFrameWorker>(
            [this](QImage image, QSize size, double aspect) { presentImage(std::move(image), size, aspect); });
    }
    if (processing_) {
        if (isVisible()) processing_->submit({composed, source_.size(), outputSize, profile_, signal, frame_, advance});
        return;
    }
    presentImage(SoftwareProcessing::process(composed, profile_, signal, *frame_, outputSize, *temporal_, advance),
        source_.size(), imageAspect);
}

void VideoSurface::presentImage(QImage image, QSize sourceSize, double aspectRatio,
    std::optional<Sampling> hardwareSampling, ImageAdjustments hardwareAdjustments, ImageRestoration hardwareRestoration, bool hardwareCrt) {
    hardwareSampling_ = hardwareSampling;
    hardwareAdjustments_ = hardwareAdjustments;
    refresh_->hardwareRestoration = hardwareRestoration;
    refresh_->hardwareCrt = {};
    refresh_->crtSource = {};
    if (hardwareCrt && frame_) {
        const auto crt = SoftwareProcessing::normalized(profile_.crt);
        const auto displayAspect = float(SoftwareProcessing::screenAspectRatio(profile_, aspectRatio));
        refresh_->hardwareCrt = SoftwareProcessing::crtShaderParameters(crt, *frame_, displayAspect, hardwareAdjustments.brightness);
        const auto restoration = SoftwareProcessing::restorationForDisplay(profile_);
        const auto prepared = SoftwareProcessing::dedither(image, restoration.dedithering);
        refresh_->crtSource = SoftwareProcessing::filterCrtConnection(prepared, crt, *frame_);
    }
    image_ = std::move(image);
    imageSourceSize_ = sourceSize;
    imageAspectRatio_ = aspectRatio;
    ++refresh_->image;
    if (!refresh_->active) scheduleUpdate();
}

void VideoSurface::clear() {
    setShaderLoading(false);
    if (processing_) processing_->invalidate(true);
    frame_.reset();
    refresh_->machineHz = 0;
    refresh_->active = false;
    refresh_->cadence = {};
    refresh_->presentedImage.reset();
    blackFrame_ = false;
    *temporal_ = {};
    source_ = previousField_ = sameParity_ = previousComplement_ = image_ = {};
    hardwareSampling_.reset();
    hardwareAdjustments_ = {};
    refresh_->hardwareRestoration = {};
    refresh_->hardwareCrt = {};
    refresh_->crtSource = {};
    motion_.clear();
    motionSequence_.reset();
    scheduleUpdate();
}
void VideoSurface::setStatistics(QString text) {
    if (statistics_ == text) return;
    statistics_ = std::move(text);
    overlay_ = {};
    scheduleUpdate();
}
void VideoSurface::setIndicator(QString text) {
    if (indicator_ == text) return;
    indicator_ = std::move(text);
    overlay_ = {};
    scheduleUpdate();
}
bool VideoSurface::isShaderLoading() const noexcept {
    return refresh_->shaderLoading;
}
void VideoSurface::setShaderLoadingObserver(std::function<void(bool)> observer) {
    refresh_->shaderLoadingObserver = std::move(observer);
    if (refresh_->shaderLoadingObserver) refresh_->shaderLoadingObserver(isShaderLoading());
}
void VideoSurface::setShaderLoading(bool loading) {
    if (loading == refresh_->shaderLoading) return;
    refresh_->shaderLoading = loading;
    if (refresh_->shaderLoadingObserver) refresh_->shaderLoadingObserver(loading);
}
void VideoSurface::samplingFinished(Renderer renderer) {
    if (profile_.renderer != renderer) return;
    setShaderLoading(false);
    if (processing_) processing_->invalidate(false);
    compose();
}
QImage VideoSurface::currentImage() const {
    if (!hardwareSampling_ || image_.isNull()) return image_;
    const QSize outputSize = imageViewport().size() * devicePixelRatioF();
    const auto& gpuSource = refresh_->hardwareCrt.enabled() ? refresh_->crtSource : image_;
    if (glSurface_) {
        const auto capture = glSurface_->captureSampled(gpuSource, imageViewport(), *hardwareSampling_, hardwareAdjustments_,
            refresh_->hardwareRestoration, refresh_->hardwareCrt);
        if (!capture.isNull()) return capture;
    }
    if (vulkanSurface_) {
        const auto capture = vulkanSurface_->captureSampled(gpuSource, imageViewport(), *hardwareSampling_, hardwareAdjustments_,
            refresh_->hardwareRestoration, refresh_->hardwareCrt);
        if (!capture.isNull()) return capture;
    }
#ifdef Q_OS_WIN
    if (d3dSurface_) {
        const auto capture = d3dSurface_->captureSampled(gpuSource, imageViewport(), *hardwareSampling_, hardwareAdjustments_,
            refresh_->hardwareRestoration, refresh_->hardwareCrt);
        if (!capture.isNull()) return capture;
    }
#endif
    if (refresh_->hardwareCrt.enabled() && frame_) {
        SoftwareProcessing::DisplayHistory history;
        return SoftwareProcessing::process(image_, profile_, {}, *frame_, outputSize, history, false);
    }
    Profile samplingOnly;
    samplingOnly.sampling = *hardwareSampling_;
    samplingOnly.adjustments = hardwareAdjustments_;
    samplingOnly.restoration = refresh_->hardwareRestoration;
    SoftwareProcessing::DisplayHistory history;
    return SoftwareProcessing::process(image_, samplingOnly, {}, Emulation::VideoFrame{}, outputSize, history, false);
}

bool VideoSurface::usesOpenGl() const { return glSurface_ && glSurface_->isVisible() && glSurface_->isValid(); }
bool VideoSurface::usesVulkan() const { return vulkanSurface_ && vulkanSurface_->isPresenting(); }

bool VideoSurface::usesDirect3D11() const {
#ifdef Q_OS_WIN
    return d3dSurface_ && d3dSurface_->isPresenting();
#else
    return false;
#endif
}

void VideoSurface::scheduleUpdate() {
    bool needsOverlay = vulkanSurface_ && hardwareSampling_;
#ifdef Q_OS_WIN
    needsOverlay |= d3dSurface_ != nullptr;
#endif
    if (needsOverlay && overlay_.isNull() && !size().isEmpty() && (!statistics_.isEmpty() || !indicator_.isEmpty())) {
        const auto scale = devicePixelRatioF();
        overlay_ = QImage(size() * scale, QImage::Format_ARGB32);
        if (!overlay_.isNull()) {
            overlay_.setDevicePixelRatio(scale);
            overlay_.fill(Qt::transparent);
            QPainter painter(&overlay_);
            drawOverlay(painter);
        }
    }
    const auto& gpuSource = refresh_->hardwareCrt.enabled() ? refresh_->crtSource : image_;
#ifdef Q_OS_WIN
    if (d3dSurface_)
        d3dSurface_->setScene(blackFrame_ ? QImage{} : gpuSource, imageViewport(),
            hardwareSampling_.value_or(profile_.displaySampling() == Sampling::Linear ? Sampling::Linear : Sampling::Nearest), overlay_, hardwareAdjustments_, refresh_->hardwareRestoration, refresh_->hardwareCrt);
#endif
    if (usesOpenGl()) glSurface_->update();
    if (vulkanSurface_) {
        if (hardwareSampling_)
            vulkanSurface_->setSampledScene(blackFrame_ ? QImage{} : gpuSource, imageViewport(), *hardwareSampling_,
                overlay_, hardwareAdjustments_, refresh_->hardwareRestoration, refresh_->hardwareCrt);
        else
            vulkanSurface_->setSampledScene({}, {}, Sampling::Nearest, {});
    }
    update();
}

void VideoSurface::refreshViewport() {
    overlay_ = {};
    if (hardwareSampling_ || usesSoftwareSampling(profile_.displaySampling()) || profile_.displayTechnology() != DisplayTechnology::Normal ||
        profile_.stylistic != StylisticEffects{} ||
        (profile_.restoration.detailRecovery > 0 && profile_.displaySampling() != Sampling::Nearest)) {
        if (processing_) processing_->invalidate(false);
        compose();
    }
    scheduleUpdate();
}

bool VideoSurface::event(QEvent* event) {
    const bool handled = QWidget::event(event);
    if (event->type() == QEvent::DevicePixelRatioChange) refreshViewport();
    return handled;
}

void VideoSurface::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (glSurface_) glSurface_->setGeometry(rect());
    if (vulkanSurface_) vulkanSurface_->setGeometry(rect());
#ifdef Q_OS_WIN
    if (d3dSurface_) d3dSurface_->setGeometry(rect());
#endif
    refreshViewport();
}

void VideoSurface::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    if (usesOpenGl() || usesDirect3D11() || usesVulkan()) painter.fillRect(rect(), Qt::black);
    else draw(painter);
}

QRect VideoSurface::imageViewport() const {
    return image_.isNull() ? QRect{} : imageViewport(imageSourceSize_, imageAspectRatio_);
}

QRect VideoSurface::imageViewport(const QSize& source, double aspectRatio) const {
    if (source.isEmpty()) return {};
    const double imageAspect = aspectRatio > 0 && std::isfinite(aspectRatio)
        ? aspectRatio : double(source.width()) / source.height();
    const auto screen = fit(size(), SoftwareProcessing::screenAspectRatio(profile_, imageAspect));
    return fit(screen.size(), imageAspect).translated(screen.topLeft());
}

void VideoSurface::draw(QPainter& painter, bool openGl) {
    painter.fillRect(rect(), Qt::black);
    if (!blackFrame_ && !image_.isNull()) {
        const auto bounds = imageViewport();
        const bool crt = refresh_->hardwareCrt.enabled();
        const auto& gpuSource = crt ? refresh_->crtSource : image_;
        const bool sampled = openGl && hardwareSampling_ && glSurface_->drawSampled(painter, gpuSource, bounds,
            *hardwareSampling_, hardwareAdjustments_, refresh_->hardwareRestoration, refresh_->hardwareCrt);
        if (!sampled) {
            painter.setRenderHint(QPainter::SmoothPixmapTransform, profile_.displaySampling() == Sampling::Linear);
            if (crt && frame_) {
                SoftwareProcessing::DisplayHistory history;
                painter.drawImage(bounds, SoftwareProcessing::process(image_, profile_, {}, *frame_,
                    bounds.size() * devicePixelRatioF(), history, false));
            } else painter.drawImage(bounds, image_);
        }
    }
    drawOverlay(painter);
}

void VideoSurface::drawOverlay(QPainter& painter) {
    painter.setRenderHint(QPainter::Antialiasing);
    paintOsd(painter, rect(), statistics_, Qt::AlignTop | Qt::AlignRight);
    paintOsd(painter, rect(), indicator_, Qt::AlignCenter);
}
}
