#include "SoftwareProcessing.h"
#include "SoftwareFrameWorker.h"
#include "SamplingFilters.h"
#include "BlackFrameCadence.h"
#include "CrtProcessing.h"
#include "VfdProcessing.h"
#include "DotMatrixProcessing.h"
#include "SegmentDisplayProcessing.h"
#include "PaperProjectionProcessing.h"
#include "CrtConnection.h"
#include "FixedPixelProcessing.h"
#include "RestorationProcessing.h"
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
#include <QTimer>
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
constexpr std::array EPaperColorNames{"Monochrome", "Grayscale16", "Color4096", "TricolorRed", "TricolorYellow", "Gallery"};
constexpr auto EPaperKey = "ePaper";
constexpr auto ProjectionKey = "projection";
constexpr auto DisplayTechnologyKey = "technology";
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
constexpr std::array<const char*, 8> SegmentColorNames{"Red", "Green", "Amber", "Blue", "White", "Black", "Gray", "BlueGreen"};
constexpr std::array<const char*, 3> SegmentTechnologies{"Led", "Lcd", "Vfd"};
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
constexpr std::array<const char*, 7> DotPalettes{"Black", "Gray", "Green", "Amber", "Red", "Blue", "Rgb"};
constexpr std::array<const char*, 3> DotTechnologies{"Reflective", "Backlit", "Emissive"};
constexpr std::array<const char*, 3> DotShapes{"Round", "Square", "Rectangle"};
constexpr auto UnknownTechnology = static_cast<DisplayTechnology>(-1);
constexpr auto LedMatrixKey = "ledMatrix";
constexpr std::array<const char*, 7> LedMatrixColors{"Rgb", "Red", "Green", "Amber", "Blue", "White", "RedGreen"};
constexpr std::array<const char*, 3> LedMatrixShapes{"Round", "Square", "RoundedSquare"};
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
constexpr std::array<const char*, 6> VfdColors{"BlueGreen", "Green", "Blue", "White", "Amber", "Red"};
constexpr std::array<const char*, 4> VectorPhosphorNames{"Color", "Green", "Amber", "White"};
constexpr auto VectorBeamWidthKey = "beamWidth";
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
constexpr std::array<const char*, 10> MatrixDefinitionNames{"Native", "Size16x16", "Size32x16", "Size32x32", "Size64x32", "Size64x64", "Size128x64", "Size128x128", "Size256x128", "Size256x256"};
constexpr std::array MatrixDefinitionSizes{QSize{}, QSize{16, 16}, QSize{32, 16}, QSize{32, 32}, QSize{64, 32}, QSize{64, 64}, QSize{128, 64}, QSize{128, 128}, QSize{256, 128}, QSize{256, 256}};
static_assert(MatrixDefinitionSizes.size() == MatrixDefinitionNames.size());
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

constexpr int FullTransparency = 100;

void paintOsd(QPainter& painter, const QRect& area, const QString& text, Qt::Alignment alignment, int transparency) {
    if (text.isEmpty()) return;
    const auto bounds = painter.fontMetrics().boundingRect(area.adjusted(OsdMargin, OsdMargin, -OsdMargin, -OsdMargin),
        alignment | Qt::TextWordWrap, text);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, OsdAlpha * (FullTransparency - transparency) / FullTransparency));
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

QSize matrixSize(MatrixDefinition definition, QSize source) {
    const auto index = size_t(definition);
    return index > 0 && index < MatrixDefinitionSizes.size() ? MatrixDefinitionSizes[index] : source;
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
    const auto technology = processing.value(QLatin1String(TechnologyKey));
    profile.setDisplayTechnology(readEnum(technology, TechnologyNames, DisplayTechnology::Normal));
    const auto segment = processing.value(QLatin1String(SegmentKey)).toObject();
    for (const auto& [key, field] : SegmentFields)
        profile.segmentDisplay.*field = segment.value(QLatin1String(key)).toInt(profile.segmentDisplay.*field);
    for (const auto& [key, flag] : SegmentFlags)
        profile.segmentDisplay.*flag = segment.value(QLatin1String(key)).toBool(profile.segmentDisplay.*flag);
    profile.segmentDisplay.layout = readEnum(segment.value(QLatin1String(SegmentLayoutKey)), SegmentLayoutNames, profile.segmentDisplay.layout);
    profile.segmentDisplay.endShape = readEnum(segment.value(QLatin1String(SegmentEndShapeKey)), SegmentEndShapeNames, profile.segmentDisplay.endShape);
    profile.segmentDisplay.technology = readEnum(segment.value(QLatin1String(DisplayTechnologyKey)), SegmentTechnologies, profile.segmentDisplay.technology);
    profile.segmentDisplay.color = readEnum(segment.value(QLatin1String(DisplayColorKey)), SegmentColorNames, profile.segmentDisplay.color);
    profile.segmentDisplay = SoftwareProcessing::normalized(profile.segmentDisplay);
    const auto ePaper = processing.value(QLatin1String(EPaperKey)).toObject();
    profile.ePaper.colorMode = readEnum(ePaper.value(QLatin1String(EPaperColorModeKey)), EPaperColorNames, profile.ePaper.colorMode);
    profile.ePaper.definition = readEnum(ePaper.value(QLatin1String(PanelDefinitionKey)), PanelDefinitionNames, profile.ePaper.definition);
    const auto projection = processing.value(QLatin1String(ProjectionKey)).toObject();
    profile.projection.ambientLight = projection.value(QLatin1String(ProjectionAmbientLightKey)).toInt();
    profile.projection.surface = readEnum(projection.value(QLatin1String(ProjectionSurfaceKey)),
        ProjectionSurfaces, profile.projection.surface);
    profile.projection.technology = readEnum(projection.value(QLatin1String(DisplayTechnologyKey)),
        ProjectionTechnologies, profile.projection.technology);
    profile.projection = SoftwareProcessing::normalized(profile.projection);
    const auto dot = processing.value(QLatin1String(DotMatrixKey)).toObject();
    const auto dotPalette = dot.value(QLatin1String(DotPaletteKey));
    constexpr auto LegacyWhite = "White";
    profile.dotMatrix.palette = dotPalette.toString() == QLatin1String(LegacyWhite) ? DotMatrixPalette::Gray
        : readEnum(dotPalette, DotPalettes, profile.dotMatrix.palette);
    profile.dotMatrix.technology = readEnum(dot.value(QLatin1String(DisplayTechnologyKey)), DotTechnologies, profile.dotMatrix.technology);
    profile.dotMatrix.shape = readEnum(dot.value(QLatin1String(DisplayShapeKey)), DotShapes, profile.dotMatrix.shape);
    profile.dotMatrix.definition = readEnum(dot.value(QLatin1String(PanelDefinitionKey)), MatrixDefinitionNames, profile.dotMatrix.definition);
    const auto ledMatrix = processing.value(QLatin1String(LedMatrixKey)).toObject();
    profile.ledMatrix.color = readEnum(ledMatrix.value(QLatin1String(DisplayColorKey)), LedMatrixColors, profile.ledMatrix.color);
    profile.ledMatrix.shape = readEnum(ledMatrix.value(QLatin1String(DisplayShapeKey)), LedMatrixShapes, profile.ledMatrix.shape);
    profile.ledMatrix.definition = readEnum(ledMatrix.value(QLatin1String(PanelDefinitionKey)), MatrixDefinitionNames, profile.ledMatrix.definition);
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
    profile.vector.beamWidth = vector.value(QLatin1String(VectorBeamWidthKey)).toInt(profile.vector.beamWidth);
    profile.vector.curvature = vector.value(QLatin1String(CrtCurvatureKey)).toInt(profile.vector.curvature);
    const auto phosphor = vector.value(QLatin1String(CrtPhosphorKey));
    profile.vector.phosphorColor = phosphor.toString() == QLatin1String("Gray") ? VectorPhosphor::White :
        readEnum(phosphor, VectorPhosphorNames, profile.vector.phosphorColor);
    profile.vector = SoftwareProcessing::normalized(profile.vector);
    const auto vfd = processing.value(QLatin1String(VfdKey)).toObject();
    profile.vfd.color = readEnum(vfd.value(QLatin1String(DisplayColorKey)), VfdColors, profile.vfd.color);
    profile.vfd.shape = readEnum(vfd.value(QLatin1String(DisplayShapeKey)), DotShapes, profile.vfd.shape);
    profile.vfd.definition = readEnum(vfd.value(QLatin1String(PanelDefinitionKey)), MatrixDefinitionNames, profile.vfd.definition);
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
    processing.remove(QLatin1String(StylisticKey));
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
    segment.insert(QLatin1String(DisplayTechnologyKey), enumName(segmentSettings.technology, SegmentTechnologies));
    segment.insert(QLatin1String(DisplayColorKey), enumName(segmentSettings.color, SegmentColorNames));
    processing.insert(QLatin1String(SegmentKey), segment);
    auto ePaper = processing.value(QLatin1String(EPaperKey)).toObject();
    ePaper.insert(QLatin1String(EPaperColorModeKey), enumName(profile.ePaper.colorMode, EPaperColorNames));
    ePaper.insert(QLatin1String(PanelDefinitionKey), enumName(profile.ePaper.definition, PanelDefinitionNames));
    processing.insert(QLatin1String(EPaperKey), ePaper);
    auto projection = processing.value(QLatin1String(ProjectionKey)).toObject();
    const auto projectionSettings = SoftwareProcessing::normalized(profile.projection);
    projection.insert(QLatin1String(ProjectionAmbientLightKey), projectionSettings.ambientLight);
    projection.insert(QLatin1String(ProjectionSurfaceKey), enumName(projectionSettings.surface, ProjectionSurfaces));
    projection.insert(QLatin1String(DisplayTechnologyKey), enumName(projectionSettings.technology, ProjectionTechnologies));
    processing.insert(QLatin1String(ProjectionKey), projection);
    auto dot = processing.value(QLatin1String(DotMatrixKey)).toObject();
    dot.insert(QLatin1String(DotPaletteKey), enumName(profile.dotMatrix.palette, DotPalettes));
    dot.insert(QLatin1String(DisplayShapeKey), enumName(profile.dotMatrix.shape, DotShapes));
    dot.insert(QLatin1String(PanelDefinitionKey), enumName(profile.dotMatrix.definition, MatrixDefinitionNames));
    dot.insert(QLatin1String(DisplayTechnologyKey), enumName(profile.dotMatrix.technology, DotTechnologies));
    processing.insert(QLatin1String(DotMatrixKey), dot);
    auto ledMatrix = processing.value(QLatin1String(LedMatrixKey)).toObject();
    ledMatrix.insert(QLatin1String(DisplayColorKey), enumName(profile.ledMatrix.color, LedMatrixColors));
    ledMatrix.insert(QLatin1String(DisplayShapeKey), enumName(profile.ledMatrix.shape, LedMatrixShapes));
    ledMatrix.insert(QLatin1String(PanelDefinitionKey), enumName(profile.ledMatrix.definition, MatrixDefinitionNames));
    processing.insert(QLatin1String(LedMatrixKey), ledMatrix);
    auto vfd = processing.value(QLatin1String(VfdKey)).toObject();
    vfd.insert(QLatin1String(DisplayColorKey), enumName(profile.vfd.color, VfdColors));
    vfd.insert(QLatin1String(DisplayShapeKey), enumName(profile.vfd.shape, DotShapes));
    vfd.insert(QLatin1String(PanelDefinitionKey), enumName(profile.vfd.definition, MatrixDefinitionNames));
    processing.insert(QLatin1String(VfdKey), vfd);
    auto vector = processing.value(QLatin1String(VectorKey)).toObject();
    const auto vectorSettings = SoftwareProcessing::normalized(profile.vector);
    vector.insert(QLatin1String(VectorBeamWidthKey), vectorSettings.beamWidth);
    vector.insert(QLatin1String(CrtCurvatureKey), vectorSettings.curvature);
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
namespace {
struct DisplayRasterHistory {
    QImage current;
    QImage previous;
    QImage packed;
    std::chrono::nanoseconds timestamp{};
    double elapsedMilliseconds = 0.0;

    void update(const QImage& image, std::chrono::nanoseconds time, bool advance, bool allowEqualTimestamp = false) {
        if (!advance && !current.isNull() && current.size() == image.size() && timestamp == time) return;
        const bool continuous = !current.isNull() && current.size() == image.size() &&
            (timestamp < time || (allowEqualTimestamp && timestamp == time));
        previous = continuous ? current : QImage{};
        elapsedMilliseconds = continuous ? std::chrono::duration<double, std::milli>(time - timestamp).count() : 0.0;
        timestamp = time;
        current = image;
        packed = {};
    }

    bool prepareTexture();
};

QImage makeDisplayHistoryTexture(const QImage& current, const QImage& previous) {
    const int width = current.width();
    if (width > std::numeric_limits<int>::max() / 2) return {};
    const auto format = current.hasAlphaChannel() ? QImage::Format_ARGB32 : QImage::Format_RGB32;
    const auto currentPixels = current.convertToFormat(format);
    const auto previousPixels = previous.convertToFormat(format);
    if (currentPixels.isNull() || previousPixels.isNull()) return {};
    QImage image(width * 2, current.height(), format);
    if (image.isNull()) return {};
    // Both frames share one point-sampled texture and use separate halves.
    const auto rowBytes = qsizetype(width) * sizeof(QRgb);
    for (int y = 0; y < image.height(); ++y) {
        auto* row = image.scanLine(y);
        std::copy_n(currentPixels.constScanLine(y), rowBytes, row);
        std::copy_n(previousPixels.constScanLine(y), rowBytes, row + rowBytes);
    }
    return image;
}

bool DisplayRasterHistory::prepareTexture() {
    if (previous.isNull() || !packed.isNull()) return true;
    packed = makeDisplayHistoryTexture(current, previous);
    return !packed.isNull();
}

struct EPaperRasterHistory {
    SoftwareProcessing::EPaperRefreshClock clock;
    SoftwareProcessing::EPaperRefresh refresh{};
    QImage retained;
    QImage packed;
    qint64 sourceKey = 0;

    void update(const QImage& source, const EPaperProfile& profile, std::chrono::nanoseconds timestamp) {
        refresh = clock.advance(profile, source.size(), timestamp);
        if (refresh.captureHistory) retained = source;
        if (sourceKey != source.cacheKey() || refresh.captureHistory) {
            sourceKey = source.cacheKey();
            packed = {};
        }
    }

    bool prepareTexture(const QImage& source) {
        if (!refresh.hasPrevious || !packed.isNull()) return true;
        packed = makeDisplayHistoryTexture(source, retained);
        return !packed.isNull();
    }
};
}
class VideoSurface::RefreshState {
public:
    BlackFrameCadence cadence;
    ImageRestoration hardwareRestoration;
    SoftwareProcessing::DisplayShaderParameters hardwareDisplay;
    QImage displaySource;
    DisplayRasterHistory segmentRaster;
    DisplayRasterHistory plasmaRaster;
    DisplayRasterHistory lcdRaster;
    quint64 lcdSequence = 0;
    DisplayRasterHistory vectorRaster;
    float plasmaAverageLight = 0.f;
    quint64 plasmaSequence = 0;
    EPaperRasterHistory ePaperRaster;
    SoftwareProcessing::ProjectionColorHistory projectionColor;
    std::optional<Emulation::VideoSegmentLevels> segmentLevels;
    Emulation::VideoVectorSegments vectors;
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
void VideoSurface::resetDisplayHistory() {
    *temporal_ = {};
    refresh_->segmentRaster = {};
    refresh_->plasmaRaster = {};
    refresh_->plasmaSequence = 0;
    refresh_->plasmaAverageLight = 0.f;
    refresh_->vectorRaster = {};
    refresh_->lcdRaster = {};
    refresh_->lcdSequence = 0;
    refresh_->ePaperRaster = {};
    refresh_->projectionColor = {};
}

void VideoSurface::setProfile(Profile profile) {
    const auto requested = profile.renderer;
    if (requestedRenderer_ != requested) failedRenderer_.reset();
    requestedRenderer_ = requested;
    if (failedRenderer_ == requested) profile.renderer = Renderer::Software;

    for (const auto& entry : PanelSignalFields) {
        auto& panel = profile.*std::get<1>(entry);
        normalizePanelInput(std::get<2>(entry), panel);
    }
    if (profile_.displayDeinterlacing() != profile.displayDeinterlacing()) motionSequence_.reset();
    const auto usesLcd = [](const Profile& value) {
        const auto screen = value.screenType();
        return screen == ScreenType::Lcd || screen == ScreenType::LedBacklitLcd;
    };
    const bool rendererChanged = profile_.renderer != profile.renderer;
    const bool lcdChanged = (usesLcd(profile_) || usesLcd(profile)) && profile_ != profile;
    const bool resetHistory = rendererChanged || lcdChanged || profile_.displayTechnology() != profile.displayTechnology() ||
        profile_.fixedPixel != profile.fixedPixel || profile_.fixedPixelEnabled != profile.fixedPixelEnabled ||
        profile_.effectiveImageAdjustments() != profile.effectiveImageAdjustments() || profile_.signal != profile.signal || profile_.sampling != profile.sampling ||
        profile_.crt.enabled != profile.crt.enabled ||
        profile_.restoration != profile.restoration || profile_.plasma != profile.plasma ||
        profile_.lcdSignal != profile.lcdSignal || profile_.ledSignal != profile.ledSignal ||
        profile_.oledSignal != profile.oledSignal || profile_.plasmaSignal != profile.plasmaSignal ||
        profile_.projectionSignal != profile.projectionSignal || profile_.ledBacklight != profile.ledBacklight ||
        profile_.lcd != profile.lcd || profile_.oled != profile.oled ||
        profile_.vector != profile.vector || profile_.vfd != profile.vfd || profile_.dotMatrix != profile.dotMatrix ||
        profile_.segmentDisplay != profile.segmentDisplay || profile_.ePaper != profile.ePaper || profile_.projection != profile.projection ||
        profile_.deinterlacing != profile.deinterlacing;
    if (resetHistory) resetDisplayHistory();
    if (processing_) processing_->invalidate(resetHistory);
    const bool clockChanged = profile_.blackFrameInsertion != profile.blackFrameInsertion;
    if (rendererChanged) setShaderLoading(false);
    profile_ = std::move(profile);
    if (clockChanged) updateRefreshClock(isVisible());
    if (profile_.renderer == Renderer::OpenGL) {
        if (!glSurface_) {
            glSurface_ = new OpenGlSurface([this](QPainter& painter) { draw(painter, true); },
                [this] { setShaderLoading(true); },
                [this] { samplingFinished(Renderer::OpenGL); },
                [this] { rendererFailed(Renderer::OpenGL); }, this);
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
                [this] { samplingFinished(Renderer::Direct3D11); },
                [this] { rendererFailed(Renderer::Direct3D11); }, this);
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
        if (!vulkanSurface_ && window && window->hasFailed()) window.reset();
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
                [this] { samplingFinished(Renderer::Vulkan); },
                [this] { rendererFailed(Renderer::Vulkan); });
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
        if (refresh_->vulkanWindow->hasFailed()) refresh_->vulkanWindow.reset();
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
    scheduleUpdate();
}

void VideoSurface::hideEvent(QHideEvent* event) {
    resumePresentation_ = true;
    if (vulkanSurface_) vulkanSurface_->hide();
    if (processing_) processing_->invalidate(true);
    updateRefreshClock(false);
    QWidget::hideEvent(event);
}

void VideoSurface::submit(std::shared_ptr<const Emulation::VideoFrame> frame) {
    submit(std::move(frame), std::nullopt);
}

void VideoSurface::submit(std::shared_ptr<const Emulation::VideoFrame> frame,
        std::optional<Emulation::VideoSegmentLevels> segmentLevels) {
    submit(std::move(frame), segmentLevels, {});
}

void VideoSurface::submit(std::shared_ptr<const Emulation::VideoFrame> frame,
        std::optional<Emulation::VideoSegmentLevels> segmentLevels, Emulation::VideoVectorSegments vectors) {
    submit(std::move(frame), segmentLevels, std::move(vectors), {});
}

void VideoSurface::submit(std::shared_ptr<const Emulation::VideoFrame> frame,
        std::optional<Emulation::VideoSegmentLevels> segmentLevels, Emulation::VideoVectorSegments vectors,
        const QList<std::shared_ptr<const Emulation::VideoFrame>>& previousFields) {
    const bool resumed = frame && isVisible() && std::exchange(resumePresentation_, false);
    if (resumed) {
        resetDisplayHistory();
        if (processing_) processing_->invalidate(true);
    }
    if (frame == frame_) {
        if (frame && (resumed || segmentLevels != refresh_->segmentLevels || bool(vectors) != bool(refresh_->vectors))) {
            refresh_->segmentLevels = segmentLevels;
            refresh_->vectors = std::move(vectors);
            compose(resumed);
        }
        return;
    }
    if (!frame) {
        clear();
        return;
    }
    if (frame->scanMode != Emulation::ScanMode::Progressive) {
        const auto start = std::max(qsizetype{0}, previousFields.size() - Emulation::IMachineVideoFieldHistory::MaximumFields);
        for (auto index = start; index < previousFields.size(); ++index) {
            const auto& field = previousFields[index];
            if (!field || field->sequence >= frame->sequence || field->timestamp >= frame->timestamp ||
                (frame_ && field->sequence <= frame_->sequence)) continue;
            acceptFrame(field);
        }
    }
    acceptFrame(std::move(frame));
    refresh_->segmentLevels = segmentLevels;
    refresh_->vectors = std::move(vectors);
    compose(true);
}

namespace {
bool consecutiveFields(const Emulation::VideoFrame& previous, const Emulation::VideoFrame& frame) {
    const auto rate = frame.signalFieldRate;
    const auto maximumGap = rate > 0 && std::isfinite(rate) ? std::clamp(1.75 / rate, .001, .15) : .1;
    return frame.scanMode != Emulation::ScanMode::Progressive && previous.scanMode != Emulation::ScanMode::Progressive &&
        frame.scanMode != previous.scanMode && frame.sequence > previous.sequence &&
        frame.sequence - previous.sequence == 1 && frame.timestamp > previous.timestamp &&
        std::chrono::duration<double>(frame.timestamp - previous.timestamp).count() <= maximumGap &&
        frame.width == previous.width && frame.height == previous.height && frame.pitch == previous.pitch &&
        frame.aspectRatio == previous.aspectRatio && frame.pixelFormat == previous.pixelFormat;
}

SignalStandard signalStandardForRate(double hz) {
    constexpr auto threshold = std::chrono::seconds(1) / PalDurationThreshold;
    return std::isfinite(hz) && hz > 0 && hz < threshold ? SignalStandard::Pal : SignalStandard::Ntsc;
}
}

void VideoSurface::acceptFrame(std::shared_ptr<const Emulation::VideoFrame> frame) {
    const auto rate = frame->signalFieldRate;
    if (!frame_ || source_.isNull() || !consecutiveFields(*frame_, *frame)) {
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
    if (frame_ && duration.count() > 0) {
        refresh_->machineHz = double(frame->sequence - previousSequence) / std::chrono::duration<double>(duration).count();
    } else if (rate > 0 && std::isfinite(rate)) {
        refresh_->machineHz = rate;
    } else {
        refresh_->machineHz = 0;
    }
    detectedSignal_ = signalStandardForRate(refresh_->machineHz);
    frame_ = std::move(frame);
    source_ = decode(*frame_);
}
void VideoSurface::compose(bool advance) {
    if (!frame_) return;
    auto signal = profile_.signal;
    if (signal.standard == SignalStandard::Automatic) signal.standard = detectedSignal_;
    const auto composed = SoftwareProcessing::composeFields(source_, previousField_, sameParity_,
        previousComplement_, *frame_, profile_.displayDeinterlacing(), motion_, motionSequence_);
    const QSize frameSize(frame_->width, frame_->height);
    const bool nativeVectors = profile_.displayTechnology() == DisplayTechnology::Vector &&
        refresh_->vectors && !frameSize.isEmpty();
    const auto sourceSize = nativeVectors ? frameSize : source_.size();
    const auto composedSize = nativeVectors ? frameSize : composed.size();
    const auto imageAspect = SoftwareProcessing::frameAspectRatio(*frame_, composedSize);
    const auto outputSize = imageViewport(composedSize, imageAspect).size() * devicePixelRatioF();
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
    bool gpuCanRenderDisplay = (glSurface_ && glSurface_->canSampleDisplay()) ||
        (vulkanSurface_ && vulkanSurface_->canSampleDisplay());
#ifdef Q_OS_WIN
    gpuCanRenderDisplay |= d3dSurface_ && d3dSurface_->canSampleDisplay();
#endif
    const auto technology = profile_.displayTechnology();
    if (technology != DisplayTechnology::Projection) refresh_->projectionColor = {};
    auto& vectorRaster = refresh_->vectorRaster;
    if (technology == DisplayTechnology::Vector && !nativeVectors && !composed.isNull())
        vectorRaster.update(composed, frame_->timestamp, advance);
    else
        vectorRaster = {};
    if (technology != DisplayTechnology::Plasma) refresh_->plasmaRaster = {};
    if (technology != DisplayTechnology::FixedPixel) refresh_->lcdRaster = {};
    auto& ePaper = refresh_->ePaperRaster;
    if (technology == DisplayTechnology::EPaper && !composed.isNull())
        ePaper.update(composed, profile_.ePaper, frame_->timestamp);
    else
        ePaper = {};
    const auto segmentLevels = refresh_->segmentLevels;
    const bool nativeSegments = frame_->pixelFormat == Emulation::PixelFormat::Xrgb8888 &&
        segmentLevels && segmentLevels->onRed > segmentLevels->offRed;
    auto& segmentRaster = refresh_->segmentRaster;
    if (technology == DisplayTechnology::SegmentDisplay && !nativeSegments && !composed.isNull() &&
        (profile_.segmentDisplay.responseTimeMilliseconds > 0 || profile_.segmentDisplay.persistenceMilliseconds > 0)) {
        segmentRaster.update(composed, frame_->timestamp, advance);
    } else {
        segmentRaster = {};
    }
    if (!composed.isNull() && !outputSize.isEmpty() && gpuCanRenderDisplay &&
        technology == DisplayTechnology::Vector && !nativeVectors && vectorRaster.prepareTexture()) {
        if (processing_) processing_->invalidate(false);
        presentImage(composed, sourceSize, imageAspect, Sampling::Nearest, adjustments, {}, technology);
        return;
    }
    if (!composed.isNull() && !outputSize.isEmpty() && gpuCanRenderDisplay &&
        technology == DisplayTechnology::SegmentDisplay && nativeSegments &&
        frame_->scanMode == Emulation::ScanMode::Progressive) {
        if (processing_) processing_->invalidate(true);
        temporal_->segmentDisplay = {};
        presentImage(composed, source_.size(), imageAspect, Sampling::Nearest, {}, {}, technology);
        return;
    }
    if (!composed.isNull() && !outputSize.isEmpty() && gpuCanRenderDisplay &&
        technology == DisplayTechnology::Crt) {
        if (processing_) processing_->invalidate(false);
        presentImage(composed, source_.size(), imageAspect, Sampling::Nearest, adjustments, {}, DisplayTechnology::Crt);
        return;
    }
    if (!composed.isNull() && !outputSize.isEmpty() && gpuCanRenderDisplay &&
        (technology == DisplayTechnology::Projection || technology == DisplayTechnology::Plasma ||
            technology == DisplayTechnology::FixedPixel) &&
        !adjustments.sharpness &&
        !SoftwareProcessing::hasSignalEffect(signal) && !restoration.debanding && !restoration.detailRecovery) {
        if (processing_) processing_->invalidate(false);
        presentImage(composed, source_.size(), imageAspect, Sampling::Nearest, adjustments,
            SoftwareProcessing::restorationForDisplay(profile_), technology);
        return;
    }
    refresh_->projectionColor = {};
    refresh_->plasmaRaster = {};
    refresh_->lcdRaster = {};
    if (!composed.isNull() && !outputSize.isEmpty() && gpuCanRenderDisplay &&
        (technology == DisplayTechnology::LedMatrix || technology == DisplayTechnology::Vfd ||
            technology == DisplayTechnology::DotMatrix || technology == DisplayTechnology::EPaper ||
            (technology == DisplayTechnology::SegmentDisplay && !nativeSegments)) && !adjustments.sharpness &&
        !SoftwareProcessing::hasSignalEffect(signal) &&
        SoftwareProcessing::restorationForDisplay(profile_) == ImageRestoration{} &&
        (technology != DisplayTechnology::SegmentDisplay || segmentRaster.prepareTexture()) &&
        (technology != DisplayTechnology::EPaper || ePaper.prepareTexture(composed))) {
        if (processing_) processing_->invalidate(false);
        presentImage(composed, source_.size(), imageAspect, Sampling::Nearest, adjustments, {}, technology);
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
    const auto requiresProcessing = (!composed.isNull() || nativeVectors) &&
        SoftwareProcessing::requiresProcessing(profile_, signal, composedSize, outputSize);
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
        if (isVisible()) processing_->submit({composed, sourceSize, outputSize, profile_, signal, frame_, advance,
            refresh_->segmentLevels, refresh_->vectors});
        return;
    }
    presentImage(SoftwareProcessing::process(composed, profile_, signal, *frame_, outputSize, *temporal_, advance,
        refresh_->segmentLevels, refresh_->vectors), sourceSize, imageAspect);
}

void VideoSurface::presentImage(QImage image, QSize sourceSize, double aspectRatio,
    std::optional<Sampling> hardwareSampling, ImageAdjustments hardwareAdjustments, ImageRestoration hardwareRestoration, DisplayTechnology hardwareDisplay) {
    hardwareSampling_ = hardwareSampling;
    hardwareAdjustments_ = hardwareAdjustments;
    refresh_->hardwareRestoration = hardwareRestoration;
    refresh_->hardwareDisplay = {};
    refresh_->displaySource = {};
    const auto preparePanelSignal = [&](const QImage& prepared, const DisplaySignalProfile& signal,
                                       bool hardwareDecoder, float& decoder) {
        if (hardwareDecoder) {
            const SoftwareProcessing::CrtSignal raster(signal, *frame_, 1.f);
            if (raster.ratio == 0.f) {
                decoder = raster.decoder == SoftwareProcessing::CrtDecoder::Separate ? 0.f : float(raster.decoder) + 1.f;
                return SoftwareProcessing::filterCrtConnection(prepared, signal, *frame_);
            }
        }
        return SoftwareProcessing::filterPanelSignal(prepared, signal, *frame_);
    };
    if (hardwareDisplay == DisplayTechnology::Crt && frame_) {
        const auto crt = SoftwareProcessing::normalized(profile_.crt);
        const auto displayAspect = float(SoftwareProcessing::screenAspectRatio(profile_, aspectRatio));
        refresh_->hardwareDisplay.crt = SoftwareProcessing::crtShaderParameters(crt, *frame_, displayAspect, hardwareAdjustments.brightness);
        const auto restoration = SoftwareProcessing::restorationForDisplay(profile_);
        const auto prepared = SoftwareProcessing::dedither(image, restoration.dedithering);
        refresh_->displaySource = SoftwareProcessing::filterCrtConnection(prepared, crt, *frame_);
    } else if (hardwareDisplay == DisplayTechnology::LedMatrix) {
        const auto& led = profile_.ledMatrix;
        const auto grid = matrixSize(led.definition, image.size());
        refresh_->hardwareDisplay.ledMatrix = {float(grid.width()), float(grid.height()), float(led.color), float(led.shape)};
    } else if (hardwareDisplay == DisplayTechnology::Vfd) {
        const auto& vfd = profile_.vfd;
        const auto grid = matrixSize(vfd.definition, image.size());
        const auto phosphor = SoftwareProcessing::vfdPhosphor(vfd.color);
        refresh_->hardwareDisplay.vfdStructure = {float(grid.width()), float(grid.height()), float(vfd.shape), 0.f};
        refresh_->hardwareDisplay.vfdPhosphor = {phosphor[0], phosphor[1], phosphor[2], 0.f};
    } else if (hardwareDisplay == DisplayTechnology::DotMatrix) {
        const auto& dot = profile_.dotMatrix;
        const auto grid = matrixSize(dot.definition, image.size());
        const auto tint = SoftwareProcessing::dotMatrixTint(dot.palette);
        refresh_->hardwareDisplay.dotMatrixGeometry = {float(grid.width()), float(grid.height()),
            dot.palette == DotMatrixPalette::Rgb ? 1.f : 0.f, float(dot.shape)};
        refresh_->hardwareDisplay.dotMatrixOptics = {tint[0], tint[1], tint[2], float(dot.technology)};
    } else if (hardwareDisplay == DisplayTechnology::Vector) {
        const auto profile = SoftwareProcessing::normalized(profile_.vector);
        const auto outputSize = imageViewport(image.size(), aspectRatio).size() * devicePixelRatioF();
        const auto optics = SoftwareProcessing::vectorOptics(profile, outputSize);
        const auto& raster = refresh_->vectorRaster;
        const auto beamGain = std::exp2(float(hardwareAdjustments.brightness) / 10.f);
        const float decay = !raster.packed.isNull() && raster.elapsedMilliseconds > 0.0
            ? std::exp2(-float(raster.elapsedMilliseconds) / optics.halfLife) : 0.f;
        refresh_->hardwareDisplay.vectorPhosphor = {optics.tint[0], optics.tint[1], optics.tint[2], optics.diffusion};
        refresh_->hardwareDisplay.vectorBeam = {std::ceil(optics.sigma * 2.f), optics.scale,
            .16f * float(profile.curvature) / CrtProfile::MaximumCurvature, decay};
        constexpr float NeighborSamples = 8.f, WideNeighborSamples = 12.f;
        const float sharpnessSamples = hardwareAdjustments.sharpness == 0 ? 0.f :
            hardwareAdjustments.sharpness > 0 ? NeighborSamples : WideNeighborSamples;
        refresh_->hardwareDisplay.vectorSource = {float(image.width()), beamGain, sharpnessSamples, 0.f};
        refresh_->displaySource = raster.packed;
    } else if (hardwareDisplay == DisplayTechnology::FixedPixel && frame_) {
        const auto technology = profile_.fixedPixel.technology;
        const bool led = technology == FixedPixelTechnology::LedBacklitLcd;
        const auto& signal = *profile_.panelSignal();
        const auto panel = signal.panelSize(image.size());
        auto& layout = refresh_->hardwareDisplay;
        layout.fixedGrid = {float(panel.width()), float(panel.height()), float(technology), float(image.width())};
        auto& raster = refresh_->lcdRaster;
        if (technology == FixedPixelTechnology::Oled) {
            const auto prepared = SoftwareProcessing::dedither(image, hardwareRestoration.dedithering);
            refresh_->displaySource = SoftwareProcessing::filterPanelSignal(prepared, signal, *frame_);
            const auto& oled = profile_.oled;
            layout.fixedOptics = {float(oled.pixelStructure), float(oled.panelType), 0.f,
                SoftwareProcessing::oledFrameLight(refresh_->displaySource)};
            layout.fixedLight[0] = SoftwareProcessing::oledLimiter(oled.panelType);
            raster = {};
        } else {
            const auto& lcd = led ? static_cast<const LcdProfile&>(profile_.ledBacklight) : profile_.lcd;
            const auto backlight = profile_.ledBacklight.type;
            const bool hardwareDecoder = !led || backlight == LedBacklightType::Edge || backlight == LedBacklightType::Direct;
            const SoftwareProcessing::CrtSignal rasterSignal(signal, *frame_, 1.f);
            if (hardwareDecoder && rasterSignal.ratio == 0.f && rasterSignal.decoder != SoftwareProcessing::CrtDecoder::Separate)
                layout.fixedTiming[1] = float(rasterSignal.decoder) + 1.f;
            if (raster.current.isNull() || raster.current.size() != image.size() ||
                    raster.timestamp != frame_->timestamp || refresh_->lcdSequence != frame_->sequence) {
                const auto prepared = SoftwareProcessing::dedither(image, hardwareRestoration.dedithering);
                auto decoded = preparePanelSignal(prepared, signal, hardwareDecoder, layout.fixedTiming[1]);
                if (led) {
                    const auto zones = SoftwareProcessing::ledBacklightZones(decoded, backlight);
                    if (!zones.empty()) {
                        auto illuminated = decoded.convertToFormat(QImage::Format_ARGB32);
                        if (!illuminated.isNull()) {
                            for (int y = 0; y < illuminated.height(); ++y) {
                                auto* row = reinterpret_cast<QRgb*>(illuminated.scanLine(y));
                                for (int x = 0; x < illuminated.width(); ++x)
                                    row[x] = (row[x] & 0x00ffffffu) | (quint32(zones[size_t(y) * illuminated.width() + x]) << 24);
                            }
                            decoded = std::move(illuminated);
                        }
                    }
                }
                if (frame_->sequence < refresh_->lcdSequence) raster = {};
                raster.update(decoded, frame_->timestamp, true);
                refresh_->lcdSequence = frame_->sequence;
                raster.prepareTexture();
            }
            refresh_->displaySource = raster.packed.isNull() ? raster.current : raster.packed;
            const auto optics = SoftwareProcessing::lcdOptics(lcd.panelType);
            const auto intensity = float(std::clamp(lcd.intensity, 0, FixedPixelProfile::PercentMaximum)) /
                FixedPixelProfile::PercentMaximum;
            layout.fixedOptics = {float(lcd.pixelStructure), float(lcd.panelType), float(lcd.orientation), float(backlight)};
            layout.fixedLight = {intensity, optics.black, optics.leak, optics.responseMilliseconds};
            layout.fixedTiming[0] = raster.packed.isNull() ? 0.f : float(raster.elapsedMilliseconds);
        }
    } else if (hardwareDisplay == DisplayTechnology::Plasma && frame_) {
        using Emission = SoftwareProcessing::PlasmaEmissionSettings;
        const auto& signal = profile_.plasmaSignal;
        const auto panel = signal.panelSize(image.size());
        auto& raster = refresh_->plasmaRaster;
        if (raster.current.isNull() || raster.current.size() != image.size() ||
            raster.timestamp != frame_->timestamp || refresh_->plasmaSequence != frame_->sequence) {
            const auto prepared = SoftwareProcessing::dedither(image, hardwareRestoration.dedithering);
            const auto decoded = SoftwareProcessing::filterPanelSignal(prepared, signal, *frame_);
            if (frame_->sequence < refresh_->plasmaSequence) raster = {};
            raster.update(decoded, frame_->timestamp, true, frame_->sequence > refresh_->plasmaSequence);
            refresh_->plasmaSequence = frame_->sequence;
            refresh_->plasmaAverageLight = SoftwareProcessing::plasmaFrameLight(decoded);
            raster.prepareTexture();
        }
        const auto phase = float(frame_->sequence % Emission::DitherPhases);
        refresh_->hardwareDisplay.plasmaGeometry = {float(panel.width()), float(panel.height()),
            float(profile_.plasma.cellStructure), float(image.width())};
        refresh_->hardwareDisplay.plasmaEmission = {Emission::PhosphorIntensity, Emission::BrightnessLimiter,
            Emission::Dithering, Emission::Persistence};
        refresh_->hardwareDisplay.plasmaTiming = {phase, refresh_->plasmaAverageLight, 0.f, 0.f};
        refresh_->displaySource = raster.packed.isNull() ? raster.current : raster.packed;
    } else if (hardwareDisplay == DisplayTechnology::Projection && frame_) {
        const auto projection = SoftwareProcessing::normalized(profile_.projection);
        const auto& signal = profile_.projectionSignal;
        const auto optics = SoftwareProcessing::projectionOptics(projection);
        const auto panelSize = signal.panelSize(image.size());
        refresh_->hardwareDisplay.projectionGeometry = {float(panelSize.width()), float(panelSize.height()), 0.f, 0.f};
        refresh_->hardwareDisplay.projectionOptics = {optics.black, optics.blur, optics.border, optics.convergence};
        refresh_->hardwareDisplay.projectionScreen = {optics.reflection, optics.ambient, optics.texture, 0.f};
        const auto prepared = SoftwareProcessing::dedither(image, hardwareRestoration.dedithering);
        refresh_->displaySource = preparePanelSignal(prepared, signal,
            projection.technology != ProjectionTechnology::SingleChipDlp, refresh_->hardwareDisplay.projectionGeometry[2]);
        if (projection.technology == ProjectionTechnology::SingleChipDlp) {
            refresh_->displaySource = SoftwareProcessing::separateProjectionColors(
                refresh_->displaySource, *frame_, projection, signal, refresh_->projectionColor);
        } else {
            refresh_->projectionColor = {};
        }
    } else if (hardwareDisplay == DisplayTechnology::EPaper) {
        const auto& panel = profile_.ePaper;
        const auto& raster = refresh_->ePaperRaster;
        const auto grid = DisplaySignalProfile{.definition = panel.definition}.panelSize(image.size());
        refresh_->hardwareDisplay.ePaperInkAndColor = {float(grid.width()), float(grid.height()), float(panel.colorMode), float(image.width())};
        refresh_->hardwareDisplay.ePaperSurface = SoftwareProcessing::ePaperSurface(panel.colorMode);
        refresh_->hardwareDisplay.ePaperTemporal = {raster.refresh.currentWeight, raster.refresh.inversion, 0.f,
            raster.refresh.hasPrevious ? 1.f : 0.f};
        if (raster.refresh.hasPrevious) refresh_->displaySource = raster.packed;
    } else if (hardwareDisplay == DisplayTechnology::SegmentDisplay) {
        const auto profile = SoftwareProcessing::normalized(profile_.segmentDisplay);
        const auto tint = SoftwareProcessing::segmentTint(profile.color);
        refresh_->hardwareDisplay.nativeSegmentOptics = {tint[0], tint[1], tint[2], float(profile.technology)};
        const auto levels = refresh_->segmentLevels;
        if (frame_ && frame_->pixelFormat == Emulation::PixelFormat::Xrgb8888 && levels && levels->onRed > levels->offRed) {
            constexpr float ByteMaximum = std::numeric_limits<quint8>::max();
            refresh_->hardwareDisplay.nativeSegmentLevels = {levels->offRed / ByteMaximum, levels->onRed / ByteMaximum, 0.f, 0.f};
        } else {
            const auto& raster = refresh_->segmentRaster;
            refresh_->hardwareDisplay.rasterSegments = SoftwareProcessing::rasterSegmentParameters(
                profile, image.size(), raster.elapsedMilliseconds);
            refresh_->displaySource = raster.packed;
        }
    }
    image_ = std::move(image);
    imageSourceSize_ = sourceSize;
    imageAspectRatio_ = aspectRatio;
    ++refresh_->image;
    if (!refresh_->active) scheduleUpdate();
}

void VideoSurface::clear() {
    if (processing_) processing_->invalidate(true);
    frame_.reset();
    refresh_->segmentLevels.reset();
    refresh_->vectors.reset();
    resetDisplayHistory();
    refresh_->machineHz = 0;
    refresh_->active = false;
    refresh_->cadence = {};
    refresh_->presentedImage.reset();
    blackFrame_ = false;
    source_ = previousField_ = sameParity_ = previousComplement_ = image_ = {};
    hardwareSampling_.reset();
    hardwareAdjustments_ = {};
    refresh_->hardwareRestoration = {};
    refresh_->hardwareDisplay = {};
    refresh_->displaySource = {};
    motion_.clear();
    motionSequence_.reset();
    scheduleUpdate();
}
void VideoSurface::setOsdTransparency(int percent) {
    percent = std::clamp(percent, 0, FullTransparency);
    if (percent == osdTransparency_) return;
    osdTransparency_ = percent;
    overlay_ = {};
    update();
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
void VideoSurface::rendererFailed(Renderer renderer) {
    if (profile_.renderer != renderer || requestedRenderer_ != renderer || failedRenderer_ == renderer) return;
    failedRenderer_ = renderer;
    QTimer::singleShot(0, this, [this, renderer] {
        if (profile_.renderer == renderer && failedRenderer_ == renderer) setProfile(profile_);
    });
}

void VideoSurface::samplingFinished(Renderer renderer) {
    if (profile_.renderer != renderer) return;
    setShaderLoading(false);
    if (processing_) processing_->invalidate(false);
    compose();
}
namespace {
QImage renderSnapshot(const QImage& composed, const Emulation::VideoFrame& frame, const Profile& profile,
        SignalSimulation signal, QSize viewport, std::optional<Emulation::VideoSegmentLevels> segments,
        Emulation::VideoVectorSegments vectors) {
    const bool nativeVectors = profile.displayTechnology() == DisplayTechnology::Vector && vectors;
    const QSize sourceSize = nativeVectors ? QSize(frame.width, frame.height) : composed.size();
    if (sourceSize.isEmpty()) return {};
    const auto aspect = SoftwareProcessing::frameAspectRatio(frame, sourceSize);
    const auto screen = fit(viewport.expandedTo(QSize(1, 1)), SoftwareProcessing::screenAspectRatio(profile, aspect));
    SoftwareProcessing::DisplayHistory display;
    return SoftwareProcessing::process(composed, profile, signal, frame, fit(screen.size(), aspect).size(),
        display, true, segments, std::move(vectors));
}
}

std::function<QImage()> VideoSurface::snapshotTask() const {
    if (profile_.renderer == Renderer::Software) {
        if (image_.isNull()) return {};
        return [image = image_] { return image; };
    }
    if (!frame_) return {};
    auto signal = profile_.signal;
    if (signal.standard == SignalStandard::Automatic) signal.standard = detectedSignal_;
    const QSize viewport = size() * devicePixelRatioF();
    return [frame = frame_, profile = profile_, signal, viewport,
        source = source_, previous = previousField_, sameParity = sameParity_, complement = previousComplement_,
        motion = motion_, sequence = motionSequence_, segments = refresh_->segmentLevels, vectors = refresh_->vectors]() mutable {
        const auto composed = SoftwareProcessing::composeFields(source, previous, sameParity, complement,
            *frame, profile.displayDeinterlacing(), motion, sequence);
        return renderSnapshot(composed, *frame, profile, signal, viewport, segments, vectors);
    };
}

QImage VideoSurface::softwareSnapshot(const Emulation::VideoFrame& frame, const Profile& profile, QSize viewport,
        const QList<std::shared_ptr<const Emulation::VideoFrame>>& previousFields,
        std::optional<Emulation::VideoSegmentLevels> segments, Emulation::VideoVectorSegments vectors) {
    auto signal = profile.signal;
    if (signal.standard == SignalStandard::Automatic) signal.standard = signalStandardForRate(frame.signalFieldRate);
    std::array<QImage, Emulation::IMachineVideoFieldHistory::MaximumFields> history;
    const auto mode = profile.displayDeinterlacing();
    if (mode != Deinterlacing::Off) {
        const auto* current = &frame;
        auto previous = previousFields.crbegin();
        for (auto& field : history) {
            if (previous == previousFields.crend() || !*previous || !consecutiveFields(**previous, *current)) break;
            field = decode(**previous);
            if (field.isNull()) break;
            current = previous->get();
            ++previous;
        }
    }
    QList<float> motion;
    std::optional<quint64> motionSequence;
    const auto composed = SoftwareProcessing::composeFields(decode(frame), history[0], history[1], history[2],
        frame, mode, motion, motionSequence);
    return renderSnapshot(composed, frame, profile, signal, viewport, segments, std::move(vectors));
}

QImage VideoSurface::currentImage() const {
    if (!hardwareSampling_ || image_.isNull()) return image_;
    const QSize outputSize = imageViewport().size() * devicePixelRatioF();
    const auto& gpuSource = refresh_->displaySource.isNull() ? image_ : refresh_->displaySource;
    if (glSurface_) {
        const auto capture = glSurface_->captureSampled(gpuSource, imageViewport(), *hardwareSampling_, hardwareAdjustments_,
            refresh_->hardwareRestoration, refresh_->hardwareDisplay);
        if (!capture.isNull()) return capture;
    }
    if (vulkanSurface_) {
        const auto capture = vulkanSurface_->captureSampled(gpuSource, imageViewport(), *hardwareSampling_, hardwareAdjustments_,
            refresh_->hardwareRestoration, refresh_->hardwareDisplay);
        if (!capture.isNull()) return capture;
    }
#ifdef Q_OS_WIN
    if (d3dSurface_) {
        const auto capture = d3dSurface_->captureSampled(gpuSource, imageViewport(), *hardwareSampling_, hardwareAdjustments_,
            refresh_->hardwareRestoration, refresh_->hardwareDisplay);
        if (!capture.isNull()) return capture;
    }
#endif
    if (refresh_->hardwareDisplay.enabled() && frame_) {
        SoftwareProcessing::DisplayHistory history;
        history.projection = refresh_->projectionColor;
        return SoftwareProcessing::process(image_, profile_, {}, *frame_, outputSize, history, false, refresh_->segmentLevels);
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
    const auto& gpuSource = refresh_->displaySource.isNull() ? image_ : refresh_->displaySource;
#ifdef Q_OS_WIN
    if (d3dSurface_)
        d3dSurface_->setScene(blackFrame_ ? QImage{} : gpuSource, imageViewport(),
            hardwareSampling_.value_or(profile_.displaySampling() == Sampling::Linear ? Sampling::Linear : Sampling::Nearest), overlay_, hardwareAdjustments_, refresh_->hardwareRestoration, refresh_->hardwareDisplay);
#endif
    if (usesOpenGl()) glSurface_->update();
    if (vulkanSurface_) {
        if (hardwareSampling_)
            vulkanSurface_->setSampledScene(blackFrame_ ? QImage{} : gpuSource, imageViewport(), *hardwareSampling_,
                overlay_, hardwareAdjustments_, refresh_->hardwareRestoration, refresh_->hardwareDisplay);
        else
            vulkanSurface_->setSampledScene({}, {}, Sampling::Nearest, {});
    }
    update();
}

void VideoSurface::refreshViewport() {
    overlay_ = {};
    if (hardwareSampling_ || usesSoftwareSampling(profile_.displaySampling()) || profile_.displayTechnology() != DisplayTechnology::Normal ||
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
        const bool hardwareDisplay = refresh_->hardwareDisplay.enabled();
        const auto& gpuSource = refresh_->displaySource.isNull() ? image_ : refresh_->displaySource;
        const bool sampled = openGl && hardwareSampling_ && glSurface_->drawSampled(painter, gpuSource, bounds,
            *hardwareSampling_, hardwareAdjustments_, refresh_->hardwareRestoration, refresh_->hardwareDisplay);
        if (!sampled) {
            painter.setRenderHint(QPainter::SmoothPixmapTransform, profile_.displaySampling() == Sampling::Linear);
            if (hardwareDisplay && frame_) {
                SoftwareProcessing::DisplayHistory history;
                history.projection = refresh_->projectionColor;
                painter.drawImage(bounds, SoftwareProcessing::process(image_, profile_, {}, *frame_,
                    bounds.size() * devicePixelRatioF(), history, false, refresh_->segmentLevels));
            } else painter.drawImage(bounds, image_);
        }
    }
    drawOverlay(painter);
}

void VideoSurface::drawOverlay(QPainter& painter) {
    painter.setRenderHint(QPainter::Antialiasing);
    paintOsd(painter, rect(), statistics_, Qt::AlignTop | Qt::AlignRight, osdTransparency_);
    paintOsd(painter, rect(), indicator_, Qt::AlignCenter, osdTransparency_);
}
}
