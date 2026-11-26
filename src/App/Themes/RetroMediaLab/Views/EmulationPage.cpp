#include "EmulationPage.h"
#include <ErrorKeys.h>
#include <RetroMediaLab/Emulation/ExecutionRate.h>
#include "EmulationCommandIcons.h"
#include "EmulationMachineTabs.h"
#include "Shared/Controls/Card.h"
#include "Shared/Controls/FlowLayout.h"
#include "Shared/Presentation/ThemeService.h"
#include "../ThemeIcons.h"
#include "MediaPageConstants.h"
#include "Shared/Controls/StatusIndicator.h"
#include "Shared/Presentation/AssetPath.h"
#include "../ThemeConstants.h"
#include "Shared/Emulation/EmulationWorkspace.h"
#include "Shared/Emulation/EmulationControllerFeedback.h"
#include "EmulationStoragePanel.h"
#include "Shared/Controls/MachineInputBindings.h"
#include "MachineControllerPorts.h"
#include "Shared/Controls/ControllerOutputs.h"
#include "Shared/Controls/ControllerArtwork.h"
#include "Shared/Emulation/EmulationHostKeyboard.h"
#include "EmulationPreferencesDialog.h"
#include "Shared/Emulation/EmulationKeyboardShortcuts.h"
#include "Shared/Emulation/EmulationPhysicalShortcuts.h"
#include "Shared/Controls/ModuleEditableSlider.h"
#include "Shared/Controls/ModuleNumberEdit.h"
#include "Shared/Controls/ModuleIpv4Edit.h"
#include "EmulationErrorDialog.h"
#include "TapeTransportButton.h"
#include <RetroMediaLab/Emulation/Messages.h>
#include "ModuleSettingPresentation.h"
#include "Shared/Emulation/ModuleNumericSetting.h"
#include "Shared/Controls/RememberedFileDialog.h"
#include "State/WorkspaceState.h"
#include <RetroMediaLab/Emulation/Host.h>
#include <RetroMediaLab/Emulation/ControllerFeedback.h>
#include <RetroMediaLab/Emulation/Firmware.h>
#include <RetroMediaLab/Emulation/SettingsRules.h>
#include <RetroMediaLab/Platform/Gamepads.h>
#include <RetroMediaLab/Translations/TranslationService.h>
#include <RetroMediaLab/VideoPresentation/FullscreenOsd.h>

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QCursor>
#include <QDialog>
#include <QDialogButtonBox>
#include <QTabBar>
#include <QDir>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QScrollArea>
#include <QKeyEvent>
#include <QLabel>
#include <QLocale>
#include <QMenu>
#include <QMouseEvent>
#include <QPointer>
#include <QThreadPool>
#include <QPushButton>
#include <QProgressBar>
#include <QPixmap>
#include <QShortcut>
#include <RetroMediaLab/Platform/MidiInput.h>
#include <QScopedValueRollback>
#include <QSignalBlocker>
#include <QSlider>
#include <QSpinBox>
#include <QStyle>
#include <QTabWidget>
#include <QTimer>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>
#include <iterator>
#include <stdexcept>
#include <system_error>
#include <array>
#include <atomic>
#include <thread>

namespace RetroMediaLab {
namespace {
using namespace Emulation;
constexpr auto KeyboardTabKey = "Emulation.Tab.Keyboard";
constexpr auto MouseTabKey = "Emulation.Tab.Mouse";
constexpr auto ControllerTabKey = "Emulation.Tab.Controllers";
constexpr auto ControllerPortKey = "Emulation.Controller.Port";
constexpr auto OsdControlsKey = "Controllers.Controls";
constexpr auto ControllerTypeKey = "Emulation.Controller.Type";
constexpr auto ControllerNoneKey = "Emulation.Controller.None";
constexpr std::array OsdControlTabs{KeyboardTabKey, MouseTabKey, ControllerTabKey};
constexpr std::array OsdMachineTabs{
    "Emulation.Tab.General", "Emulation.Tab.Cpu", "Emulation.Tab.Rom", "Emulation.Tab.Ram",
    "Emulation.Tab.Extensions", "Emulation.Tab.Storage", "Emulation.Tab.Cheats", "Emulation.Tab.Ethernet"};
using VideoPresentation::FullscreenOsd;
using VideoPresentation::VideoSurface;

GraphicsApi graphicsApi(VideoPresentation::Renderer renderer) {
    using VideoPresentation::Renderer;
    switch (renderer) {
    case Renderer::OpenGL: return GraphicsApi::OpenGL;
    case Renderer::Vulkan: return GraphicsApi::Vulkan;
    case Renderer::Direct3D11: return GraphicsApi::Direct3D11;
    case Renderer::Software: break;
    }
    return GraphicsApi::Automatic;
}
using Translation = Translations::TranslationService;

constexpr int FramePollMilliseconds = 16;
constexpr int MediaPollMilliseconds = 200;
constexpr int TapeBlinkMilliseconds = 400;
constexpr auto VideoKey = "Emulation.Tab.Video";
constexpr auto VideoRendererKey = "Emulation.Video.Settings.Rendering";
constexpr std::array VideoRenderers{
#ifdef Q_OS_WIN
    std::pair{VideoPresentation::Renderer::Direct3D11, "Emulation.Video.Renderer.Direct3D11"},
#endif
    std::pair{VideoPresentation::Renderer::Software, "Qt"},
    std::pair{VideoPresentation::Renderer::Vulkan, "Emulation.Video.Renderer.Vulkan"},
    std::pair{VideoPresentation::Renderer::OpenGL, "Emulation.Video.Renderer.OpenGL"}
};
constexpr auto VideoShaderLoadingKey = "Emulation.Video.Shader.Loading";
constexpr auto AspectKey = "Emulation.Video.AspectRatio";
constexpr auto SamplingKey = "Emulation.Video.Sampling";
constexpr auto BlackFrameKey = "Emulation.Video.Parameter.BlackFrameInsertion";
constexpr auto NearestKey = "Emulation.Video.Sampling.Nearest";
constexpr auto BilinearKey = "Emulation.Video.Sampling.Bilinear";
constexpr std::array VideoSamplers{
    std::pair{VideoPresentation::Sampling::Nearest, NearestKey},
    std::pair{VideoPresentation::Sampling::Linear, BilinearKey},
    std::pair{VideoPresentation::Sampling::SharpBilinear, "Emulation.Video.Sampling.SharpBilinear"},
    std::pair{VideoPresentation::Sampling::Bicubic, "Emulation.Video.Sampling.Bicubic"},
    std::pair{VideoPresentation::Sampling::Xbr, "Emulation.Video.Sampling.Xbr"},
    std::pair{VideoPresentation::Sampling::Xbrz, "Emulation.Video.Sampling.Xbrz"},
    std::pair{VideoPresentation::Sampling::ScaleFx, "Emulation.Video.Sampling.ScaleFx"},
    std::pair{VideoPresentation::Sampling::ScaleNx, "Emulation.Video.Sampling.ScaleNx"},
    std::pair{VideoPresentation::Sampling::Sabr, "Emulation.Video.Sampling.Sabr"},
    std::pair{VideoPresentation::Sampling::Hq2x, "Emulation.Video.Sampling.Hq2x"},
    std::pair{VideoPresentation::Sampling::Hq3x, "Emulation.Video.Sampling.Hq3x"},
    std::pair{VideoPresentation::Sampling::Hq4x, "Emulation.Video.Sampling.Hq4x"},
    std::pair{VideoPresentation::Sampling::TwoXSai, "Emulation.Video.Sampling.TwoXSai"},
    std::pair{VideoPresentation::Sampling::SuperTwoXSai, "Emulation.Video.Sampling.SuperTwoXSai"},
    std::pair{VideoPresentation::Sampling::SuperEagle, "Emulation.Video.Sampling.SuperEagle"},
    std::pair{VideoPresentation::Sampling::EpxScale2x, "Emulation.Video.Sampling.EpxScale2x"},
    std::pair{VideoPresentation::Sampling::Jinc2, "Emulation.Video.Sampling.Jinc2"},
    std::pair{VideoPresentation::Sampling::Lanczos, "Emulation.Video.Sampling.Lanczos"},
};
constexpr auto LegacyHqxKey = "Emulation.Video.Sampling.Hqx";

auto videoSamplers(VideoPresentation::Sampling current) {
    std::vector options(VideoSamplers.begin(), VideoSamplers.end());
    if (current == VideoPresentation::Sampling::Hqx)
        options.emplace_back(current, LegacyHqxKey);
    return options;
}
constexpr auto OsdPauseKey = "Emulation.Shortcut.OsdWithPause";
constexpr auto OsdLiveKey = "Emulation.Shortcut.OsdWithoutPause";
constexpr auto OsdResumeKey = "Emulation.Shortcut.OsdResume";
constexpr auto OsdSavedStatesKey = "Emulation.Shortcut.OsdSavedStates";
constexpr auto OsdNavigateKey = "Emulation.Shortcut.OsdNavigateHint";
constexpr auto OsdAdjustKey = "Common.Modify";
constexpr auto OsdConfirmKey = "Emulation.Shortcut.OsdConfirmHint";
constexpr auto OsdBackKey = "Emulation.Shortcut.OsdBackHint";
constexpr auto OsdVideoSettingsKey = "Emulation.Shortcut.OsdVideoSettings";
constexpr auto AudioTabKey = "Emulation.Tab.Audio";
constexpr auto OsdInformationKey = "Menu.Information";
constexpr std::pair<const char*, EmulationCommandIcons::Glyph> OsdInformationTabs[]{
    {"Emulation.Tab.General", EmulationCommandIcons::Settings},
    {"Emulation.Tab.Cpu", EmulationCommandIcons::Processor},
    {"Emulation.Tab.Rom", EmulationCommandIcons::File},
    {"Emulation.Tab.Ram", EmulationCommandIcons::Memory},
    {VideoKey, EmulationCommandIcons::Display},
    {AudioTabKey, EmulationCommandIcons::Audio},
    {"Emulation.Tab.Extensions", EmulationCommandIcons::Connection}};
constexpr auto OsdAudioKey = "Emulation.Shortcut.OsdAudioSettings";
constexpr auto AutomaticSettingKey = "Emulation.Controller.Automatic";
constexpr auto ModuleAudioEnabledKey = "Emulation.Audio.Enabled";
constexpr auto AudioOutputKey = "Emulation.Audio.Output";
constexpr auto OsdExitKey = "Emulation.Shortcut.OsdExitFullscreen";
constexpr auto ScreenshotKey = "Emulation.Shortcut.Screenshot";
constexpr auto ShortcutUnavailableKey = "Emulation.Shortcut.Unavailable";
constexpr auto EmulationMediaScope = "EmulationMedia/%1/%2/%3";
constexpr auto EmulationStateScope = "EmulationState";
constexpr auto CloseKey = "Common.Cancel";

constexpr auto BrandKey = "Emulation.Configuration.Brand";
constexpr auto MachineKey = "Emulation.Configuration.Machine";
constexpr auto SelectorKey = "Emulation.Configuration.Selector";
constexpr auto SelectorToggleKey = "Emulation.Configuration";
constexpr auto SettingsKey = "Emulation.Machine.Settings";
constexpr auto OpenKey = "Emulation.Machine.Open";
constexpr auto WelcomeKey = "Emulation.Tab.Welcome";
constexpr auto WelcomeTextKey = "Emulation.Welcome.Text";
constexpr auto PowerKey = "Emulation.Shortcut.Power";
constexpr auto PauseResumeKey = "Emulation.Shortcut.PauseResume";
constexpr auto SoftResetKey = "Emulation.Native.SoftReset";
constexpr auto HardResetKey = "Emulation.Native.HardReset";
constexpr auto FullScreenKey = "Emulation.Native.FullScreen";
constexpr auto StatisticsKey = "Emulation.Native.Statistics";
constexpr auto FpsKey = "Emulation.Native.Fps";
constexpr auto ExecutionRateKey = "Emulation.Status.ExecutionRate";
constexpr auto FrameStatisticsFormat = "%1 × %2 · %3 Hz · %4";
constexpr auto StatisticsSizeFormat = "%1 × %2";
constexpr auto StatisticsFrequencyFormat = "%1 Hz";
constexpr auto StatisticsClockFormat = "%1 MHz";
constexpr auto StatisticsRendererKey = "Emulation.Video.Renderer.Direct3D11";
constexpr auto StatisticsVulkanKey = "Emulation.Video.Renderer.Vulkan";
constexpr auto StatisticsOpenGlKey = "Emulation.Video.Renderer.OpenGL";
constexpr auto SoftwareRendererName = "Qt";
constexpr double HertzPerMegahertz = 1'000'000.0;
constexpr double PercentScale = 100;
constexpr auto VolumeLabelKey = "Explorer.Volume";
constexpr auto AudioMixerKey = "Emulation.Audio.Mixer";
constexpr auto AudioPsgKey = "Emulation.Sega.Audio.Psg";
constexpr auto AudioFmKey = "Emulation.Sega.Audio.Fm";
constexpr auto Ym2413CoreId = "segaemu_ym2413_core";
constexpr auto MixerVolumeSuffix = "_volume";
constexpr auto MixerPreampSuffix = "_preamp";
constexpr auto MixerEqualizerInfix = "_audio_eq_";
constexpr auto MuteIndicatorKey = "Emulation.Shortcut.Mute";
constexpr int MinimumVolume = 0, MaximumVolume = 100, VolumeStep = 1;
constexpr auto QuickSaveKey = "Emulation.Shortcut.QuickSave";
constexpr auto QuickLoadKey = "Emulation.Shortcut.QuickLoad";
constexpr auto SaveStateKey = "Emulation.Native.SaveState";
constexpr auto LoadStateKey = "Emulation.Native.LoadState";
constexpr auto ErrorKey = "Emulation.Error.EmulatorRejected";
constexpr auto MediaOperationFailedKey = "Emulation.Error.MediaOperationFailed";
constexpr auto StateOperationFailedKey = "Emulation.Error.SavedStateOperationFailed";
constexpr auto SaveKey = "Common.Save";
constexpr auto InsertKey = "Emulation.Media.Insert";
constexpr auto AssociatedMediaKey = "Emulation.Storage.Media.Associated";
constexpr auto MediaReadOnlyKey = "Emulation.Storage.ReadOnly";
constexpr auto MediaNameFilter = "%1 (%2)";
constexpr auto AnyExtension = "*";
constexpr auto CartridgeMediaIcon = "rom-memory", HardDiskMediaIcon = "data";
constexpr std::pair<double, const char*> FloppyMediaIcons[]{
    {3.0, "media-floppy-3"}, {3.5, "media-floppy-3.5-dd"}, {5.25, "media-floppy-5.25"}, {8.0, "media-floppy-8"},
};
constexpr auto ActiveIndicator = 0x32cd32u, MountedIndicator = 0x228b22u, EmptyIndicator = 0x808080u;
constexpr auto MediaActivityPersistence = std::chrono::milliseconds(140);
constexpr int MediaIconSize = 18;
constexpr auto EjectKey = "Emulation.Media.Eject";
constexpr auto EjectGlyph = u'\u23cf';
constexpr QSize EjectButtonSize{22, 20};
constexpr QMargins MediaDevicePadding{4, 0, 4, 0};
constexpr int MediaDeviceSpacing = 3;
constexpr auto MediaDeviceStyle = "QFrame#emulationMediaDevice { border-right: 1px solid #D7DEE7; }";
constexpr auto MediaDeviceObjectName = "emulationMediaDevice";
constexpr QMargins TapeTransportMargin{18, 2, 2, 3};
constexpr int TapeTransportPadding = 2;
constexpr int TapeTransportButtonSpacing = 2;
constexpr auto TapeTransportObjectName = "emulationTapeTransport";
constexpr auto TapeTransportStyle = "QFrame#emulationTapeTransport {"
    "background-color: %1; border: 1px solid %2; border-radius: 6px; }";
constexpr auto AudioKey = "Emulation.Tab.Audio";
constexpr auto ControllerStatusKey = "Emulation.Tab.Controllers";
constexpr auto PointerStatusKey = "Emulation.Tab.Mouse";
constexpr auto SwitchControllerPointerKey = "Emulation.Controller.Action.SwitchJoystickMouse";
constexpr auto EnabledStatusKey = "Emulation.Value.Enabled";
constexpr auto DisabledStatusKey = "Emulation.Value.Disabled";
constexpr auto ReleasePointerKey = "Emulation.Shortcut.ReleaseMouse";
constexpr auto RescanKey = "Emulation.Native.Rescan";
constexpr auto TapeRawExtension = "raw";
constexpr auto TapePcmOutputKey = "Emulation.Tape.Pcm.Output";
constexpr auto TapePcmPrefix = "tape.pcm.";
constexpr auto TapeFormatFilter = "%1 (*.%2)";
constexpr std::array TapeSampleRates{22050, 44100, 48000, 96000, 192000};
constexpr std::array TapeChannels{1, 2};
constexpr std::array TapeSampleSizes{8, 16, 24, 32};

struct TapeControl {
    TapeCommand command;
    const char* label;
};
constexpr std::array TapeControls{
    TapeControl{TapeCommand::Record, "Emulation.Tape.Command.Record"},
    TapeControl{TapeCommand::Play, "Emulation.Tape.Command.Play"},
    TapeControl{TapeCommand::Rewind, "Emulation.Tape.Command.Rewind"},
    TapeControl{TapeCommand::FastForward, "Emulation.Tape.Command.FastForward"},
    TapeControl{TapeCommand::Stop, "Emulation.Tape.Command.Stop"},
    TapeControl{TapeCommand::Pause, "Emulation.Tape.Command.Pause"}
};
constexpr std::array TapeStates{"Emulation.Tape.State.Empty", "Emulation.Tape.State.Stopped",
    "Emulation.Tape.State.Playing", "Emulation.Tape.State.Paused", "Emulation.Tape.State.Recording",
    "Emulation.Tape.State.EndOfTape"};

using namespace VideoPresentation;

constexpr auto DotMatrixTechnologyResource = "Emulation.Video.Technology.DotMatrix";
constexpr auto DotPaletteResource = "Emulation.Video.Parameter.DotMatrixPalette";
constexpr auto DotShapeResource = "Emulation.Video.Parameter.DotMatrixShape";
constexpr auto DotTechnologyResource = "Emulation.Video.Parameter.DotMatrixTechnology";
constexpr auto VfdTechnologyResource = "Emulation.Video.Technology.Vfd";
constexpr auto VfdColorResource = "Emulation.Video.Parameter.VfdColor";
constexpr auto VfdShapeResource = "Emulation.Video.Parameter.VfdShape";
constexpr auto VectorPhosphorResource = "Emulation.Video.Parameter.VectorPhosphorColor";
constexpr auto VectorBeamWidthResource = "Emulation.Video.Parameter.VectorBeamWidth";
constexpr auto VectorCurvatureResource = "Emulation.Video.Parameter.CrtCurvature";
constexpr auto VectorScreenResource = "Emulation.Video.Screen.Vector";
constexpr auto PlasmaCellStructureResource = "Emulation.Video.Parameter.PlasmaCellStructure";
constexpr std::array plasmaCellChoices{
    std::pair{PlasmaCellStructure::Stripe, "Emulation.Video.Plasma.CellStructure.Stripe"},
    std::pair{PlasmaCellStructure::Waffle, "Emulation.Video.Plasma.CellStructure.Waffle"},
    std::pair{PlasmaCellStructure::Delta, "Emulation.Video.Plasma.CellStructure.Delta"},
    std::pair{PlasmaCellStructure::Monochrome, "Emulation.Video.Subpixels.Monochrome"}
};
constexpr std::array maskChoices{
    std::pair{CrtMask::None, "Emulation.Video.Crt.Mask.None"},
    std::pair{CrtMask::ApertureGrille, "Emulation.Video.Crt.Mask.ApertureGrille"},
    std::pair{CrtMask::ShadowMask, "Emulation.Video.Crt.Mask.ShadowMask"},
    std::pair{CrtMask::SlotMask, "Emulation.Video.Crt.Mask.SlotMask"},
    std::pair{CrtMask::StaggeredSlotMask, "Emulation.Video.Crt.Mask.StaggeredSlotMask"},
    std::pair{CrtMask::Monochrome, "Emulation.Video.Crt.Mask.Monochrome"},
    std::pair{CrtMask::HitachiEdp, "Emulation.Video.Crt.Mask.HitachiEdp"}
};
constexpr std::array rateChoices{
    std::pair{CrtFieldRate::Hz50, "Emulation.Video.Crt.FieldRate.Hz50"},
    std::pair{CrtFieldRate::Hz60, "Emulation.Video.Crt.FieldRate.Hz60"},
    std::pair{CrtFieldRate::Hz50And60, "Emulation.Video.Crt.FieldRate.Hz50And60"}
};
constexpr std::array decoderChoices{
    std::pair{CrtColorStandards::Pal, "Emulation.Video.Crt.ColorStandards.Pal"},
    std::pair{CrtColorStandards::Secam, "Emulation.Video.Crt.ColorStandards.Secam"},
    std::pair{CrtColorStandards::Ntsc, "Emulation.Video.Crt.ColorStandards.Ntsc"},
    std::pair{CrtColorStandards::PalSecam, "Emulation.Video.Crt.ColorStandards.PalSecam"},
    std::pair{CrtColorStandards::PalNtsc, "Emulation.Video.Crt.ColorStandards.PalNtsc"},
    std::pair{CrtColorStandards::PalSecamNtsc, "Emulation.Video.Crt.ColorStandards.PalSecamNtsc"}
};
constexpr std::array connectionChoices{
    std::pair{CrtConnection::Digital, "Emulation.Video.Crt.Connection.Digital"},
    std::pair{CrtConnection::Scart, "Emulation.Video.Crt.Connection.Scart"},
    std::pair{CrtConnection::Composite, "Emulation.Video.Crt.Connection.Composite"},
    std::pair{CrtConnection::Component, "Emulation.Video.Crt.Connection.Component"},
    std::pair{CrtConnection::SVideo, "Emulation.Video.Crt.Connection.SVideo"},
    std::pair{CrtConnection::AnalogRf, "Emulation.Video.Crt.Connection.AnalogRf"},
    std::pair{CrtConnection::LegacyMonitor, "Emulation.Video.Crt.Connection.LegacyMonitor"}
};
template<class Options, class Values>
auto orderedChoices(const Options& source, const Values& values) {
    using Choice = typename Options::value_type;
    std::vector<Choice> options;
    options.reserve(values.size());
    for (const auto value : values) {
        const auto choice = std::ranges::find(source, value, &Choice::first);
        if (choice != source.end()) options.push_back(*choice);
    }
    return options;
}

constexpr auto PanelDefinitionResource = "Emulation.Video.Lcd.Definition";
constexpr std::array panelDefinitionChoices{
    std::pair{PanelDefinition::Native, "Emulation.Video.Lcd.Definition.Native"},
    std::pair{PanelDefinition::Vga, "Emulation.Video.Lcd.Definition.Vga"},
    std::pair{PanelDefinition::Svga, "Emulation.Video.Lcd.Definition.Svga"},
    std::pair{PanelDefinition::Xga, "Emulation.Video.Lcd.Definition.Xga"},
    std::pair{PanelDefinition::Hd, "Emulation.Video.Lcd.Definition.Hd"},
    std::pair{PanelDefinition::Sxga, "Emulation.Video.Lcd.Definition.Sxga"},
    std::pair{PanelDefinition::HdReady, "Emulation.Video.Lcd.Definition.HdReady"},
    std::pair{PanelDefinition::FullHd, "Emulation.Video.Lcd.Definition.FullHd"},
    std::pair{PanelDefinition::Qhd, "Emulation.Video.Lcd.Definition.Qhd"},
    std::pair{PanelDefinition::Uhd4K, "Emulation.Video.Lcd.Definition.Uhd4K"}
};
constexpr auto LcdPanelTypeResource = "Emulation.Video.Lcd.PanelType";
constexpr auto LedBacklightTypeResource = "Emulation.Video.Led.BacklightType";
constexpr std::array lcdPanelTypes{"Emulation.Video.Lcd.Tn", "Emulation.Video.Lcd.Ips", "Emulation.Video.Lcd.Va"};
constexpr std::array ledBacklightTypes{"Emulation.Video.Led.BacklightType.Edge", "Emulation.Video.Led.BacklightType.Direct",
    "Emulation.Video.Led.BacklightType.FullArray", "Emulation.Video.Led.BacklightType.MiniLed"};
constexpr auto LcdPixelStructureResource = "Emulation.Video.Lcd.PixelStructure";
constexpr std::array oledPanels{"Emulation.Video.Oled.Panel.RgbOled", "Emulation.Video.Oled.Panel.Woled",
    "Emulation.Video.Oled.Panel.QdOled"};
constexpr std::array oledStructures{"Emulation.Video.Lcd.NoGrid", "Emulation.Video.Oled.Structure.RgbVertical",
    "Emulation.Video.Oled.Structure.BgrVertical", "Emulation.Video.Oled.Structure.Wrgb",
    "Emulation.Video.Oled.Structure.TriangularRgb", "Emulation.Video.Oled.Structure.PenTileDiamond",
    "Emulation.Video.Subpixels.Monochrome"};
constexpr auto ProjectionTechnologyResource = "Emulation.Video.Projection.Technology";
constexpr std::array projectionTechnologies{
    "Emulation.Video.Projection.Technology.Crt", "Emulation.Video.Projection.Technology.ThreeLcd",
    "Emulation.Video.Projection.Technology.SingleChipDlp", "Emulation.Video.Projection.Technology.ThreeChipDlp",
    "Emulation.Video.Projection.Technology.Lcos"
};
constexpr auto ProjectionSurfaceResource = "Emulation.Video.Projection.Surface";
constexpr std::array projectionSurfaces{
    "Emulation.Video.Projection.Surface.WhiteScreen", "Emulation.Video.Projection.Surface.GrayScreen",
    "Emulation.Video.Projection.Surface.WhiteWall"
};
constexpr std::array palettes{
    std::pair{CrtMonitorPalette::Mda, "Emulation.Video.Crt.MonitorPalette.Mda"},
    std::pair{CrtMonitorPalette::Cga, "Emulation.Video.Crt.MonitorPalette.Cga"},
    std::pair{CrtMonitorPalette::Ega, "Emulation.Video.Crt.MonitorPalette.Ega"}
};
constexpr std::array fixedTypes{"Emulation.Video.FixedPixel.Technology.Lcd",
    "Emulation.Video.FixedPixel.Technology.LedBacklitLcd", "Emulation.Video.FixedPixel.Technology.Oled"};
constexpr auto LcdOrientationResource = "Emulation.Video.Lcd.PixelOrientation";
constexpr std::array lcdStructures{"Emulation.Video.Lcd.NoGrid", "Emulation.Video.Subpixels.Rgb",
    "Emulation.Video.Subpixels.Bgr", "Emulation.Video.Subpixels.Monochrome"};
constexpr std::array lcdOrientations{"Emulation.Video.Lcd.Vertical", "Emulation.Video.Lcd.Horizontal"};
constexpr std::array segmentLayoutLabels{"Emulation.Video.SegmentDisplay.Layout.Seven", "Emulation.Video.SegmentDisplay.Layout.Fourteen", "Emulation.Video.SegmentDisplay.Layout.Sixteen"};
constexpr auto SegmentTechnologyResource = "Emulation.Video.Parameter.SegmentDisplayTechnology";
constexpr auto SegmentColorResource = "Emulation.Video.Parameter.SegmentDisplayColor";
constexpr auto SegmentLayoutResource = "Emulation.Video.Parameter.SegmentDisplayLayout";
constexpr auto SegmentDisplayResource = "Emulation.Video.Technology.SegmentDisplay";
constexpr std::array segmentTechnologies{
    std::pair{SegmentDisplayTechnology::Lcd, "Emulation.Video.SegmentDisplay.Technology.Lcd"},
    std::pair{SegmentDisplayTechnology::Led, "Emulation.Video.SegmentDisplay.Technology.Led"},
    std::pair{SegmentDisplayTechnology::Vfd, "Emulation.Video.SegmentDisplay.Technology.Vfd"}
};
constexpr std::array segmentColorLabels{
    "Emulation.Video.SegmentDisplay.Color.Red", "Emulation.Video.SegmentDisplay.Color.Green",
    "Emulation.Video.SegmentDisplay.Color.Amber", "Emulation.Video.SegmentDisplay.Color.Blue",
    "Emulation.Video.SegmentDisplay.Color.White", "Emulation.Video.SegmentDisplay.Color.Black",
    "Emulation.Video.SegmentDisplay.Color.Gray", "Emulation.Video.SegmentDisplay.Color.BlueGreen"
};
constexpr std::array vectorColors{"Emulation.Video.Crt.Color.Color", "Emulation.Video.Crt.Color.Green",
    "Emulation.Video.Crt.Color.Amber", "Emulation.Video.Crt.Color.White"};
constexpr std::array inkModes{"Emulation.Video.EPaper.ColorMode.Monochrome",
    "Emulation.Video.EPaper.ColorMode.Grayscale16", "Emulation.Video.EPaper.ColorMode.Color4096",
    "Emulation.Video.EPaper.ColorMode.TricolorRed", "Emulation.Video.EPaper.ColorMode.TricolorYellow",
    "Emulation.Video.EPaper.ColorMode.Gallery"};
constexpr std::array displayTypes{
    std::pair{DisplayTechnology::Normal, "Emulation.Video.Technology.Normal"},
    std::pair{DisplayTechnology::Crt, "Emulation.Video.Screen.Crt"},
    std::pair{DisplayTechnology::FixedPixel, "Emulation.Video.Technology.FixedPixel"},
    std::pair{DisplayTechnology::Plasma, "Emulation.Video.Technology.Plasma"},
    std::pair{DisplayTechnology::Vector, "Emulation.Video.Technology.Vector"},
    std::pair{DisplayTechnology::Vfd, "Emulation.Video.Technology.Vfd"},
    std::pair{DisplayTechnology::LedMatrix, "Emulation.Video.Technology.LedMatrix"},
    std::pair{DisplayTechnology::DotMatrix, "Emulation.Video.Technology.DotMatrix"},
    std::pair{DisplayTechnology::SegmentDisplay, "Emulation.Video.Technology.SegmentDisplay"},
    std::pair{DisplayTechnology::EPaper, "Emulation.Video.Technology.EPaper"},
    std::pair{DisplayTechnology::Projection, "Emulation.Video.Technology.Projection"}
};

constexpr std::array osdScreenTypes{
    std::pair{ScreenType::Host, "Emulation.Video.Screen.Host"},
    std::pair{ScreenType::Crt, "Emulation.Video.Screen.Crt"},
    std::pair{ScreenType::Lcd, fixedTypes[0]},
    std::pair{ScreenType::LedBacklitLcd, "Emulation.Video.Screen.LedBacklitLcd"},
    std::pair{ScreenType::Oled, fixedTypes[2]},
    std::pair{ScreenType::Plasma, "Emulation.Video.Technology.Plasma"},
    std::pair{ScreenType::Projection, "Emulation.Video.Technology.Projection"},
    std::pair{ScreenType::Vector, "Emulation.Video.Screen.Vector"},
    std::pair{ScreenType::LedMatrix, "Emulation.Video.Technology.LedMatrix"},
    std::pair{ScreenType::DotMatrix, "Emulation.Video.Technology.DotMatrix"},
    std::pair{ScreenType::Vfd, "Emulation.Video.Screen.Vfd"},
    std::pair{ScreenType::SegmentDisplay, "Emulation.Video.Screen.SegmentDisplay"},
    std::pair{ScreenType::EPaper, "Emulation.Video.Screen.EPaper"}
};

constexpr std::array vfdColors{"Emulation.Video.Vfd.Color.BlueGreen", "Emulation.Video.Vfd.Color.Green",
    "Emulation.Video.Vfd.Color.Blue", "Emulation.Video.SegmentDisplay.Color.White",
    "Emulation.Video.Vfd.Color.Amber", "Emulation.Video.Vfd.Color.Red"};
constexpr std::array matrixDefinitionChoices{
    std::pair{MatrixDefinition::Native, "Emulation.Video.Lcd.Definition.Native"},
    std::pair{MatrixDefinition::Size16x16, "Emulation.Video.LedMatrix.Definition.Size16x16"},
    std::pair{MatrixDefinition::Size32x16, "Emulation.Video.LedMatrix.Definition.Size32x16"},
    std::pair{MatrixDefinition::Size32x32, "Emulation.Video.LedMatrix.Definition.Size32x32"},
    std::pair{MatrixDefinition::Size64x32, "Emulation.Video.LedMatrix.Definition.Size64x32"},
    std::pair{MatrixDefinition::Size64x64, "Emulation.Video.LedMatrix.Definition.Size64x64"},
    std::pair{MatrixDefinition::Size128x64, "Emulation.Video.LedMatrix.Definition.Size128x64"},
    std::pair{MatrixDefinition::Size128x128, "Emulation.Video.LedMatrix.Definition.Size128x128"},
    std::pair{MatrixDefinition::Size256x128, "Emulation.Video.LedMatrix.Definition.Size256x128"},
    std::pair{MatrixDefinition::Size256x256, "Emulation.Video.LedMatrix.Definition.Size256x256"}
};
constexpr auto LedMatrixTechnologyResource = "Emulation.Video.Technology.LedMatrix";
constexpr std::array ledColors{"Emulation.Video.LedMatrix.Color.Rgb", "Emulation.Video.LedMatrix.Color.Red",
    "Emulation.Video.LedMatrix.Color.Green", "Emulation.Video.LedMatrix.Color.Amber", "Emulation.Video.LedMatrix.Color.Blue",
    "Emulation.Video.LedMatrix.Color.White", "Emulation.Video.LedMatrix.Color.RedGreen"};
constexpr std::array ledShapes{"Emulation.Video.LedMatrix.Shape.Round", "Emulation.Video.LedMatrix.Shape.Square",
    "Emulation.Video.LedMatrix.Shape.RoundedSquare"};
constexpr std::array dotPalettes{"Emulation.Video.DotMatrix.Color.Black", "Emulation.Video.DotMatrix.Color.Gray",
    "Emulation.Video.DotMatrix.Color.Green", "Emulation.Video.DotMatrix.Color.Amber", "Emulation.Video.DotMatrix.Color.Red",
    "Emulation.Video.DotMatrix.Color.Blue", "Emulation.Video.DotMatrix.Color.Rgb"};
constexpr std::array dotTechnologies{"Emulation.Video.DotMatrix.Display.Reflective",
    "Emulation.Video.DotMatrix.Display.Backlit", "Emulation.Video.DotMatrix.Display.Emissive"};
constexpr std::array dotShapes{"Emulation.Video.DotMatrix.Shape.Round", "Emulation.Video.DotMatrix.Shape.Square",
    "Emulation.Video.DotMatrix.Shape.Rectangle"};
constexpr std::array phosphorChoices{
    std::pair{CrtPhosphor::Gray, "Emulation.Video.Crt.Color.Gray"},
    std::pair{CrtPhosphor::Green, "Emulation.Video.Crt.Color.Green"},
    std::pair{CrtPhosphor::Amber, "Emulation.Video.Crt.Color.Amber"}
};
constexpr std::array modes{
    std::pair{Deinterlacing::Off, "Emulation.Video.Deinterlacing.Off"},
    std::pair{Deinterlacing::Blend, "Emulation.Video.Deinterlacing.Blend"},
    std::pair{Deinterlacing::Bob, "Emulation.Video.Deinterlacing.Bob"},
    std::pair{Deinterlacing::Weave, "Emulation.Video.Deinterlacing.Weave"},
    std::pair{Deinterlacing::Adaptive, "Emulation.Video.Deinterlacing.Adaptive"}
};
constexpr std::array connections{
    std::pair{SignalConnection::None, "Emulation.Video.Signal.Connection.None"},
    std::pair{SignalConnection::RgbScart, "Emulation.Video.Signal.Connection.RgbScart"},
    std::pair{SignalConnection::Component, "Emulation.Video.Signal.Connection.Component"},
    std::pair{SignalConnection::SVideo, "Emulation.Video.Signal.Connection.SVideo"},
    std::pair{SignalConnection::Composite, "Emulation.Video.Signal.Connection.Composite"},
    std::pair{SignalConnection::Rf, "Emulation.Video.Signal.Connection.Rf"}
};
constexpr std::array standards{
    std::pair{SignalStandard::Automatic, "Emulation.Video.Signal.Standard.Automatic"},
    std::pair{SignalStandard::Pal, "Emulation.Video.Signal.Standard.Pal"},
    std::pair{SignalStandard::Ntsc, "Emulation.Video.Signal.Standard.Ntsc"},
    std::pair{SignalStandard::Secam, "Emulation.Video.Signal.Standard.Secam"}
};

QString text(const Translation& translations, const char* key) {
    return QString::fromStdString(translations.text(key));
}


class EmulatorPanel final : public QWidget {
public:
    EmulatorPanel(const Translation& translations, std::shared_ptr<Host> host, QString module, Configuration configuration,
        const EmulatorSelection& selection, const std::function<void(const QString&)>& select, QWidget* parent)
        : QWidget(parent), translations_(translations), host_(std::move(host)), module_(std::move(module)),
          configuration_(std::move(configuration)) {
        auto* layout = new QVBoxLayout(this);
        auto* form = new QFormLayout;
        layout->addLayout(form);
        emulators_ = new QComboBox(this);
        for (const auto& item : selection.available) emulators_->addItem(item.emulator.displayName, item.emulator.id);
        emulators_->setCurrentIndex(emulators_->findData(selection.selectedId));
        locked_ = selection.locked || emulators_->count() <= 1;
        form->addRow(label("Emulation.Core.Emulator"), emulators_);
        description_ = plainLabel(this);
        installed_ = plainLabel(this);
        layout->addWidget(description_);
        layout->addWidget(installed_);

        auto* actions = new QHBoxLayout;
        search_ = new QPushButton(label("Emulation.Core.NameSearch"), this);
        download_ = new QPushButton(label("Emulation.Core.NameDownload"), this);
        cancel_ = new QPushButton(label("Common.Cancel"), this);
        actions->addWidget(search_);
        versions_ = new QComboBox(this);
        actions->addWidget(versions_, 1);
        actions->addWidget(download_);
        actions->addWidget(cancel_);
        layout->addLayout(actions);
        status_ = plainLabel(this);
        layout->addWidget(status_);
        progress_ = new QProgressBar(this);
        progress_->setRange(0, ProgressScale);
        layout->addWidget(progress_);
        layout->addStretch();
        refresh(selection);
        connect(emulators_, &QComboBox::currentIndexChanged, this, [this, select] { select(emulators_->currentData().toString()); });
        connect(versions_, &QComboBox::currentIndexChanged, this, [this] { updateActions(); });
        connect(search_, &QPushButton::clicked, this, [this] { run(std::nullopt); });
        connect(download_, &QPushButton::clicked, this, [this] {
            const int index = versions_->currentIndex();
            if (index >= 0) run(releases_.at(index));
        });
        connect(cancel_, &QPushButton::clicked, this, [this] {
            worker_.request_stop();
            cancel_->setEnabled(false);
        });
        connect(&progressTimer_, &QTimer::timeout, this, [this] { progress_->setValue(progressValue_.load()); });
    }

    void finish() {
        if (!worker_.joinable()) return;
        worker_.request_stop();
        worker_.join();
    }

private:
    static constexpr int ProgressScale = 1000;
    static constexpr int ProgressUpdateMs = 50;

    static QLabel* plainLabel(QWidget* parent) {
        auto* widget = new QLabel(parent);
        widget->setWordWrap(true);
        widget->setTextFormat(Qt::PlainText);
        return widget;
    }

    QString label(const char* key) const { return text(translations_, key); }
    QString label(const char* key, const QString& argument) const {
        return label(key).replace(QStringLiteral("{0}"), argument);
    }

    void refresh(const EmulatorSelection& selection) {
        const auto found = std::ranges::find_if(selection.available, [&](const auto& item) {
            return item.emulator.id == selection.selectedId;
        });
        const auto current = found == selection.available.end() ? nullptr : &*found;
        canSearch_ = current && current->canSearchReleases;
        installedVersion_ = current ? current->installedVersion : std::nullopt;
        description_->setText(current ? host_->translate(module_, current->emulator.descriptionResourceKey,
            QString::fromStdString(translations_.language())) : QString{});
        installed_->setText(installedVersion_ ? label("Emulation.Core.NameInstalled", *installedVersion_)
            : label("Emulation.Core.NameNotInstalled"));
        status_->setText(!current ? label("Emulation.Core.NameNoEmulatorAvailable")
            : canSearch_ ? label("Emulation.Core.NameSearchPrompt") : QString{});
        releases_.clear();
        versions_->clear();
        updateActions();
    }

    void updateActions() {
        emulators_->setEnabled(!busy_ && !locked_);
        search_->setVisible(canSearch_);
        search_->setEnabled(!busy_);
        versions_->setVisible(!releases_.isEmpty());
        versions_->setEnabled(!busy_);
        download_->setVisible(!releases_.isEmpty());
        download_->setEnabled(!busy_ && versions_->currentIndex() >= 0);
        cancel_->setVisible(busy_);
        progress_->setVisible(busy_ && installing_);
    }

    void showReleases(QList<EmulatorRelease> found) {
        releases_ = std::move(found);
        versions_->clear();
        int selected = 0;
        int bestRank = 2;
        for (const auto& release : std::as_const(releases_)) {
            const bool installed = installedVersion_ && release.version == *installedVersion_;
            const auto rank = release.required ? 0 : installed ? 1 : 2;
            if (rank < bestRank) {
                bestRank = rank;
                selected = versions_->count();
            }
            versions_->addItem(installed ? label("Emulation.Core.NameInstalled", release.displayName) : release.displayName);
        }
        if (!releases_.isEmpty()) versions_->setCurrentIndex(selected);
        status_->setText(releases_.isEmpty() ? label("Emulation.Core.NameNoneFound") :
            label("Emulation.Core.NameVersionsFound", QString::number(releases_.size())));
    }

    void run(std::optional<EmulatorRelease> release) {
        busy_ = true;
        installing_ = release.has_value();
        cancel_->setEnabled(true);
        status_->setText(release ? label("Emulation.Core.NameDownloading", release->displayName)
            : label("Emulation.Core.NameSearching"));
        if (installing_) {
            progressValue_.store(0);
            progress_->setValue(0);
            progressTimer_.start(ProgressUpdateMs);
        } else {
            releases_.clear();
            versions_->clear();
        }
        updateActions();
        worker_ = std::jthread([this, release = std::move(release)](std::stop_token token) {
            QList<EmulatorRelease> found;
            QString installedPath;
            std::optional<EmulatorSelection> selection;
            bool failed = false;
            try {
                if (release) {
                    installedPath = host_->installEmulator(module_, configuration_, *release, [this](double value) {
                        if (std::isfinite(value))
                            progressValue_.store(static_cast<int>(std::clamp(value, 0., 1.) * ProgressScale));
                    }, token);
                    selection = host_->emulatorSelection(module_, configuration_, token);
                    failed = installedPath.isEmpty();
                } else {
                    found = host_->findEmulatorReleases(module_, configuration_, token);
                }
            } catch (...) {
                failed = true;
            }
            QMetaObject::invokeMethod(this, [this, token, failed, selection = std::move(selection),
                found = std::move(found), installedPath = std::move(installedPath)]() mutable {
                busy_ = false;
                progressTimer_.stop();
                if (selection) refresh(*selection);
                if (token.stop_requested()) status_->setText(label("Operation.Cancelled"));
                else if (failed) status_->setText(label(ErrorKey));
                else if (installing_) status_->setText(label("Emulation.Core.NameInstalledPath", installedPath));
                else showReleases(std::move(found));
                updateActions();
            }, Qt::QueuedConnection);
        });
    }

    const Translation& translations_;
    std::shared_ptr<Host> host_;
    QString module_;
    Configuration configuration_;
    QList<EmulatorRelease> releases_;
    std::optional<QString> installedVersion_;
    QComboBox* emulators_ = nullptr;
    QComboBox* versions_ = nullptr;
    QLabel* description_ = nullptr;
    QLabel* installed_ = nullptr;
    QLabel* status_ = nullptr;
    QPushButton* search_ = nullptr;
    QPushButton* download_ = nullptr;
    QPushButton* cancel_ = nullptr;
    QProgressBar* progress_ = nullptr;
    bool locked_ = false;
    bool canSearch_ = false;
    bool busy_ = false;
    bool installing_ = false;
    QTimer progressTimer_;
    std::atomic<int> progressValue_ = 0;
    std::jthread worker_;
};

std::optional<FirmwareCandidate> pickFirmware(QWidget* parent, const Translation& translations,
    const std::shared_ptr<Host>& host, const QString& module, const Configuration& configuration,
    const SettingsField& field, const QString& currentPath, const QString& directory) {
    constexpr auto Title = "Emulation.Firmware.Select";
    constexpr auto OpenFolder = "Emulation.Firmware.Rom.OpenFolder";
    constexpr auto Empty = "Emulation.Firmware.Empty";
    constexpr auto NotSelected = "Emulation.Firmware.NotSelected";
    constexpr auto Official = "Emulation.Firmware.Official";
    constexpr auto Partial = "Emulation.Firmware.PartiallyCompatible";
    QDialog picker(parent);
    picker.setWindowTitle(text(translations, Title));
    auto* layout = new QVBoxLayout(&picker);
    auto* actions = new QHBoxLayout;
    auto* refresh = new QPushButton(text(translations, RescanKey), &picker);
    auto* folder = new QPushButton(text(translations, OpenFolder), &picker);
    actions->addWidget(refresh);
    actions->addWidget(folder);
    actions->addStretch();
    layout->addLayout(actions);
    auto* list = new QListWidget(&picker);
    layout->addWidget(list, 1);
    auto* detail = new QLabel(&picker);
    detail->setTextFormat(Qt::PlainText);
    detail->setWordWrap(true);
    layout->addWidget(detail);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &picker);
    auto* choose = buttons->button(QDialogButtonBox::Save);
    choose->setText(text(translations, SaveKey));
    buttons->button(QDialogButtonBox::Cancel)->setText(text(translations, "Common.Cancel"));
    choose->setEnabled(false);
    layout->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &picker, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &picker, &QDialog::reject);
    QObject::connect(folder, &QPushButton::clicked, &picker, [&] {
        if (!QDir().mkpath(directory) || !QDesktopServices::openUrl(QUrl::fromLocalFile(directory)))
            detail->setText(text(translations, ErrorKey));
    });
    QList<FirmwareCandidate> candidates;
    QObject::connect(list, &QListWidget::currentRowChanged, &picker, [&](int row) {
        const bool valid = row >= 0 && row < candidates.size();
        choose->setEnabled(valid);
        if (!valid) return;
        const auto& candidate = candidates.at(row);
        QStringList metadata;
        if (!candidate.path.isEmpty()) metadata.append(candidate.path);
        if (candidate.compatibility == FirmwareCompatibility::Official) metadata.append(text(translations, Official));
        if (candidate.compatibility == FirmwareCompatibility::PartiallyCompatible) metadata.append(text(translations, Partial));
        if (!candidate.explanationResourceKey.isEmpty())
            metadata.append(host->translate(module, candidate.explanationResourceKey, QString::fromStdString(translations.language())));
        detail->setText(metadata.isEmpty() && candidates.size() == 1 ? text(translations, Empty) : metadata.join(u'\n'));
    });
    std::jthread worker;
    const auto scan = [&] {
        refresh->setEnabled(false);
        choose->setEnabled(false);
        list->clear();
        detail->clear();
        worker = std::jthread([&](std::stop_token token) {
            QList<FirmwareCandidate> entries;
            bool failed = false;
            try { entries = host->scanFirmware(module, configuration, token); }
            catch (...) { failed = true; }
            if (token.stop_requested()) return;
            QMetaObject::invokeMethod(&picker, [&, entries = std::move(entries), failed]() mutable {
                refresh->setEnabled(true);
                if (failed) {
                    detail->setText(text(translations, ErrorKey));
                    return;
                }
                // Prefer a recognized candidate for this field to an unclassified file of the same path.
                std::stable_sort(entries.begin(), entries.end(), [&](const auto& first, const auto& second) {
                    const bool firstMatches = first.destinationFieldId == field.id;
                    const bool secondMatches = second.destinationFieldId == field.id;
                    return firstMatches != secondMatches ? firstMatches : first.compatibility < second.compatibility;
                });
                candidates.clear();
                QSet<QString> seen;
                for (auto& entry : entries) {
                    const bool unknown = entry.destinationFieldId.isNull() &&
                        entry.compatibility == FirmwareCompatibility::Unknown;
                    if (!unknown && entry.destinationFieldId != field.id) continue;
                    const auto key = entry.path.toCaseFolded();
                    if (seen.contains(key)) continue;
                    seen.insert(key);
                    if (entry.compatibility == FirmwareCompatibility::Incompatible) continue;
                    entry.destinationFieldId = field.id;
                    candidates.append(std::move(entry));
                }
                std::stable_sort(candidates.begin(), candidates.end(), [](const auto& first, const auto& second) {
                    if (first.compatibility != second.compatibility) return first.compatibility < second.compatibility;
                    return QString::localeAwareCompare(first.displayName.toCaseFolded(), second.displayName.toCaseFolded()) < 0;
                });
                FirmwareCandidate none;
                none.destinationFieldId = field.id;
                none.displayName = text(translations, NotSelected);
                candidates.prepend(std::move(none));
                int selected = currentPath.isEmpty() ? 0 : -1;
                for (const auto& entry : std::as_const(candidates)) {
                    if (entry.path.compare(currentPath, Qt::CaseInsensitive) == 0) selected = list->count();
                    auto label = entry.displayName;
                    if (!entry.version.isEmpty()) label += QStringLiteral(" · ") + entry.version;
                    list->addItem(label);
                }
                list->setCurrentRow(selected);
            }, Qt::QueuedConnection);
        });
    };
    QObject::connect(refresh, &QPushButton::clicked, &picker, scan);
    scan();
    const int result = picker.exec();
    worker.request_stop();
    worker.join();
    const int selected = list->currentRow();
    return result == QDialog::Accepted && selected >= 0 ? std::optional{candidates.at(selected)} : std::nullopt;
}

QString emulationMachineName(const Host& host, const Translation& translations, const QString& moduleId,
    const Configuration& configuration) {
    if (!configuration.name.trimmed().isEmpty()) return configuration.name;
    const auto installed = host.modules();
    const auto module = std::ranges::find(installed, moduleId, &ModuleInfo::id);
    if (module == installed.cend()) return configuration.machineId;
    const auto machine = std::ranges::find(module->machines, configuration.machineId, &MachineDefinition::id);
    if (machine == module->machines.cend() || machine->displayResourceKey.isEmpty()) return configuration.machineId;
    const auto& key = machine->displayResourceKey;
    const auto localized = host.translate(moduleId, key, QString::fromStdString(translations.language()));
    if (localized != key) return localized;
    const auto shared = QString::fromStdString(translations.text(key.toStdString()));
    return shared.startsWith(QLatin1Char('[')) ? configuration.machineId : shared;
}

std::optional<Profile> editVideoProfile(QWidget* parent, const Translation& translations, const Profile& profile,
    const std::function<QWidget*(QWidget*)>& createShaderStatus = {}) {
    QDialog dialog(parent);
    dialog.setWindowTitle(text(translations, VideoKey));
    auto* container = new QVBoxLayout(&dialog);
    auto* tabs = new QTabWidget(&dialog);
    container->addWidget(tabs);
    constexpr auto DisplayTabKey = "Emulation.Video.Tab.Display";
    constexpr auto ImageTabKey = "Emulation.Video.Tab.Image";
    const auto addPage = [&](const char* key) {
        auto* scroll = new QScrollArea(tabs);
        scroll->setWidgetResizable(true);
        auto* page = new QWidget(scroll);
        scroll->setWidget(page);
        tabs->addTab(scroll, text(translations, key));
        return page;
    };
    auto* appearance = addPage(DisplayTabKey);
    auto* form = new QFormLayout(appearance);
    auto* image = addPage(ImageTabKey);
    auto* imageLayout = new QVBoxLayout(image);
    auto* imageForm = new QFormLayout;
    imageLayout->addLayout(imageForm);
    if (createShaderStatus) form->addRow(createShaderStatus(appearance));
    auto* renderer = new QComboBox(&dialog);
    for (const auto& [value, key] : VideoRenderers)
        renderer->addItem(text(translations, key), static_cast<int>(value));
    const auto configuredRenderer = renderer->findData(static_cast<int>(profile.renderer));
    renderer->setCurrentIndex(configuredRenderer >= 0 ? configuredRenderer : renderer->findData(static_cast<int>(Renderer::Software)));
    const auto currentRenderer = static_cast<Renderer>(renderer->currentData().toInt());
    form->addRow(text(translations, VideoRendererKey), renderer);
    auto* aspect = new QComboBox(&dialog);
    auto aspectDraft = profile.aspectRatio;
    QObject::connect(aspect, &QComboBox::activated, &dialog, [aspect, &aspectDraft] {
        aspectDraft = AspectRatio(aspect->currentData().toInt());
    });
    const auto updateAspects = [&](ScreenType screen) {
        aspect->clear();
        for (const auto ratio : aspectRatioChoices(screen)) {
            const auto dimensions = aspectRatioDimensions(ratio);
            const auto label = ratio == AspectRatio::Original
                ? text(translations, "Emulation.Native.AspectOriginal")
                : QString::number(dimensions.width()) + QLatin1Char('/') + QString::number(dimensions.height());
            aspect->addItem(label, int(ratio));
        }
        const int configured = aspect->findData(int(aspectDraft));
        aspect->setCurrentIndex(configured >= 0 ? configured : aspect->findData(int(AspectRatio::Original)));
    };
    form->addRow(text(translations, AspectKey), aspect);
    auto* sampling = new QComboBox(&dialog);
    for (const auto& [mode, label] : videoSamplers(profile.sampling))
        sampling->addItem(text(translations, label), static_cast<int>(mode));
    sampling->setCurrentIndex(sampling->findData(static_cast<int>(profile.sampling)));
    form->addRow(text(translations, SamplingKey), sampling);
    auto* blackFrame = new QCheckBox(&dialog);
    blackFrame->setChecked(profile.blackFrameInsertion);
    form->addRow(text(translations, BlackFrameKey), blackFrame);
    auto* deinterlacing = new QComboBox(&dialog);
    for (const auto& [mode, label] : modes) deinterlacing->addItem(text(translations, label), static_cast<int>(mode));
    const auto configured = profile.deinterlacing;
    const auto visibleMode = configured == Deinterlacing::BobEvenLines || configured == Deinterlacing::BobOddLines
        ? Deinterlacing::Bob : configured;
    deinterlacing->setCurrentIndex(deinterlacing->findData(static_cast<int>(visibleMode)));
    imageForm->addRow(text(translations, "Emulation.Video.Parameter.Deinterlacing"), deinterlacing);
    constexpr std::array levels{"Emulation.Video.Restoration.Level.None", "Emulation.Video.Restoration.Level.Light",
        "Emulation.Video.Restoration.Level.Medium", "Emulation.Video.Restoration.Level.Strong"};
    auto* dedithering = new QComboBox(&dialog);
    for (size_t index = 0; index < levels.size(); ++index)
        dedithering->addItem(text(translations, levels[index]), ImageRestoration::DeditheringLevels[index]);
    const int level = qRound(double(profile.restoration.dedithering) * (levels.size() - 1) / ImageRestoration::MaximumIntensity);
    dedithering->setCurrentIndex(level);
    imageForm->addRow(text(translations, "Emulation.Video.Parameter.Dedithering"), dedithering);
    const auto restorationValue = [&](const char* key, int value) {
        auto* spin = new QSpinBox(&dialog);
        spin->setRange(0, ImageRestoration::MaximumIntensity);
        spin->setValue(value);
        imageForm->addRow(text(translations, key), spin);
        return spin;
    };
    auto* debanding = restorationValue("Emulation.Video.Parameter.Debanding", profile.restoration.debanding);
    auto* detailRecovery = restorationValue("Emulation.Video.Parameter.DetailRecovery", profile.restoration.detailRecovery);
    detailRecovery->setToolTip(text(translations, "Emulation.Video.Parameter.FineDetails"));
    const auto adjustment = [&](const char* key, int value) {
        auto* edit = new QSpinBox(&dialog);
        edit->setRange(ImageAdjustments::Minimum, ImageAdjustments::Maximum);
        edit->setValue(value);
        imageForm->addRow(text(translations, key), edit);
        return edit;
    };
    auto adjustmentDraft = profile;
    auto adjustmentScreen = adjustmentDraft.screenType();
    const auto initialAdjustments = adjustmentDraft.imageAdjustments(adjustmentScreen);
    auto* brightness = adjustment("Emulation.Video.Parameter.Brightness", initialAdjustments.brightness);
    auto* contrast = adjustment("Emulation.Video.Parameter.Contrast", initialAdjustments.contrast);
    auto* gamma = new QComboBox(&dialog);
    for (const auto curve : ImageAdjustments::GammaCurves) {
        const auto label = curve == 0 ? text(translations, "Emulation.Video.Gamma.Original") :
            curve == ImageAdjustments::Bt1886 ? QStringLiteral("BT.1886") : QLocale().toString(curve / 100.0, 'f', 1);
        gamma->addItem(label, curve);
    }
    gamma->setCurrentIndex(gamma->findData(initialAdjustments.gamma));
    imageForm->addRow(text(translations, "Emulation.Video.Gamma"), gamma);
    auto* saturation = adjustment("Emulation.Video.Parameter.Saturation", initialAdjustments.saturation);
    auto* sharpness = adjustment("Emulation.Video.Parameter.Sharpness", initialAdjustments.sharpness);
    auto* signalPage = new QGroupBox(text(translations, "Emulation.Video.Tab.Signal"), image);
    auto* signalForm = new QFormLayout(signalPage);
    const auto signalChoice = [&](const auto& choices, auto active, const char* label) {
        auto* combo = new QComboBox(signalPage);
        for (const auto& [value, key] : choices) combo->addItem(text(translations, key), static_cast<int>(value));
        combo->setCurrentIndex(combo->findData(static_cast<int>(active)));
        signalForm->addRow(text(translations, label), combo);
        return combo;
    };
    const auto intensity = [&](const char* key, int value) {
        auto* spin = new QSpinBox(signalPage);
        spin->setRange(0, SignalSimulation::MaximumIntensity);
        spin->setValue(value);
        signalForm->addRow(text(translations, key), spin);
        return spin;
    };
    const auto* connection = signalChoice(connections, profile.signal.connection, "Emulation.Video.Parameter.SignalConnection");
    const auto* connectionIntensity = intensity("Emulation.Video.Parameter.SignalConnectionIntensity", profile.signal.connectionIntensity);
    const auto* standard = signalChoice(standards, profile.signal.standard, "Emulation.Video.Parameter.SignalStandard");
    const auto* standardIntensity = intensity("Emulation.Video.Parameter.SignalStandardIntensity", profile.signal.standardIntensity);
    auto* crtPage = new QWidget(image);
    imageLayout->addWidget(crtPage);
    imageLayout->addWidget(signalPage);
    imageLayout->addStretch();
    auto* technologyLayout = new QVBoxLayout(crtPage);
    technologyLayout->setContentsMargins({});
    auto* displayType = new QComboBox(appearance);
    for (const auto& [technology, key] : displayTypes)
        displayType->addItem(text(translations, key), int(technology));
    displayType->setCurrentIndex(displayType->findData(int(profile.displayTechnology())));
    form->insertRow(0, text(translations, "Emulation.Video.Technology"), displayType);
    auto* crtSettings = new QWidget(crtPage);
    technologyLayout->addWidget(crtSettings);
    auto* crtForm = new QFormLayout(crtSettings);
    const auto displayInputChoice = [&](QFormLayout* placement, const auto& options, auto selected, const char* key) {
        auto* combo = new QComboBox(placement->parentWidget());
        for (const auto& [value, label] : options) combo->addItem(text(translations, label), static_cast<int>(value));
        combo->setCurrentIndex(combo->findData(static_cast<int>(selected)));
        placement->addRow(text(translations, key), combo);
        return combo;
    };
    const auto crtChoice = [&](const auto& options, auto selected, const char* key) {
        return displayInputChoice(crtForm, options, selected, key);
    };
    const auto* crtmask = crtChoice(maskChoices, profile.crt.mask, "Emulation.Video.Parameter.CrtMask");
    const auto* crtphosphor = crtChoice(phosphorChoices, profile.crt.phosphor, "Emulation.Video.Parameter.VectorPhosphorColor");
    const auto* crtrate = crtChoice(rateChoices, profile.crt.fieldRate, "Emulation.Video.Parameter.CrtFieldRate");
    const auto* crtdecoder = crtChoice(decoderChoices, profile.crt.colorStandards, "Emulation.Video.Parameter.CrtColorStandards");
    auto* crtconnection = crtChoice(connectionChoices, profile.crt.connection, "Emulation.Video.Parameter.CrtConnection");
    auto* crtPalette = crtChoice(palettes, profile.crt.monitorPalette, "Emulation.Video.Parameter.CrtMonitorPalette");
    const auto crtValue = [&](const char* key, int value, int low, int high) {
        auto* spin = new QSpinBox(crtPage);
        spin->setRange(low, high);
        spin->setValue(value);
        crtForm->addRow(text(translations, key), spin);
        return spin;
    };
    auto* crtDegradation = crtValue("Emulation.Video.Parameter.CrtConnectionDegradation", profile.crt.connectionDegradation,
        0, CrtProfile::MaximumConnectionDegradation);
    const auto updateConnection = [crtconnection, crtPalette, crtDegradation] {
        const auto mode = static_cast<CrtConnection>(crtconnection->currentData().toInt());
        crtPalette->setEnabled(mode == CrtConnection::LegacyMonitor);
        crtDegradation->setEnabled(mode >= CrtConnection::Scart && mode <= CrtConnection::AnalogRf);
    };
    QObject::connect(crtconnection, &QComboBox::currentIndexChanged, &dialog, updateConnection);
    updateConnection();
    const auto* crtBeam = crtValue("Emulation.Video.Parameter.VectorBeamWidth", profile.crt.beamWidth,
        ImageAdjustments::Minimum, ImageAdjustments::Maximum);
    const auto* crtCurvature = crtValue("Emulation.Video.Parameter.CrtCurvature", profile.crt.curvature, 0, CrtProfile::MaximumCurvature);
    auto* fixedSettings = new QWidget(crtPage);
    technologyLayout->addWidget(fixedSettings);
    auto* plasmaSettings = new QWidget(crtPage);
    technologyLayout->addWidget(plasmaSettings);
    auto* vectorSettings = new QWidget(crtPage);
    technologyLayout->addWidget(vectorSettings);
    auto* vfdSettings = new QWidget(crtPage);
    technologyLayout->addWidget(vfdSettings);
    auto* ledSettings = new QWidget(crtPage);
    technologyLayout->addWidget(ledSettings);
    auto* dotSettings = new QWidget(crtPage);
    technologyLayout->addWidget(dotSettings);
    auto* segmentSettings = new QWidget(crtPage);
    technologyLayout->addWidget(segmentSettings);
    technologyLayout->addStretch();
    auto* ePaperSettings = new QWidget(crtPage);
    technologyLayout->insertWidget(technologyLayout->count() - 1, ePaperSettings);
    auto* projectionSettings = new QWidget(crtPage);
    technologyLayout->insertWidget(technologyLayout->count() - 1, projectionSettings);
    auto* segmentGrid = new QGridLayout(segmentSettings);
    const auto segmentChoice = [&](QFormLayout* placement, const auto& labels, int selected, const char* key) {
        auto* combo = new QComboBox(placement->parentWidget());
        for (const auto* label : labels) combo->addItem(text(translations, label));
        combo->setCurrentIndex(selected);
        placement->addRow(text(translations, key), combo);
        return combo;
    };
    const auto addTechnologyGroup = [&](QGridLayout* grid, const char* key, int row, int column, int span = 1) {
        auto* group = new QGroupBox(text(translations, key), grid->parentWidget());
        grid->addWidget(group, row, column, 1, span);
        return new QFormLayout(group);
    };
    const auto addTechnologyValue = [&](QFormLayout* form, const char* key, int value, int maximum) {
        auto* field = new QSpinBox(form->parentWidget());
        field->setRange(0, maximum);
        field->setValue(value);
        form->addRow(text(translations, key), field);
        return field;
    };
    auto* ink = new QFormLayout(ePaperSettings);
    const auto* inkMode = segmentChoice(ink, inkModes, int(profile.ePaper.colorMode), "Emulation.Video.Parameter.EPaperColorMode");
    const auto* ePaperDefinition = displayInputChoice(ink, panelDefinitionChoices, profile.ePaper.definition, PanelDefinitionResource);
    auto* projectionForm = new QFormLayout(projectionSettings);
    const auto* projectionTechnology = segmentChoice(projectionForm, projectionTechnologies,
        int(profile.projection.technology), ProjectionTechnologyResource);
    const auto* projectionSurface = segmentChoice(projectionForm, projectionSurfaces,
        int(profile.projection.surface), ProjectionSurfaceResource);
    const auto* projectionAmbientLight = addTechnologyValue(projectionForm, "Emulation.Video.Parameter.ProjectionAmbientLight",
        profile.projection.ambientLight, ProjectionProfile::MaximumAmbientLight);
    auto* segmentForm = addTechnologyGroup(segmentGrid, SegmentDisplayResource, 0, 0);
    const auto* segmentLayoutChoice = segmentChoice(segmentForm, segmentLayoutLabels, int(profile.segmentDisplay.layout), SegmentLayoutResource);
    auto* segmentTechnologyChoice = new QComboBox(segmentForm->parentWidget());
    for (const auto& [value, key] : segmentTechnologies)
        segmentTechnologyChoice->addItem(text(translations, key), int(value));
    segmentTechnologyChoice->setCurrentIndex(segmentTechnologyChoice->findData(int(profile.segmentDisplay.technology)));
    segmentForm->addRow(text(translations, SegmentTechnologyResource), segmentTechnologyChoice);
    auto* segmentColorChoice = new QComboBox(segmentForm->parentWidget());
    segmentForm->addRow(text(translations, SegmentColorResource), segmentColorChoice);
    const auto updateSegmentColors = [=, &translations] {
        const auto previous = segmentColorChoice->count() ? segmentColorChoice->currentData().toInt() : int(profile.segmentDisplay.color);
        segmentColorChoice->clear();
        const auto technology = SegmentDisplayTechnology(segmentTechnologyChoice->currentData().toInt());
        for (const auto value : segmentColors(technology))
            segmentColorChoice->addItem(text(translations, segmentColorLabels[size_t(value)]), int(value));
        segmentColorChoice->setCurrentIndex(std::max(0, segmentColorChoice->findData(previous)));
    };
    QObject::connect(segmentTechnologyChoice, &QComboBox::currentIndexChanged, &dialog, updateSegmentColors);
    updateSegmentColors();
    const auto displayGroups = [](QWidget* parent) {
        auto* layout = new QHBoxLayout(parent);
        layout->setContentsMargins(0, 0, 0, 0);
        return layout;
    };
    const auto displayGroup = [&](QHBoxLayout* layout, const char* key) {
        auto* group = new QGroupBox(text(translations, key), layout->parentWidget());
        layout->addWidget(group, 1);
        return new QFormLayout(group);
    };
    const auto displayChoice = [&](QFormLayout* place, const auto& labels, int selected, const char* key) {
        auto* control = new QComboBox(place->parentWidget());
        for (const auto* label : labels) control->addItem(text(translations, label));
        control->setCurrentIndex(selected);
        place->addRow(text(translations, key), control);
        return control;
    };
    const auto displaySlider = [&](QFormLayout* place, const char* key, int value, int minimum, int maximum) {
        auto* row = new QWidget(place->parentWidget());
        auto* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        auto* control = new QSlider(Qt::Horizontal, row);
        control->setRange(minimum, maximum);
        control->setValue(value);
        if (maximum > FixedPixelProfile::PercentMaximum) {
            constexpr int DurationStepMilliseconds = 5;
            control->setSingleStep(DurationStepMilliseconds);
            QObject::connect(control, &QSlider::actionTriggered, control, [control] {
                const int position = control->sliderPosition();
                control->setSliderPosition(
                    (position + DurationStepMilliseconds / 2) / DurationStepMilliseconds * DurationStepMilliseconds);
            });
        }
        auto* number = new QLabel(QLocale().toString(control->value()), row);
        number->setMinimumWidth(number->fontMetrics().horizontalAdvance(QLocale().toString(maximum)));
        number->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        QObject::connect(control, &QSlider::valueChanged, number, [number](int current) {
            number->setText(QLocale().toString(current));
        });
        rowLayout->addWidget(control, 1);
        rowLayout->addWidget(number);
        auto* label = new QLabel(text(translations, key), place->parentWidget());
        label->setBuddy(control);
        control->setAccessibleName(label->text());
        place->addRow(label, row);
        return control;
    };

    auto* dotLayout = displayGroups(dotSettings);
    auto* dotForm = displayGroup(dotLayout, DotMatrixTechnologyResource);
    const auto* dotPalette = displayChoice(dotForm, dotPalettes, int(profile.dotMatrix.palette), DotPaletteResource);
    const auto* dotDefinition = displayInputChoice(dotForm, matrixDefinitionChoices, profile.dotMatrix.definition, PanelDefinitionResource);
    const auto* dotShape = displayChoice(dotForm, dotShapes, int(profile.dotMatrix.shape), DotShapeResource);
    const auto* dotTechnology = displayChoice(dotForm, dotTechnologies, int(profile.dotMatrix.technology), DotTechnologyResource);
    auto* ledLayout = displayGroups(ledSettings);
    auto* ledForm = displayGroup(ledLayout, LedMatrixTechnologyResource);
    const auto* ledColor = displayChoice(ledForm, ledColors, int(profile.ledMatrix.color), "Emulation.Video.Parameter.LedMatrixColor");
    const auto* ledDefinition = displayInputChoice(ledForm, matrixDefinitionChoices, profile.ledMatrix.definition, PanelDefinitionResource);
    const auto* ledShape = displayChoice(ledForm, ledShapes, int(profile.ledMatrix.shape), "Emulation.Video.Parameter.LedMatrixShape");
    auto* vfdLayout = displayGroups(vfdSettings);
    auto* vfdForm = displayGroup(vfdLayout, VfdTechnologyResource);
    const auto* vfdColor = displayChoice(vfdForm, vfdColors, int(profile.vfd.color), VfdColorResource);
    const auto* vfdDefinition = displayInputChoice(vfdForm, matrixDefinitionChoices, profile.vfd.definition, PanelDefinitionResource);
    const auto* vfdShape = displayChoice(vfdForm, dotShapes, int(profile.vfd.shape), VfdShapeResource);

    auto* vectorForm = new QFormLayout(vectorSettings);
    const auto* vectorPhosphor = displayChoice(vectorForm, vectorColors, int(profile.vector.phosphorColor), VectorPhosphorResource);
    const auto* vectorBeamWidth = displaySlider(vectorForm, VectorBeamWidthResource, profile.vector.beamWidth,
        VectorProfile::MinimumBeamWidth, VectorProfile::MaximumBeamWidth);
    const auto* vectorCurvature = displaySlider(vectorForm, VectorCurvatureResource, profile.vector.curvature,
        0, CrtProfile::MaximumCurvature);
    auto* plasmaForm = new QFormLayout(plasmaSettings);
    auto* plasmaStructure = displayInputChoice(plasmaForm, plasmaCellChoices, profile.plasma.cellStructure,
        PlasmaCellStructureResource);
    auto* fixedForm = new QFormLayout(fixedSettings);
    const auto fixedChoice = [&](const auto& labels, int selected, const char* key) {
        auto* combo = new QComboBox(fixedSettings);
        for (const auto* label : labels) combo->addItem(text(translations, label));
        combo->setCurrentIndex(selected);
        fixedForm->addRow(text(translations, key), combo);
        return combo;
    };
    auto fixedProfiles = profile.fixedPixelProfiles;
    auto fixedDraft = profile.fixedPixel;
    auto lcd = profile.lcd;
    auto ledBacklight = profile.ledBacklight;
    auto* fixedType = fixedChoice(fixedTypes, int(fixedDraft.technology), "Emulation.Video.Parameter.FixedPixelTechnology");
    auto* lcdStructure = fixedChoice(lcdStructures, int(lcd.pixelStructure), LcdPixelStructureResource);
    auto* lcdOrientation = fixedChoice(lcdOrientations, int(lcd.orientation), LcdOrientationResource);
    auto* lcdPanel = fixedChoice(lcdPanelTypes, int(lcd.panelType), LcdPanelTypeResource);
    auto* backlight = addTechnologyValue(fixedForm, "Emulation.Video.Parameter.FixedPixelBacklight",
        lcd.intensity, FixedPixelProfile::PercentMaximum);
    auto* ledBacklightType = fixedChoice(ledBacklightTypes, int(ledBacklight.type), LedBacklightTypeResource);
    auto* oledPanel = fixedChoice(oledPanels, int(profile.oled.panelType), LcdPanelTypeResource);
    auto* oledStructure = fixedChoice(oledStructures, int(profile.oled.pixelStructure), LcdPixelStructureResource);
    const auto currentLcd = [&]() -> LcdProfile& {
        return fixedDraft.technology == FixedPixelTechnology::LedBacklitLcd ? static_cast<LcdProfile&>(ledBacklight) : lcd;
    };
    const auto showOrientation = [&] {
        const auto structure = LcdPixelStructure(lcdStructure->currentIndex());
        fixedForm->setRowVisible(lcdOrientation, fixedDraft.technology != FixedPixelTechnology::Oled &&
            (structure == LcdPixelStructure::Rgb || structure == LcdPixelStructure::Bgr));
    };
    QObject::connect(lcdStructure, &QComboBox::currentIndexChanged, &dialog, showOrientation);
    const auto showFixed = [&] {
        const auto& panel = currentLcd();
        const bool oled = fixedDraft.technology == FixedPixelTechnology::Oled;
        lcdStructure->setCurrentIndex(int(panel.pixelStructure));
        lcdOrientation->setCurrentIndex(int(panel.orientation));
        lcdPanel->setCurrentIndex(int(panel.panelType));
        backlight->setValue(panel.intensity);
        fixedForm->setRowVisible(lcdStructure, !oled);
        fixedForm->setRowVisible(lcdPanel, !oled);
        fixedForm->setRowVisible(backlight, !oled);
        fixedForm->setRowVisible(ledBacklightType, fixedDraft.technology == FixedPixelTechnology::LedBacklitLcd);
        fixedForm->setRowVisible(oledPanel, oled);
        fixedForm->setRowVisible(oledStructure, oled);
        showOrientation();
    };
    const auto captureFixed = [&] {
        if (fixedDraft.technology == FixedPixelTechnology::Oled) return;
        auto& panel = currentLcd();
        panel.pixelStructure = LcdPixelStructure(lcdStructure->currentIndex());
        panel.orientation = LcdPixelOrientation(lcdOrientation->currentIndex());
        panel.panelType = LcdPanelType(lcdPanel->currentIndex());
        panel.intensity = backlight->value();
        ledBacklight.type = LedBacklightType(ledBacklightType->currentIndex());
    };
    QObject::connect(fixedType, &QComboBox::currentIndexChanged, &dialog, [&](int index) {
        captureFixed();
        fixedDraft = fixedProfiles[size_t(index)].value_or(FixedPixelProfile{FixedPixelTechnology(index)});
        showFixed();
    });
    showFixed();
    const auto showTechnology = [=] {
        const auto display = DisplayTechnology(displayType->currentData().toInt());
        crtSettings->setVisible(display == DisplayTechnology::Crt);
        fixedSettings->setVisible(display == DisplayTechnology::FixedPixel);
        plasmaSettings->setVisible(display == DisplayTechnology::Plasma);
        vectorSettings->setVisible(display == DisplayTechnology::Vector);
        vfdSettings->setVisible(display == DisplayTechnology::Vfd);
        ledSettings->setVisible(display == DisplayTechnology::LedMatrix);
        dotSettings->setVisible(display == DisplayTechnology::DotMatrix);
        segmentSettings->setVisible(display == DisplayTechnology::SegmentDisplay);
        ePaperSettings->setVisible(display == DisplayTechnology::EPaper);
        projectionSettings->setVisible(display == DisplayTechnology::Projection);
    };
    QObject::connect(displayType, &QComboBox::currentIndexChanged, &dialog, showTechnology);
    showTechnology();
    using Adjustment = ImageAdjustmentControl;
    auto* panelSettings = new QGroupBox(text(translations, "Emulation.Video.Tab.Signal"), crtPage);
    technologyLayout->insertWidget(technologyLayout->count() - 1, panelSettings);
    auto* panelForm = new QFormLayout(panelSettings);
    const DisplaySignalProfile defaultSignal;
    const auto plasmaDefinitions = orderedChoices(panelDefinitionChoices, plasmaDefinitionChoices());
    auto* panelStandards = displayInputChoice(panelForm, decoderChoices, defaultSignal.colorStandards, "Emulation.Video.Parameter.CrtColorStandards");
    auto* panelRate = displayInputChoice(panelForm, rateChoices, defaultSignal.fieldRate, "Emulation.Video.Parameter.CrtFieldRate");
    auto* panelDefinition = displayInputChoice(panelForm, panelDefinitionChoices, defaultSignal.definition, PanelDefinitionResource);
    auto* panelConnection = displayInputChoice(panelForm, connectionChoices, defaultSignal.connection, "Emulation.Video.Parameter.CrtConnection");
    auto* panelPalette = displayInputChoice(panelForm, palettes, defaultSignal.monitorPalette, "Emulation.Video.Parameter.CrtMonitorPalette");
    auto* panelDegradation = addTechnologyValue(panelForm, "Emulation.Video.Parameter.CrtConnectionDegradation",
        defaultSignal.connectionDegradation, DisplaySignalProfile::MaximumConnectionDegradation);
    DisplaySignalProfile* displayedSignal = nullptr;
    const auto updatePanelFields = [=] {
        DisplaySignalProfile signal;
        signal.connection = CrtConnection(panelConnection->currentData().toInt());
        panelForm->setRowVisible(panelPalette, signal.connection == CrtConnection::LegacyMonitor);
        panelForm->setRowVisible(panelDegradation, signal.canDegrade());
    };
    QObject::connect(panelConnection, &QComboBox::currentIndexChanged, &dialog, updatePanelFields);
    const auto captureAdjustments = [&] {
        if (displayedSignal) {
            displayedSignal->colorStandards = CrtColorStandards(panelStandards->currentData().toInt());
            displayedSignal->fieldRate = CrtFieldRate(panelRate->currentData().toInt());
            displayedSignal->definition = PanelDefinition(panelDefinition->currentData().toInt());
            displayedSignal->connection = CrtConnection(panelConnection->currentData().toInt());
            displayedSignal->monitorPalette = CrtMonitorPalette(panelPalette->currentData().toInt());
            displayedSignal->connectionDegradation = panelDegradation->value();
        }
        adjustmentDraft.setImageAdjustments(adjustmentScreen, {brightness->value(), contrast->value(),
            gamma->currentData().toInt(), saturation->value(), sharpness->value()});
    };
    const auto updateAdjustments = [&] {
        captureAdjustments();
        adjustmentDraft.setDisplayTechnology(DisplayTechnology(displayType->currentData().toInt()));
        adjustmentDraft.fixedPixel.technology = FixedPixelTechnology(fixedType->currentIndex());
        adjustmentDraft.ledMatrix.color = LedMatrixColor(ledColor->currentIndex());
        adjustmentDraft.dotMatrix.palette = DotMatrixPalette(dotPalette->currentIndex());
        adjustmentScreen = adjustmentDraft.screenType();
        updateAspects(adjustmentScreen);
        displayedSignal = adjustmentDraft.panelSignal();
        panelSettings->setVisible(displayedSignal != nullptr);
        if (displayedSignal) {
            panelStandards->setCurrentIndex(panelStandards->findData(int(displayedSignal->colorStandards)));
            panelRate->setCurrentIndex(panelRate->findData(int(displayedSignal->fieldRate)));
            {
                const QSignalBlocker block(panelDefinition);
                panelDefinition->clear();
                const auto addDefinition = [&](const auto& choices) {
                    for (const auto& [definition, key] : choices)
                        panelDefinition->addItem(text(translations, key), int(definition));
                };
                if (adjustmentScreen == ScreenType::Plasma) addDefinition(plasmaDefinitions);
                else addDefinition(panelDefinitionChoices);
                panelDefinition->setCurrentIndex(std::max(0, panelDefinition->findData(int(displayedSignal->definition))));
            }
            {
                const QSignalBlocker block(panelConnection);
                panelConnection->clear();
                for (const auto& [connection, key] : orderedChoices(connectionChoices, displayConnectionChoices(adjustmentScreen)))
                    panelConnection->addItem(text(translations, key), int(connection));
                panelConnection->setCurrentIndex(std::max(0, panelConnection->findData(int(displayedSignal->connection))));
            }
            panelPalette->setCurrentIndex(panelPalette->findData(int(displayedSignal->monitorPalette)));
            panelDegradation->setValue(displayedSignal->connectionDegradation);
            updatePanelFields();
        }
        const auto values = adjustmentDraft.imageAdjustments(adjustmentScreen);
        brightness->setValue(values.brightness);
        contrast->setValue(values.contrast);
        gamma->setCurrentIndex(gamma->findData(values.gamma));
        saturation->setValue(values.saturation);
        sharpness->setValue(values.sharpness);
        form->setRowVisible(sampling, adjustmentDraft.usesHostDisplay());
        imageForm->setRowVisible(dedithering, adjustmentDraft.supportsDedithering());
        imageForm->setRowVisible(deinterlacing, adjustmentDraft.supportsDeinterlacing());
        imageForm->setRowVisible(debanding, adjustmentDraft.usesHostDisplay());
        imageForm->setRowVisible(detailRecovery, adjustmentDraft.usesHostDisplay());
        imageForm->setRowVisible(brightness, adjustmentDraft.showsImageAdjustment(Adjustment::Brightness));
        imageForm->setRowVisible(contrast, adjustmentDraft.showsImageAdjustment(Adjustment::Contrast));
        imageForm->setRowVisible(gamma, adjustmentDraft.showsImageAdjustment(Adjustment::Gamma));
        imageForm->setRowVisible(saturation, adjustmentDraft.showsImageAdjustment(Adjustment::Saturation));
        imageForm->setRowVisible(sharpness, adjustmentDraft.showsImageAdjustment(Adjustment::Sharpness));
    };
    for (const auto* control : std::array<const QComboBox*, 4>{displayType, fixedType, ledColor, dotPalette})
        QObject::connect(control, &QComboBox::currentIndexChanged, &dialog, updateAdjustments);
    updateAdjustments();
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    buttons->button(QDialogButtonBox::Save)->setText(text(translations, SaveKey));
    buttons->button(QDialogButtonBox::Cancel)->setText(text(translations, CloseKey));
    container->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) return std::nullopt;
    auto updated = profile;
    const auto chosenRenderer = static_cast<Renderer>(renderer->currentData().toInt());
    if (chosenRenderer != currentRenderer) updated.renderer = chosenRenderer;
    updated.aspectRatio = aspectDraft;
    updated.sampling = static_cast<Sampling>(sampling->currentData().toInt());
    updated.blackFrameInsertion = blackFrame->isChecked();
    const auto mode = static_cast<Deinterlacing>(deinterlacing->currentData().toInt());
    if (mode != visibleMode) updated.deinterlacing = mode;
    updated.restoration = {dedithering->currentIndex() == level ? profile.restoration.dedithering : dedithering->currentData().toInt(),
        debanding->value(), detailRecovery->value()};
    captureAdjustments();
    updated.adjustments = adjustmentDraft.adjustments;
    updated.screenAdjustments = std::move(adjustmentDraft.screenAdjustments);
    updated.lcdSignal = adjustmentDraft.lcdSignal;
    updated.ledSignal = adjustmentDraft.ledSignal;
    updated.oledSignal = adjustmentDraft.oledSignal;
    updated.plasmaSignal = adjustmentDraft.plasmaSignal;
    updated.projectionSignal = adjustmentDraft.projectionSignal;
    updated.signal = {static_cast<SignalConnection>(connection->currentData().toInt()), connectionIntensity->value(),
        static_cast<SignalStandard>(standard->currentData().toInt()), standardIntensity->value()};
    updated.setDisplayTechnology(DisplayTechnology(displayType->currentData().toInt()));
    updated.ePaper.colorMode = EPaperColorMode(inkMode->currentIndex());
    updated.ePaper.definition = PanelDefinition(ePaperDefinition->currentData().toInt());
    updated.projection.technology = ProjectionTechnology(projectionTechnology->currentIndex());
    updated.projection.surface = ProjectionSurface(projectionSurface->currentIndex());
    updated.projection.ambientLight = projectionAmbientLight->value();
    updated.segmentDisplay.layout = SegmentLayout(segmentLayoutChoice->currentIndex());
    updated.segmentDisplay.technology = SegmentDisplayTechnology(segmentTechnologyChoice->currentData().toInt());
    updated.segmentDisplay.color = SegmentColor(segmentColorChoice->currentData().toInt());
    updated.dotMatrix.palette = DotMatrixPalette(dotPalette->currentIndex());
    updated.dotMatrix.definition = MatrixDefinition(dotDefinition->currentData().toInt());
    updated.dotMatrix.shape = DotMatrixShape(dotShape->currentIndex());
    updated.dotMatrix.technology = DotMatrixTechnology(dotTechnology->currentIndex());
    updated.ledMatrix.color = LedMatrixColor(ledColor->currentIndex());
    updated.ledMatrix.definition = MatrixDefinition(ledDefinition->currentData().toInt());
    updated.ledMatrix.shape = LedMatrixShape(ledShape->currentIndex());

    updated.vfd.color = VfdColor(vfdColor->currentIndex());
    updated.vfd.definition = MatrixDefinition(vfdDefinition->currentData().toInt());
    updated.vfd.shape = DotMatrixShape(vfdShape->currentIndex());
    updated.vector.beamWidth = vectorBeamWidth->value();
    updated.vector.curvature = vectorCurvature->value();
    updated.vector.phosphorColor = VectorPhosphor(vectorPhosphor->currentIndex());
    updated.plasma.cellStructure = PlasmaCellStructure(plasmaStructure->currentData().toInt());
    captureFixed();
    updated.fixedPixel = fixedDraft;
    updated.lcd = lcd;
    updated.ledBacklight = ledBacklight;
    updated.oled.panelType = OledPanelType(oledPanel->currentIndex());
    updated.oled.pixelStructure = OledPixelStructure(oledStructure->currentIndex());
    updated.fixedPixelProfiles = std::move(fixedProfiles);
    if (crtmask->currentIndex() >= 0)
        updated.crt.mask = static_cast<CrtMask>(crtmask->currentData().toInt());
    if (crtphosphor->currentIndex() >= 0)
        updated.crt.phosphor = static_cast<CrtPhosphor>(crtphosphor->currentData().toInt());
    if (crtrate->currentIndex() >= 0)
        updated.crt.fieldRate = static_cast<CrtFieldRate>(crtrate->currentData().toInt());
    if (crtdecoder->currentIndex() >= 0)
        updated.crt.colorStandards = static_cast<CrtColorStandards>(crtdecoder->currentData().toInt());
    if (crtconnection->currentIndex() >= 0)
        updated.crt.connection = static_cast<CrtConnection>(crtconnection->currentData().toInt());
    updated.crt.monitorPalette = static_cast<CrtMonitorPalette>(crtPalette->currentData().toInt());
    updated.crt.connectionDegradation = crtDegradation->value();
    updated.crt.beamWidth = crtBeam->value();
    updated.crt.curvature = crtCurvature->value();
    return updated;
}

class SessionView final : public QWidget {
public:
    using OpenMachineSettings = std::function<void(QWidget*, const std::shared_ptr<Session>&)>;

    SessionView(std::shared_ptr<Session> session, std::shared_ptr<Host> host, Translation& translations,
        WorkspaceState& workspace, const VideoPresentation::ProfileStore& profiles,
        const std::shared_ptr<const ThemeData>& theme, OpenMachineSettings openMachineSettings, QWidget* parent)
        : QWidget(parent), session_(std::move(session)), host_(std::move(host)), translations_(translations),
          workspace_(workspace), theme_(theme), profiles_(profiles),
          profile_(profiles_.load(session_->moduleId(), session_->configuration().id)),
          openMachineSettings_(std::move(openMachineSettings)) {
        auto* root = new QVBoxLayout(this);
        auto* controls = new QHBoxLayout;
        root->addLayout(controls);
        surface_ = new VideoSurface(this);
        surface_->setProfile(profile_);
        surface_->setShaderLoadingObserver([this](bool) {
            updateShaderBadge();
            updateShaderStatusRows();
        });
        surface_->setMouseTracking(true);
        qApp->installEventFilter(this);
        root->addWidget(surface_, 1);
        mediaStrip_ = new QWidget(this);
        mediaLayout_ = new QHBoxLayout(mediaStrip_);
        mediaLayout_->setSpacing(MediaDeviceSpacing);
        root->addWidget(mediaStrip_);
        mediaStrip_->hide();
        layout_ = root;
        const auto create = [&](const char* key, auto action) {
            auto* button = new QPushButton(this);
            buttons_.push_back({button, key});
            connect(button, &QPushButton::clicked, this, std::move(action));
            controls->addWidget(button);
            return button;
        };
        namespace Icons = EmulationCommandIcons;
        constexpr int GroupHorizontalInset = 2, GroupVerticalInset = 2, ButtonSpacing = 2;
        const auto addGroup = [&] {
            auto* card = new Card(theme, this);
            auto* group = new QHBoxLayout(card);
            group->setContentsMargins(GroupHorizontalInset, GroupVerticalInset,
                GroupHorizontalInset, GroupVerticalInset);
            group->setSpacing(ButtonSpacing);
            controls->addWidget(card);
            return group;
        };
        const auto commandButton = [&](QHBoxLayout* group, const char* key, Icons::Glyph glyph,
                std::function<void()> action = {}) {
            auto* button = new QToolButton(group->parentWidget());
            button->setFixedSize(Icons::ButtonSize);
            button->setIconSize(Icons::IconSize);
            button->setAutoRaise(true);
            button->setFocusPolicy(Qt::TabFocus);
            button->setIcon(Icons::icon(*button, glyph, button->palette().color(QPalette::ButtonText).rgb()));
            commandButtons_.push_back({button, key});
            if (action) connect(button, &QToolButton::clicked, this, std::move(action));
            group->addWidget(button);
            return button;
        };
        auto* lifecycle = addGroup();
        power_ = commandButton(lifecycle, PowerKey, Icons::Power, [this] {
            run([this] {
                const auto state = session_->machine().state();
                const bool start = state == MachineState::Created || state == MachineState::Stopped;
                if (!session_->execute(start ? LifecycleCommand::Start : LifecycleCommand::Stop)) return false;
                if (start) {
                    surface_->clear();
                    sendInput();
                }
                return true;
            });
        });
        pause_ = commandButton(lifecycle, PauseResumeKey, Icons::Pause, [this] {
            run([this] {
                return session_->execute(session_->machine().state() == MachineState::Paused
                    ? LifecycleCommand::Resume : LifecycleCommand::Pause);
            });
        });
        reset_ = commandButton(lifecycle, SoftResetKey, Icons::SoftReset, [this] {
            run([this] { return session_->execute(LifecycleCommand::SoftReset); });
        });
        hardReset_ = commandButton(lifecycle, HardResetKey, Icons::HardReset, [this] {
            run([this] { return session_->execute(LifecycleCommand::HardReset); });
        });
        power_->setIcon(Icons::icon(*power_, Icons::Power, Icons::Unpowered));
        pause_->setIcon(Icons::icon(*pause_, Icons::Pause, Icons::PauseColor));
        reset_->setIcon(Icons::icon(*reset_, Icons::SoftReset, Icons::SoftResetColor));
        hardReset_->setIcon(Icons::icon(*hardReset_, Icons::HardReset, Icons::HardResetColor));
        auto* states = addGroup();
        quickSave_ = commandButton(states, SaveStateKey, Icons::Save, [this] { showStateSlots(true); });
        quickLoad_ = commandButton(states, LoadStateKey, Icons::Load, [this] { showStateSlots(false); });
        auto* display = addGroup();
        screenshotButton_ = commandButton(display, ScreenshotKey, Icons::Capture, [this] { screenshot(); });
        auto* fullscreenButton = commandButton(display, FullScreenKey, Icons::Fullscreen, [this] { toggleFullScreen(); });
        save_ = create(SaveStateKey, [this] { stateFile(true); });
        load_ = create(LoadStateKey, [this] { stateFile(false); });
        create(VideoKey, [this] { configureVideo(); });
        controls->addStretch();
        auto* settings = commandButton(addGroup(), SettingsKey, Icons::Settings, [this] { configureMachine(); });
        auto* inputCommands = addGroup();
        mute_ = commandButton(inputCommands, AudioKey, Icons::Audio);
        mute_->setCheckable(true);
        mute_->setIcon(Icons::icon(*mute_, Icons::Audio, Icons::Audible));
        connect(mute_, &QToolButton::toggled, this, [this](bool muted) {
            if (muted) session_->stopAudio();
            mute_->setIcon(Icons::icon(*mute_, muted ? Icons::Muted : Icons::Audio, muted ? Icons::Unpowered : Icons::Audible));
            updateFullscreenMute();
        });
        controllerPointer_ = commandButton(inputCommands, SwitchControllerPointerKey, Icons::Pointer, [this] {
            run([this] { return session_->switchControllerPointer().has_value(); });
        });
        controllerPointer_->hide();
        controllerStatus_ = new QLabel(inputCommands->parentWidget());
        pointerStatus_ = new QLabel(inputCommands->parentWidget());
        for (auto* indicator : {controllerStatus_, pointerStatus_}) {
            indicator->setAlignment(Qt::AlignCenter);
            indicator->setFixedSize(Icons::ButtonSize);
            inputCommands->addWidget(indicator);
        }
        statistics_ = new QCheckBox(this);
        controls->addWidget(statistics_);
        namespace Keys = EmulationKeyboardShortcuts;
        bindCommandShortcut(Keys::Power, power_);
        bindCommandShortcut(Keys::PauseResume, pause_);
        bindCommandShortcut(Keys::SoftReset, reset_);
        bindCommandShortcut(Keys::HardReset, hardReset_);
        bindCommandShortcut(Keys::OpenSettings, settings);
        commandShortcuts_.insert(quickSave_, &Keys::QuickSave);
        commandShortcuts_.insert(quickLoad_, &Keys::QuickLoad);
        bindShortcut(Keys::QuickSave, [this] { quickState(true); });
        bindShortcut(Keys::QuickLoad, [this] { quickState(false); });
        bindCommandShortcut(Keys::Screenshot, screenshotButton_);
        bindCommandShortcut(Keys::ToggleFullscreen, fullscreenButton);
        bindShortcut(Keys::ReleaseMouse, [this] { releasePointer(); });
        bindCommandShortcut(Keys::ToggleMute, mute_);
        bindShortcut(Keys::VolumeDown, [this] { stepVolume(-VolumeStep); });
        bindShortcut(Keys::VolumeUp, [this] { stepVolume(VolumeStep); });
        bindShortcut(Keys::OsdWithPause, [this] { if (fullScreen_) openOsd(true); });
        bindShortcut(Keys::OsdWithoutPause, [this] { if (fullScreen_) openOsd(false); });
        bindShortcut(Keys::OsdStatistics, [this] { if (fullScreen_) toggleFullscreenStatistics(); });
        using Navigation = FullscreenOsd::Navigation;
        bindNavigation(Keys::OsdUp, Navigation::Up);
        bindNavigation(Keys::OsdDown, Navigation::Down);
        bindNavigation(Keys::OsdLeft, Navigation::Left);
        bindNavigation(Keys::OsdRight, Navigation::Right);
        bindNavigation(Keys::OsdConfirm, Navigation::Confirm);
        bindNavigation(Keys::OsdCancel, Navigation::Cancel);
        shortcutSubscription_ = workspace_.subscribeToEmulationShortcuts([this] {
            shortcutRepeat_.clear();
            for (auto& entry : configuredShortcuts_) refreshShortcut(entry);
            physicalShortcutsPrimed_ = false;
            refreshCommandTooltips();
            if (osd_) osd_->setFooterHints(osdFooterHints());
        });
        connect(&timer_, &QTimer::timeout, this, [this] { poll(); });
        connect(&mediaTimer_, &QTimer::timeout, this, [this] {
            refreshMedia();
            refreshQuickStates();
            refreshCommandInputStatus();
        });
        connect(&tapeBlinkTimer_, &QTimer::timeout, this, [this] {
            tapeBlinkVisible_ = !tapeBlinkVisible_;
            refreshMedia();
        });
        subscription_ = translations.subscribe([this] { localize(); });
        localize();
        poll();
        refreshQuickStates();
        timer_.start(FramePollMilliseconds);
        mediaTimer_.start(MediaPollMilliseconds);
        tapeBlinkTimer_.setInterval(TapeBlinkMilliseconds);
        screenshotWorkers_.setMaxThreadCount(1);
    }
    ~SessionView() override {
        screenshotWorkers_.waitForDone();
        if (osd_) osd_->flushPendingSetting();
        surface_->setShaderLoadingObserver({});
        delete shaderBadge_.data();
        timer_.stop();
        mediaTimer_.stop();
        tapeBlinkTimer_.stop();
        session_->stopAudio();
        releaseInput();
        qApp->removeEventFilter(this);
        if (fullScreen_) disconnect(fullScreen_, nullptr, this, nullptr);
    }
    const std::shared_ptr<Session>& session() const noexcept { return session_; }

    void refreshMachineSettings() {
        if (osd_) refreshOsdBlocks();
        refreshMedia(true);
    }

    void refreshMachineInput() {
        physicalShortcutsPrimed_ = false;
        physicalKeys_.clear();
        physicalKeypadKeys_.clear();
        hostKeyboard_.clear();
        heldOsdKeys_.clear();
        shortcutModifiers_.clear();
        shortcutRepeat_.clear();
        if (!pointerCaptureAllowed()) resetPointer();
        const bool captured = input_.pointerCaptured;
        input_ = {};
        input_.pointerCaptured = captured;
        rawControllerInput_.clear();
        sendInput();
    }

    void configureMachine() {
        run([this] {
            openMachineSettings_(fullScreen_ ? static_cast<QWidget*>(fullScreen_) : this, session_);
            return true;
        });
        if (!osd_ && surface_->window()->isActiveWindow()) surface_->setFocus();
    }

    QWidget* createShaderStatusRow(QWidget* parent) {
        auto* row = new QWidget(parent);
        auto* layout = new QVBoxLayout(row);
        constexpr int RowGap = 7, BottomInset = 12, ProgressHeight = 4;
        layout->setContentsMargins(0, 0, 0, BottomInset);
        layout->setSpacing(RowGap);
        auto* label = new QLabel(row);
        label->setWordWrap(true);
        auto font = label->font();
        font.setWeight(QFont::DemiBold);
        label->setFont(font);
        layout->addWidget(label);
        auto* progress = new QProgressBar(row);
        progress->setRange(0, 0);
        progress->setTextVisible(false);
        progress->setFixedHeight(ProgressHeight);
        layout->addWidget(progress);
        shaderStatusRows_.removeIf([](const auto& status) { return status.first.isNull(); });
        shaderStatusRows_.emplaceBack(row, label);
        updateShaderStatusRows();
        return row;
    }

    bool applyVideoProfile(Profile profile) {
        return saveVideoProfile(std::move(profile), false);
    }

    void configureVideo() {
        if (auto updated = editVideoProfile(fullScreen_ ? static_cast<QWidget*>(fullScreen_) : this, translations_, profile_,
            [this](QWidget* parent) { return createShaderStatusRow(parent); }))
            applyVideoProfile(std::move(*updated));
    }
protected:
    bool eventFilter(QObject* object, QEvent* event) override {
        const auto type = event->type();
        if (shaderBadge_ && shaderBadge_->isVisible() && (object == surface_ || object == surface_->window()) &&
            (type == QEvent::Move || type == QEvent::Resize))
            shaderBadge_->centerOn(*surface_);
        if (volumeBadge_ && volumeBadge_->isVisible() && (object == surface_ || object == fullScreen_) &&
            (type == QEvent::Move || type == QEvent::Resize))
            volumeBadge_->reposition(*surface_);
        if (statisticsBadge_ && statisticsBadge_->isVisible() && (object == surface_ || object == fullScreen_) &&
            (type == QEvent::Move || type == QEvent::Resize))
            statisticsBadge_->reposition(*surface_);
        if (muteBadge_ && muteBadge_->isVisible() && (object == surface_ || object == fullScreen_) &&
            (type == QEvent::Move || type == QEvent::Resize))
            muteBadge_->reposition(*surface_);
        const bool buttonEvent = type == QEvent::MouseButtonPress || type == QEvent::MouseButtonRelease
            || type == QEvent::MouseButtonDblClick;
        const bool keyEvent = type == QEvent::KeyPress || type == QEvent::KeyRelease || type == QEvent::ShortcutOverride;
        const bool pointerEvent = buttonEvent || type == QEvent::MouseMove || type == QEvent::Wheel;
        if (pointerEvent || keyEvent) {
            const auto* widget = qobject_cast<QWidget*>(object);
            const bool osdTarget = widget && osd_ && (widget == osd_ || osd_->isAncestorOf(widget));
            const bool inputTarget = widget && (widget == surface_ || surface_->isAncestorOf(widget) || osdTarget);
            if (inputTarget && keyEvent) {
                auto& key = *static_cast<QKeyEvent*>(event);
                const bool routeLiveKey = osd_ && osdPassesInput_ && type != QEvent::ShortcutOverride;
                if (routeLiveKey) hostKeyboard_.update(key, input_);
                const bool consumed = trackPhysicalKey(key);
                if (routeLiveKey) {
                    const auto native = int(EmulationHostKeyboard::keyFor(key));
                    if (type == QEvent::KeyRelease) heldOsdKeys_.remove(native);
                    else if (consumed) heldOsdKeys_.insert(native);
                    sendInput();
                }
                if (consumed) return true;
            }
            if (inputTarget && pointerEvent) {
                const auto& pointer = *static_cast<QSinglePointEvent*>(event);
                const auto buttons = pointer.buttons();
                const auto wheel = type == QEvent::Wheel ? static_cast<QWheelEvent*>(event)->angleDelta() : QPoint{};
                const bool routeLive = osdTarget && osdPassesInput_ && physicalInputActive();
                if (osdPassesInput_ || !input_.pointerCaptured)
                    heldPointerButtons_ &= static_cast<quint32>(buttons.toInt());
                const bool handled = (buttonEvent || type == QEvent::Wheel) && dispatchPhysicalShortcuts(buttons, wheel);
                if (routeLive && osd_ &&
                    routeLiveOsdPointer(type, pointer.globalPosition(), buttons, handled ? QPoint{} : wheel))
                    return true;
                if (handled) return true;
            }
        }
        if (object != surface_) return QWidget::eventFilter(object, event);
        if (surface_->isShaderLoading() && (type == QEvent::Show || type == QEvent::Hide))
            updateShaderBadge();
        if (type == QEvent::FocusOut || type == QEvent::Hide) {
            if (type == QEvent::FocusOut && osdPassesInput_) return false;
            physicalShortcutsPrimed_ = false;
            physicalKeys_.clear();
            physicalKeypadKeys_.clear();
            shortcutModifiers_.clear();
            releaseInput();
            return false;
        }
        if (type == QEvent::KeyPress || type == QEvent::KeyRelease) {
            const auto* key = static_cast<QKeyEvent*>(event);
            if (fullScreen_ && key->key() == Qt::Key_Escape) return false;
            if (type == QEvent::KeyRelease)
                heldOsdKeys_.remove(int(EmulationHostKeyboard::keyFor(*key)));
            if (hostKeyboard_.update(*key, input_)) sendInput();
            return true;
        }
        if (type == QEvent::ContextMenu && input_.pointerCaptured) return true;
        if (type == QEvent::UngrabMouse && input_.pointerCaptured) {
            releasePointer();
            return false;
        }
        if (type == QEvent::MouseButtonPress || type == QEvent::MouseButtonDblClick) {
            surface_->setFocus();
            if (!input_.pointerCaptured) run([this] { capturePointer(); return true; });
        }
        if (type == QEvent::MouseMove || type == QEvent::MouseButtonPress ||
            type == QEvent::MouseButtonDblClick || type == QEvent::MouseButtonRelease) {
            if (!input_.pointerCaptured) return false;
            const auto* mouse = static_cast<QMouseEvent*>(event);
            const auto physicalButtons = static_cast<quint32>(mouse->buttons().toInt());
            heldPointerButtons_ &= physicalButtons;
            const auto buttons = physicalButtons & ~heldPointerButtons_;
            const auto center = surface_->mapToGlobal(surface_->rect().center());
            if (type == QEvent::MouseMove) {
                const auto delta = QCursor::pos() - center;
                input_.pointerX = qRound(delta.x() * surface_->devicePixelRatioF());
                input_.pointerY = qRound(delta.y() * surface_->devicePixelRatioF());
                if (delta.isNull() && buttons == input_.pointerButtons) return true;
                if (!delta.isNull()) QCursor::setPos(center);
            }
            input_.pointerButtons = buttons;
            sendInput();
            input_.pointerX = input_.pointerY = 0;
            return true;
        }
        if (type == QEvent::Wheel && input_.pointerCaptured) {
            const auto delta = static_cast<QWheelEvent*>(event)->angleDelta();
            input_.wheel = delta.y();
            input_.horizontalWheel = delta.x();
            sendInput();
            input_.wheel = input_.horizontalWheel = 0;
            return true;
        }
        if (type == QEvent::Wheel && !osd_ && physicalInputActive() &&
            dynamic_cast<const IMachineKeyboardBindings*>(&session_->machine())) {
            sendInput(static_cast<QWheelEvent*>(event)->angleDelta());
        }
        return QWidget::eventFilter(object, event);
    }
private:
    void updateShaderStatusRows() {
        const bool visible = surface_->isShaderLoading() && !fullScreen_;
        for (const auto& [row, label] : shaderStatusRows_) {
            if (!row || !label) continue;
            label->setText(text(translations_, VideoShaderLoadingKey));
            row->setVisible(visible);
        }
    }

    QString formatPercent(int percent) const {
        return text(translations_, DisplayConstants::PercentLabel)
            .replace(DisplayConstants::Argument, QLocale().toString(percent));
    }

    void updateFullscreenMute() {
        if (!fullScreen_) return;
        try {
            const bool visible = mute_->isChecked() || session_->audioVolumeIsZero();
            if (visible && !muteBadge_) {
                namespace Icons = EmulationCommandIcons;
                muteBadge_ = new VideoPresentation::FullscreenMuteBadge(*fullScreen_,
                    Icons::icon(*mute_, Icons::Muted, Icons::Unpowered));
                muteBadge_->setAccessibleName(text(translations_, MuteIndicatorKey));
            }
            if (!muteBadge_ || visible == muteBadge_->isVisible()) return;
            if (visible) {
                muteBadge_->reposition(*surface_);
                muteBadge_->show();
                muteBadge_->raise();
            } else {
                muteBadge_->hide();
            }
        } catch (...) {
            if (muteBadge_) muteBadge_->hide();
            surface_->setIndicator(text(translations_, ErrorKey));
        }
    }

    void setSessionVolume(int percent) {
        session_->setVolumePercent(percent);
        if (percent > MinimumVolume && mute_->isChecked()) mute_->setChecked(false);
        else updateFullscreenMute();
    }

    void stepVolume(int direction) {
        try {
            const int percent = std::clamp(session_->volumePercent() + direction, MinimumVolume, MaximumVolume);
            setSessionVolume(percent);
            if (!fullScreen_) return;
            if (!volumeBadge_) {
                const auto icon = EmulationCommandIcons::icon(*mute_, EmulationCommandIcons::Audio, EmulationCommandIcons::Audible);
                volumeBadge_ = new VideoPresentation::FullscreenVolumeBadge(*fullScreen_, icon);
                volumeBadge_->setTransparency(workspace_.fullscreenOsdTransparency());
            }
            volumeBadge_->showVolume(*surface_, text(translations_, VolumeLabelKey), formatPercent(percent), percent);
        } catch (...) {
            surface_->setIndicator(text(translations_, ErrorKey));
        }
    }

    QIcon osdCommandIcon(EmulationCommandIcons::Glyph glyph) const {
        return EmulationCommandIcons::icon(*this, glyph, qRgb(255, 255, 255));
    }

    FullscreenOsd::Entry osdVolumeSetting() {
        FullscreenOsd::Entry entry{text(translations_, VolumeLabelKey)};
        entry.icon = osdCommandIcon(EmulationCommandIcons::Audio);
        entry.setting = FullscreenOsd::Setting{
            [this] { return session_->volumePercent(); },
            [this](int percent) { return formatPercent(percent); },
            [this](int percent) { setSessionVolume(percent); return true; },
            MinimumVolume, MaximumVolume,
            true, [this](int percent) { setSessionVolume(percent); }};
        return entry;
    }

    void previewOsdTransparency(int percent) {
        if (osd_) osd_->setTransparency(percent);
        if (volumeBadge_) volumeBadge_->setTransparency(percent);
        if (statisticsBadge_) statisticsBadge_->setTransparency(percent);
        if (fullScreen_ && shaderBadge_) shaderBadge_->setTransparency(percent);
        surface_->setOsdTransparency(fullScreen_ ? percent : DisplayConstants::MinimumOsdTransparency);
    }

    FullscreenOsd::Entry osdTransparencySetting() {
        FullscreenOsd::Entry entry{text(translations_, DisplayConstants::OsdTransparencyLabel)};
        constexpr auto OsdSection = "OSD";
        entry.section = QString::fromLatin1(OsdSection);
        entry.setting = FullscreenOsd::Setting{
            [this] { return workspace_.fullscreenOsdTransparency(); },
            [this](int percent) { return formatPercent(percent); },
            [this](int percent) {
                try {
                    workspace_.setFullscreenOsdTransparency(percent);
                    return true;
                } catch (...) {
                    surface_->setIndicator(text(translations_, ErrorKeys::CannotSavePreferences));
                    return false;
                }
            },
            DisplayConstants::MinimumOsdTransparency, DisplayConstants::MaximumOsdTransparency,
            true, [this](int percent) { previewOsdTransparency(percent); }};
        return entry;
    }

    void updateShaderBadge() {
        const bool loading = surface_->isShaderLoading() && surface_->isVisible();
        const auto caption = loading ? text(translations_, VideoShaderLoadingKey) : QString{};
        if (osd_) osd_->setShaderLoading(loading, caption);
        if (osd_ || !loading) {
            if (shaderBadge_) shaderBadge_->hide();
            return;
        }
        auto* owner = surface_->window();
        if (!shaderBadge_ || shaderBadge_->parentWidget() != owner) {
            delete shaderBadge_.data();
            constexpr auto flags = Qt::Tool | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint |
                Qt::WindowDoesNotAcceptFocus | Qt::WindowTransparentForInput;
            shaderBadge_ = new VideoPresentation::ShaderLoadingBadge(fullScreen_ != nullptr, owner, flags);
        }
        if (fullScreen_) shaderBadge_->setTransparency(workspace_.fullscreenOsdTransparency());
        shaderBadge_->setCaption(caption);
        shaderBadge_->centerOn(*surface_);
        shaderBadge_->show();
        shaderBadge_->raise();
    }

    void localize() {
        if (surface_->isShaderLoading()) updateShaderBadge();
        updateShaderStatusRows();
        if (muteBadge_) muteBadge_->setAccessibleName(text(translations_, MuteIndicatorKey));
        for (const auto& [button, key] : buttons_) button->setText(text(translations_, key));
        for (const auto& [button, key] : commandButtons_) button->setAccessibleName(text(translations_, key));
        controllerStatus_->setAccessibleName(text(translations_, ControllerStatusKey));
        pointerStatus_->setAccessibleName(text(translations_, PointerStatusKey));
        refreshCommandTooltips();
        updateCommandInputStatus(commandInputStatus_.second, true);
        statistics_->setText(text(translations_, StatisticsKey));
        refreshMedia();
    }

    void refreshCommandTooltips() {
        namespace Keys = EmulationKeyboardShortcuts;
        const auto& configured = workspace_.emulationShortcuts();
        const auto tooltip = [&](QString label, const Keys::Binding* binding) {
            if (!binding) return label;
            const auto value = configured.value(QLatin1String(binding->id));
            const auto shortcut = value.isUndefined()
                ? QKeySequence(binding->defaultKey).toString(QKeySequence::NativeText)
                : value.toString();
            if (!shortcut.isEmpty()) label += QLatin1Char('\n') + shortcut;
            return label;
        };
        for (const auto& [button, key] : commandButtons_)
            button->setToolTip(tooltip(text(translations_, key), commandShortcuts_.value(button)));
        pointerStatus_->setToolTip(tooltip(text(translations_, ReleasePointerKey), &Keys::ReleaseMouse));
    }
    void updateCommandInputStatus(bool controllerAvailable, bool force = false) {
        const auto current = std::pair{input_.pointerCaptured, controllerAvailable};
        if (!force && current == commandInputStatus_) return;
        namespace Icons = EmulationCommandIcons;
        const auto update = [this](QLabel& indicator, Icons::Glyph glyph, bool active) {
            const auto icon = Icons::icon(indicator, glyph, active ? Icons::Powered : Icons::Inactive);
            indicator.setPixmap(icon.pixmap(Icons::IconSize, indicator.devicePixelRatioF()));
            indicator.setAccessibleDescription(text(translations_, active ? EnabledStatusKey : DisabledStatusKey));
        };
        if (force || current.first != commandInputStatus_.first) update(*pointerStatus_, Icons::Pointer, current.first);
        if (force || current.second != commandInputStatus_.second) update(*controllerStatus_, Icons::Controller, current.second);
        commandInputStatus_ = current;
    }

    void refreshCommandInputStatus() {
        if (!isVisible()) return;
        bool available = false;
        try {
            if (pause_->isEnabled() && physicalInputActive()) available = !gamepadSamples_.isEmpty();
            else {
                if (!gamepadSource_) gamepadSource_ = std::make_unique<Platform::GamepadSource>();
                available = !gamepadSource_->read().isEmpty();
            }
        } catch (...) {
            // An unavailable input backend leaves the indicator inactive.
        }
        updateCommandInputStatus(available);
    }

    void refreshControllerPointer(bool powered) {
        const auto mode = session_->controllerPointerMode();
        controllerPointer_->setEnabled(powered && mode.has_value());
        if (mode == controllerPointerMode_) return;
        controllerPointerMode_ = mode;
        controllerPointer_->setVisible(mode.has_value());
        if (!mode) return;
        namespace Icons = EmulationCommandIcons;
        controllerPointer_->setIcon(Icons::icon(*controllerPointer_,
            *mode ? Icons::Pointer : Icons::Controller, Icons::Powered));
    }

    struct ConfiguredShortcut {
        const EmulationKeyboardShortcuts::Binding* binding;
        QShortcut* keyboard;
        std::function<void()> action;
        EmulationPhysicalShortcuts::Chord primary;
        EmulationPhysicalShortcuts::Chord alternative;
        bool held = false;
        bool midi = false;
    };

    void refreshShortcut(ConfiguredShortcut& entry) {
        namespace Keys = EmulationKeyboardShortcuts;
        namespace Physical = EmulationPhysicalShortcuts;
        const auto& configured = workspace_.emulationShortcuts();
        const auto id = QLatin1String(entry.binding->id);
        entry.keyboard->setKeys(Keys::sequences(configured, *entry.binding));
        const auto stored = configured.value(id);
        const bool direction = Keys::isOsdDirection(*entry.binding);
        const bool trackDefault = direction || Keys::isOsdToggle(*entry.binding) ||
            Keys::isVolumeShortcut(*entry.binding) || entry.binding == &Keys::ToggleMute;
        entry.primary = Physical::chord(stored.isUndefined() && trackDefault
            ? QKeySequence(entry.binding->defaultKey).toString(QKeySequence::PortableText) : stored.toString(), trackDefault);
        if (entry.binding->hasAlternative)
            entry.alternative = Physical::chord(
                configured.value(id + QLatin1String(Keys::AlternativeSuffix)).toString(), direction);
        entry.held = false;
        entry.midi = Physical::isOsdShortcut(id.toString()) &&
            (Physical::hasMidiSource(entry.primary) || Physical::hasMidiSource(entry.alternative));
    }

    QShortcut* bindShortcut(const EmulationKeyboardShortcuts::Binding& binding, std::function<void()> action) {
        auto* shortcut = new QShortcut(surface_);
        configuredShortcuts_.append(ConfiguredShortcut{&binding, shortcut, action});
        refreshShortcut(configuredShortcuts_.last());
        shortcut->setAutoRepeat(false);
        connect(shortcut, &QShortcut::activated, this, [this, shortcut, action = std::move(action)] {
            if (osd_ ? shortcut->parent() == osd_ : surface_->hasFocus()) action();
        });
        return shortcut;
    }

    void bindCommandShortcut(const EmulationKeyboardShortcuts::Binding& binding, QToolButton* button) {
        commandShortcuts_.insert(button, &binding);
        bindShortcut(binding, [button] { button->click(); });
    }

    bool physicalInputActive() const {
        const auto* modal = QApplication::activeModalWidget();
        const bool inputWindowActive = osd_ ? osd_->isActiveWindow()
            : surface_->hasFocus() && surface_->window()->isActiveWindow();
        return inputWindowActive
            && !QApplication::activePopupWidget() && (!modal || modal == fullScreen_);
    }

    bool shortcutAvailable(const ConfiguredShortcut& entry) const {
        using namespace EmulationKeyboardShortcuts;
        const auto* binding = entry.binding;
        const bool fullscreenOnly = isOsdToggle(*binding);
        return entry.keyboard->isEnabled() && (!fullscreenOnly || fullScreen_) &&
            (!osd_ || EmulationPhysicalShortcuts::isOsdShortcut(QString::fromLatin1(binding->id)));
    }

    QList<Platform::GamepadState> shortcutControllers() const {
        if (midiSamples_.isEmpty()) return gamepadSamples_;
        auto controllers = gamepadSamples_;
        controllers.append(midiSamples_);
        return controllers;
    }

    bool trackPhysicalKey(QKeyEvent& event) {
        if (!physicalInputActive() || event.isAutoRepeat()) return false;
        namespace Physical = EmulationPhysicalShortcuts;
        const auto key = event.key();
        if (event.type() != QEvent::ShortcutOverride)
            shortcutModifiers_.observe(event, EmulationHostKeyboard::keyFor(event));
        const auto modifiers = shortcutModifiers_.normalize(event.modifiers());
        const bool ordinary = !Physical::modifierKey(key);
        if (event.type() == QEvent::ShortcutOverride) {
            if (!ordinary) return false;
            auto keys = physicalKeys_;
            keys.insert(key);
            const auto controllers = shortcutControllers();
            const Physical::Input prospective{QApplication::mouseButtons(), {}, controllers, modifiers, std::move(keys)};
            if (modifiers == event.modifiers() && !std::ranges::any_of(configuredShortcuts_, [&](const auto& entry) {
                return shortcutAvailable(entry) &&
                    (Physical::pressed(entry.primary, prospective) || Physical::pressed(entry.alternative, prospective));
            })) return false;
            event.accept();
            return true;
        }
        if (ordinary && event.type() == QEvent::KeyPress) {
            physicalKeys_.insert(key);
            if (modifiers.testFlag(Qt::KeypadModifier)) physicalKeypadKeys_.insert(key);
        } else if (ordinary) {
            physicalKeys_.remove(key);
            physicalKeypadKeys_.remove(key);
        }
        return dispatchPhysicalShortcuts(QApplication::mouseButtons());
    }

    quint32 liveOsdButtons(Qt::MouseButtons buttons, bool overControls) {
        const auto physical = static_cast<quint32>(buttons.toInt());
        heldPointerButtons_ &= physical;
        if (overControls) heldPointerButtons_ |= physical;
        return physical & ~heldPointerButtons_;
    }

    bool routeLiveOsdPointer(QEvent::Type type, const QPointF& position, Qt::MouseButtons buttons, const QPoint& wheel) {
        const auto delta = liveOsdPointerPosition_ ? position - *liveOsdPointerPosition_ : QPointF{};
        liveOsdPointerPosition_ = position;
        const bool overControls = osd_->usesPointerAt(position.toPoint());
        const auto forwarded = liveOsdButtons(buttons, overControls);
        if (overControls) {
            if (input_.pointerButtons != forwarded) {
                input_.pointerButtons = forwarded;
                sendInput();
            }
            return false;
        }
        const auto scale = surface_->devicePixelRatioF();
        if (type == QEvent::MouseMove) {
            input_.pointerX = qRound(delta.x() * scale);
            input_.pointerY = qRound(delta.y() * scale);
        }
        input_.wheel = wheel.y();
        input_.horizontalWheel = wheel.x();
        if (input_.pointerX || input_.pointerY || !wheel.isNull() || input_.pointerButtons != forwarded) {
            input_.pointerButtons = forwarded;
            sendInput();
            input_.pointerX = input_.pointerY = input_.wheel = input_.horizontalWheel = 0;
        }
        return true;
    }

    void consumeShortcutKeys(const EmulationPhysicalShortcuts::Chord& chord) {
        for (const auto key : chord.keys) {
            const QKeyEvent event(QEvent::KeyPress, key, chord.modifiers);
            heldOsdKeys_.insert(int(EmulationHostKeyboard::keyFor(event)));
        }
        using Key = EmulationKey;
        constexpr std::array modifierKeys{
            std::pair{Qt::ShiftModifier, std::pair{Key::LeftShift, Key::RightShift}},
            std::pair{Qt::ControlModifier, std::pair{Key::LeftControl, Key::RightControl}},
            std::pair{Qt::AltModifier, std::pair{Key::LeftAlt, Key::RightAlt}},
        };
        for (const auto& [modifier, keys] : modifierKeys) {
            if (!chord.modifiers.testFlag(modifier)) continue;
            for (const auto key : {keys.first, keys.second})
                if (input_.keys.contains(int(key))) heldOsdKeys_.insert(int(key));
        }
    }

    void consumeOsdSources(const EmulationPhysicalShortcuts::Chord& chord) {
        namespace Physical = EmulationPhysicalShortcuts;
        for (const auto& source : chord.sources) {
            if (source.startsWith(QLatin1String(Physical::MousePrefix), Qt::CaseInsensitive)) {
                const auto control = QStringView(source).sliced(QLatin1String(Physical::MousePrefix).size());
                for (const auto& [name, button] : Physical::MouseButtons)
                    if (control.compare(name, Qt::CaseInsensitive) == 0) heldPointerButtons_ |= quint32(button);
                continue;
            }
            const auto pad = std::ranges::max_element(gamepadSamples_, {}, [&](const auto& candidate) {
                return Physical::controllerPressure(source, candidate);
            });
            if (pad != gamepadSamples_.end() && Physical::controllerPressure(source, *pad) > Physical::PressedThreshold)
                Physical::controllerPressure(source, *pad, &heldGamepads_[pad->id]);
        }
    }

    bool dispatchPhysicalShortcuts(Qt::MouseButtons buttons, const QPoint& wheel = {}) {
        if (dispatchingPhysicalShortcut_ || !physicalInputActive()) return false;
        namespace Physical = EmulationPhysicalShortcuts;
        const auto powered = session_->machine().state();
        if (powered != MachineState::Running && powered != MachineState::Paused && !osd_ && !fullScreen_) return false;
        auto modifiers = shortcutModifiers_.normalize(QApplication::keyboardModifiers());
        if (!physicalKeypadKeys_.isEmpty()) modifiers |= Qt::KeypadModifier;
        const auto controllers = shortcutControllers();
        const Physical::Input current{buttons, wheel, controllers, modifiers, physicalKeys_};
        const Physical::Input held{buttons, {}, controllers, modifiers, physicalKeys_};
        std::function<void()> action;
        const ConfiguredShortcut* repeating = nullptr;
        const bool editing = osd_ && osd_->isEditing();
        const auto previouslyFilteredKeys = heldOsdKeys_.size();
        for (auto& entry : configuredShortcuts_) {
            const bool liveNavigation = osdPassesInput_ && Physical::isOsdShortcut(QString::fromLatin1(entry.binding->id));
            const bool primary = Physical::pressed(entry.primary, current, liveNavigation);
            const bool alternative = Physical::pressed(entry.alternative, current, liveNavigation);
            const bool pressed = primary || alternative;
            const bool available = shortcutAvailable(entry);
            const bool triggered = physicalShortcutsPrimed_ && available && pressed && !entry.held;
            const bool volume = EmulationKeyboardShortcuts::isVolumeShortcut(*entry.binding);
            const bool osdToggle = EmulationKeyboardShortcuts::isOsdToggle(*entry.binding);
            if (available && pressed && (liveNavigation || (triggered && (volume || osdToggle)))) {
                if (primary) consumeShortcutKeys(entry.primary);
                if (alternative) consumeShortcutKeys(entry.alternative);
            }
            if (available && pressed && (liveNavigation || osdToggle)) {
                if (primary) consumeOsdSources(entry.primary);
                if (alternative) consumeOsdSources(entry.alternative);
            }
            if (!action && triggered) action = entry.action;
            entry.held = Physical::pressed(entry.primary, held, liveNavigation) ||
                Physical::pressed(entry.alternative, held, liveNavigation);
            const bool repeatable = volume ||
                (editing && EmulationKeyboardShortcuts::isOsdDirection(*entry.binding));
            if (!repeating && available && entry.held && repeatable)
                repeating = &entry;
        }
        if (shortcutRepeat_.due(repeating ? repeating->binding : nullptr, Physical::ShortcutRepeat::Clock::now()) && !action)
            action = repeating->action;
        physicalShortcutsPrimed_ = true;
        if (!action) return false;
        const QScopedValueRollback dispatching(dispatchingPhysicalShortcut_, true);
        if (previouslyFilteredKeys != heldOsdKeys_.size()) sendInput();
        action();
        return true;
    }

    void bindNavigation(const EmulationKeyboardShortcuts::Binding& binding, FullscreenOsd::Navigation direction) {
        auto* shortcut = bindShortcut(binding, [this, direction] {
            if (osd_) osd_->navigate(direction);
        });
        shortcut->setEnabled(false);
        osdNavigationShortcuts_.append(shortcut);
    }

    Qt::MouseButtons machineBindingMouseButtons() const {
        if (osd_ || input_.pointerCaptured) return Qt::MouseButtons::fromInt(static_cast<int>(input_.pointerButtons));
        return physicalInputActive() ? QApplication::mouseButtons() &
            ~Qt::MouseButtons::fromInt(static_cast<int>(heldPointerButtons_)) : Qt::MouseButtons{};
    }

    void sendInput(const QPoint& uncapturedWheel = {}) {
        try {
            auto& machine = session_->machine();
            const auto* keyboard = dynamic_cast<const IMachineKeyboardBindings*>(&machine);
            bindingMouseButtons_ = {};
            bindingWheelPending_ = false;
            if (heldOsdKeys_.isEmpty() && !keyboard) machine.setInput(input_);
            else {
                auto filtered = input_;
                filtered.keys.subtract(heldOsdKeys_);
                for (const auto key : heldOsdKeys_) filtered.hostCharacters.remove(key);
                if (keyboard) {
                    namespace Physical = EmulationPhysicalShortcuts;
                    const auto active = physicalInputActive();
                    const auto reserved = [&](const QString& source) {
                        if (!active) return true;
                        const auto chord = Physical::chord(source, true);
                        return std::ranges::any_of(configuredShortcuts_, [&](const auto& entry) {
                            return !Physical::isOsdShortcut(QString::fromLatin1(entry.binding->id)) &&
                                (Physical::conflicts(chord, entry.primary) || Physical::conflicts(chord, entry.alternative));
                        });
                    };
                    bindingMouseButtons_ = machineBindingMouseButtons();
                    const auto wheel = input_.pointerCaptured || osd_ ? QPoint(input_.horizontalWheel, input_.wheel)
                                                                      : uncapturedWheel;
                    bindingWheelPending_ = !wheel.isNull();
                    EmulationHostKeyboard::applyMachineBindings(filtered, rawControllerInput_,
                        bindingMouseButtons_, wheel, keyboard->keyboardBindings(), reserved);
                }
                machine.setInput(filtered);
            }
            if (auto* extension = dynamic_cast<IMachineRawControllerInput*>(&machine))
                extension->setRawControllerInput(rawControllerInput_);
        } catch (...) { surface_->setIndicator(text(translations_, ErrorKey)); }
    }
    bool pointerCaptureAllowed() const {
        const auto& machine = session_->machine();
        const auto state = machine.state();
        if (state != MachineState::Running && state != MachineState::Paused) return false;
        const auto* policy = dynamic_cast<const IMachinePointerCapture*>(&machine);
        return policy && policy->supportsPointerCapture() && policy->capturePointerOnClick();
    }

    void capturePointer() {
        if (osd_ || !surface_->isVisible() || !surface_->window()->isActiveWindow() || !pointerCaptureAllowed()) return;
        surface_->grabMouse(QCursor(Qt::BlankCursor));
        if (QWidget::mouseGrabber() != surface_) return;
        input_.pointerCaptured = true;
        QCursor::setPos(surface_->mapToGlobal(surface_->rect().center()));
        sendInput();
    }

    void resetPointer() {
        if (!input_.pointerCaptured) return;
        input_.pointerCaptured = false;
        input_.pointerX = input_.pointerY = input_.wheel = input_.horizontalWheel = 0;
        input_.pointerButtons = heldPointerButtons_ = 0;
        if (QWidget::mouseGrabber() == surface_) surface_->releaseMouse();
    }

    void releasePointer() {
        if (!input_.pointerCaptured) return;
        resetPointer();
        sendInput();
    }

    void releaseInput() {
        if (gamepadSource_) gamepadSource_->stopRumble();
        hostKeyboard_.clear();
        heldOsdKeys_.clear();
        shortcutModifiers_.clear();
        shortcutRepeat_.clear();
        if (!input_.pointerCaptured && input_.keys.isEmpty() && input_.pointerButtons == 0 &&
            input_.controllers.isEmpty() && rawControllerInput_.isEmpty() &&
            !bindingMouseButtons_ && !bindingWheelPending_) return;
        resetPointer();
        input_ = {};
        rawControllerInput_.clear();
        sendInput();
    }
    static constexpr float GamepadReleaseThreshold = 0.5f;

    void pollControllers() {
        const bool midiEnabled = fullScreen_ && std::ranges::any_of(configuredShortcuts_, &ConfiguredShortcut::midi);
        if (!midiEnabled) {
            midiInput_.reset();
            midiSamples_.clear();
        } else if (!midiInput_) {
            midiInput_.emplace();
        }
        if (!physicalInputActive()) {
            physicalShortcutsPrimed_ = false;
            physicalKeys_.clear();
            physicalKeypadKeys_.clear();
            shortcutModifiers_.clear();
            shortcutRepeat_.clear();
            gamepadSamples_.clear();
            midiSamples_.clear();
            liveOsdPointerPosition_.reset();
            if (!input_.controllers.isEmpty() || !rawControllerInput_.isEmpty() || bindingMouseButtons_ || bindingWheelPending_ ||
                (osdPassesInput_ && (input_.pointerButtons || !input_.keys.isEmpty()))) {
                input_.controllers.clear();
                rawControllerInput_.clear();
                if (osdPassesInput_) {
                    hostKeyboard_.clear();
                    heldOsdKeys_.clear();
                    input_.keys.clear();
                    input_.hostCharacters.clear();
                    input_.pointerButtons = 0;
                }
                sendInput();
            }
            return;
        }

        if (!gamepadSource_) gamepadSource_ = std::make_unique<Platform::GamepadSource>();
        gamepadSamples_ = gamepadSource_->read();
        if (midiInput_) midiSamples_ = midiInput_->read();
        const auto mouseButtons = QApplication::mouseButtons();
        if (osdPassesInput_ || !input_.pointerCaptured)
            heldPointerButtons_ &= static_cast<quint32>(mouseButtons.toInt());
        dispatchPhysicalShortcuts(mouseButtons);
        const auto forwardedButtons = osd_ && osdPassesInput_
            ? liveOsdButtons(mouseButtons, osd_->usesPointerAt(QCursor::pos())) : input_.pointerButtons;
        QSet<quint64> connected;
        QList<InputSnapshot::Controller> forwarded;
        QList<RawControllerState> raw;
        for (const auto& current : gamepadSamples_) {
            connected.insert(current.id);
            if (osd_ && !osdPassesInput_) continue;

            auto filtered = current;
            if (auto held = heldGamepads_.find(current.id); held != heldGamepads_.end()) {
                held->buttons &= current.buttons;
                filtered.buttons &= ~held->buttons;
                for (qsizetype i = 0; i < std::min(held->axes.size(), current.axes.size()); ++i) {
                    const float axis = current.axes[i];
                    if (std::abs(axis) > GamepadReleaseThreshold && axis * held->axes[i] > 0) filtered.axes[i] = 0;
                    else held->axes[i] = 0;
                }
                for (auto control = held->controls.begin(); control != held->controls.end();) {
                    namespace Raw = Platform::RawGamepad;
                    const auto currentValue = current.controls.constFind(control.key());
                    const auto direction = Raw::direction(control.key(), *control);
                    if (currentValue != current.controls.cend() && direction != 0 &&
                        direction == Raw::direction(control.key(), *currentValue)) {
                        filtered.controls.insert(control.key(), Raw::isAxis(control.key()) ? Raw::AxisCenter : 0.0f);
                        ++control;
                    } else {
                        control = held->controls.erase(control);
                    }
                }
            }
            forwarded.append(InputSnapshot::Controller{filtered.buttons, std::move(filtered.axes)});
            raw.append(RawControllerState{std::move(filtered.deviceId), std::move(filtered.controls)});
        }
        for (auto held = heldGamepads_.begin(); held != heldGamepads_.end();) {
            if (connected.contains(held.key())) ++held;
            else held = heldGamepads_.erase(held);
        }
        const bool bindingButtonsChanged = !osd_ && !input_.pointerCaptured &&
            dynamic_cast<const IMachineKeyboardBindings*>(&session_->machine()) &&
            machineBindingMouseButtons() != bindingMouseButtons_;
        if (input_.controllers != forwarded || rawControllerInput_ != raw || input_.pointerButtons != forwardedButtons ||
            bindingButtonsChanged || bindingWheelPending_) {
            input_.controllers = std::move(forwarded);
            rawControllerInput_ = std::move(raw);
            input_.pointerButtons = forwardedButtons;
            sendInput();
        }
    }

    template<class Operation>
    void run(Operation operation, const char* fallbackError = ErrorKey) {
        const auto previous = session_->runtimeRevision();
        try {
            surface_->setIndicator(operation() ? QString{} : text(translations_, fallbackError));
        } catch (const EmulationMessageException& failure) {
            const auto& configuration = session_->configuration();
            auto* owner = fullScreen_ ? static_cast<QWidget*>(fullScreen_) : this;
            surface_->setIndicator(presentEmulationError(*owner, theme_, translations_, failure,
                emulationMachineName(*host_, translations_, session_->moduleId(), configuration)));
        } catch (const std::system_error& failure) {
            surface_->setIndicator(failure.code() == std::errc::operation_canceled
                ? QString{} : text(translations_, fallbackError));
        } catch (...) {
            surface_->setIndicator(text(translations_, fallbackError));
        }
        if (previous != session_->runtimeRevision()) {
            executionRate_.reset();
            for (auto& row : rows_) row.activeUntil = {};
            session_->stopAudio();
        }
        poll();
        refreshMedia();
        refreshQuickStates();
    }
    void refreshQuickStates() {
        try {
            const bool supported = session_->canQuickSave();
            quickSave_->setEnabled(supported);
            quickLoad_->setEnabled(supported);
        } catch (...) {
            quickSave_->setEnabled(false);
            quickLoad_->setEnabled(false);
        }
    }
    void quickState(bool save) {
        run([this, save] {
            if (save) return !session_->canQuickSave() || session_->quickSave();
            return !session_->canQuickLoad() || session_->quickLoad();
        }, StateOperationFailedKey);
    }
    QString stateSlotTimestamp(int slot) const {
        const auto modified = session_->stateSlotModified(slot);
        if (!modified.isValid()) return {};
        return QLocale(QString::fromStdString(translations_.language())).toString(modified, QLocale::ShortFormat);
    }
    void useStateSlot(bool save, int slot) {
        run([this, save, slot] {
            return save ? session_->saveStateSlot(slot) : session_->loadStateSlot(slot);
        }, StateOperationFailedKey);
    }
    void showStateSlots(bool save) {
        auto* button = save ? quickSave_ : quickLoad_;
        QMenu menu(button);
        for (int slot = Session::FirstStateSlot; slot <= Session::StateSlotCount; ++slot) {
            const auto date = stateSlotTimestamp(slot);
            const auto number = QString::number(slot);
            auto* action = menu.addAction(date.isEmpty() ? number : QStringLiteral("%1  —  %2").arg(number, date));
            action->setData(slot);
            action->setEnabled(save || !date.isEmpty());
        }
        if (auto* selected = menu.exec(button->mapToGlobal(QPoint(0, button->height()))))
            useStateSlot(save, selected->data().toInt());
    }
    std::vector<FullscreenOsd::Entry> osdStateSlots(bool save) {
        std::vector<FullscreenOsd::Entry> entries;
        entries.reserve(Session::StateSlotCount);
        for (int slot = Session::FirstStateSlot; slot <= Session::StateSlotCount; ++slot) {
            FullscreenOsd::Entry entry{};
            entry.label = QString::number(slot);
            entry.command = [this, save, slot] { useStateSlot(save, slot); };
            entry.enabled = [this, save, slot] {
                return session_->canQuickSave() && (save || session_->stateSlotModified(slot).isValid());
            };
            entry.status = [this, slot] { return stateSlotTimestamp(slot); };
            entries.push_back(std::move(entry));
        }
        return entries;
    }
    void stateFile(bool save) {
        const auto title = text(translations_, save ? SaveStateKey : LoadStateKey);
        auto* owner = fullScreen_ ? static_cast<QWidget*>(fullScreen_) : this;
        const auto directory = resolveEmulationFolders(workspace_.emulationFolders()).states;
        const auto path = RememberedFileDialog::choose(owner, workspace_, title, QLatin1String(EmulationStateScope),
            directory, save ? QFileDialog::AnyFile : QFileDialog::ExistingFile, save);
        if (!path.isEmpty())
            run([&] { return save ? session_->saveState(path) : session_->loadState(path); }, StateOperationFailedKey);
    }
    const StorageDevice* mediaDevice(const MediaSlot& slot) const {
        if (!mediaStorage_) return nullptr;
        const auto& devices = mediaStorage_->availableDevices;
        const auto found = std::ranges::find_if(devices, [&](const auto& device) {
            return device.slot.kind == slot.kind && device.slot.id == slot.id;
        });
        return found == devices.cend() ? nullptr : &*found;
    }
    bool canOpenMedia(const MediaSlot& slot) const {
        const auto* device = mediaDevice(slot);
        return (!device || device->removable) &&
            (slot.kind != MediaKind::MemoryCard ||
                (device && device->configurationKind == StorageConfigurationKind::MemoryCard));
    }
    QIcon mediaIcon(const MediaSlot& slot) const {
        using namespace MediaPageConstants;
        const char* asset = FileIcon;
        switch (slot.kind) {
        case MediaKind::Floppy: {
            asset = FloppyIcon;
            const auto* device = mediaDevice(slot);
            if (!device) break;
            auto diameter = device->floppyModel ? device->floppyModel->diameterInches : 0.0;
            if (device->floppyOptions) {
                const auto& models = device->floppyOptions->models;
                const auto& settings = mediaStorage_->deviceSettings;
                const auto configured = std::ranges::find_if(settings, [&](const auto& setting) {
                    return setting.slot.kind == slot.kind && setting.slot.id == slot.id;
                });
                if (configured != settings.cend() && configured->floppy) {
                    const auto model = std::ranges::find_if(models, [&](const auto& model) {
                        return model.choice.id == configured->floppy->model;
                    });
                    if (model != models.cend()) diameter = model->diameterInches;
                    else if (models.size() == 1) diameter = models.front().diameterInches;
                } else if (models.size() == 1) {
                    diameter = models.front().diameterInches;
                }
            }
            for (const auto& [size, icon] : FloppyMediaIcons)
                if (diameter == size) { asset = icon; break; }
            break;
        }
        case MediaKind::OpticalDisc: asset = OpticalIcon; break;
        case MediaKind::Tape: asset = TapeIcon; break;
        case MediaKind::Cartridge: asset = CartridgeMediaIcon; break;
        case MediaKind::HardDisk: asset = HardDiskMediaIcon; break;
        case MediaKind::MemoryCard: break;
        }
        return QIcon(assetPath(QString::fromLatin1(ResourceIconPath).arg(QLatin1String(asset))));
    }
    QString mediaName(const MediaSlot& slot) const {
        if (const auto* device = mediaDevice(slot); device && !device->displayLabel.isEmpty())
            return device->displayLabel;
        if (slot.displayResourceKey.isEmpty()) return slot.id;
        const auto localized = host_->translate(session_->moduleId(), slot.displayResourceKey,
            QString::fromStdString(translations_.language()));
        if (localized != slot.displayResourceKey) return localized;
        const auto common = QString::fromStdString(translations_.text(slot.displayResourceKey.toStdString()));
        return common.startsWith(QLatin1Char('[')) ? slot.id : common;
    }
    QString mediaFolderScope(const MediaSlot& slot) const {
        static constexpr auto categories = std::to_array<const char*>(
            {"Floppy", "HardDisk", "CompactDisc", "Tape", "Cartridge", "MemoryCard"});
        return QString::fromLatin1(EmulationMediaScope).arg(
            QString::fromLatin1(session_->moduleId().toUtf8().toPercentEncoding()),
            QString::fromLatin1(session_->configuration().machineId.toUtf8().toPercentEncoding()),
            QLatin1String(categories.at(static_cast<std::size_t>(slot.kind))));
    }
    void insertMedia(const MediaSlot& slot) {
        auto* parent = fullScreen_ ? static_cast<QWidget*>(fullScreen_) : this;
        if (slot.kind == MediaKind::MemoryCard) {
            const auto* currentDevice = mediaDevice(slot);
            if (!currentDevice) return;
            const auto device = *currentDevice;
            const auto& mounted = session_->mountedMedia();
            const auto current = std::ranges::find_if(mounted, [&](const auto& item) {
                return item.slot.kind == slot.kind && item.slot.id == slot.id;
            });
            const auto previous = current == mounted.cend() ? QString{} : current->path;
            const auto selected = EmulationStoragePanel::selectMemoryCard(*parent, device, mediaName(slot), previous,
                workspace_, translations_, mediaFolderScope(slot), [this](const QString& key) {
                    return host_->translate(session_->moduleId(), key, QString::fromStdString(translations_.language()));
                });
            if (!selected || *selected == previous) return;
            if (selected->isEmpty()) ejectMedia(slot);
            else run([&] { return session_->insertMedia({slot, *selected, false}); }, MediaOperationFailedKey);
            return;
        }
        const bool canSelectReadOnly = slot.kind == MediaKind::Floppy || slot.kind == MediaKind::Tape;
        QFileDialog dialog(parent);
        dialog.setOption(QFileDialog::DontUseNativeDialog, canSelectReadOnly);
        dialog.setWindowTitle(text(translations_, InsertKey));
        dialog.setFileMode(QFileDialog::ExistingFile);
        QStringList patterns;
        for (const auto& extension : slot.acceptedExtensions)
            if (!extension.isEmpty()) patterns.append(QLatin1String(AnyExtension) + extension);
        if (patterns.isEmpty()) patterns.append(QLatin1String(AnyExtension));
        dialog.setNameFilter(QString::fromLatin1(MediaNameFilter).arg(
            text(translations_, AssociatedMediaKey), patterns.join(QLatin1Char(' '))));
        QCheckBox* readOnly = nullptr;
        if (canSelectReadOnly) {
            if (auto* grid = qobject_cast<QGridLayout*>(dialog.layout())) {
                readOnly = new QCheckBox(text(translations_, MediaReadOnlyKey), &dialog);
                grid->addWidget(readOnly, grid->rowCount(), 0, 1, grid->columnCount());
            }
        }
        const auto scope = mediaFolderScope(slot);
        RememberedFileDialog::prepare(dialog, workspace_, scope, workspace_.defaultImageDirectory());
        RememberedFileDialog::remember(dialog, workspace_, scope);
        if (dialog.exec() != QDialog::Accepted || dialog.selectedFiles().isEmpty()) return;
        const QFileInfo file(dialog.selectedFiles().front());
        const bool protect = StorageRules::requiresReadOnly(slot.kind) ||
            (readOnly && readOnly->isChecked()) || !file.isWritable();
        run([&] { return session_->insertMediaFile({slot, file.absoluteFilePath(), protect}); }, MediaOperationFailedKey);
    }
    void ejectMedia(const MediaSlot& slot) {
        run([&] { return session_->ejectMedia(slot); }, MediaOperationFailedKey);
    }
    std::optional<TapePcmSettings> chooseTapePcm(const MediaSlot& slot, QWidget* parent) {
        QDialog dialog(parent);
        dialog.setWindowTitle(text(translations_, TapePcmOutputKey));
        auto* form = new QFormLayout(&dialog);
        const TapePcmSettings defaults;
        const auto current = [&](const char* id) {
            return slot.settings.value(QString::fromLatin1(TapePcmPrefix) + QString::fromLatin1(id));
        };
        const auto number = [&](const auto& options, const char* id, const char* key, int fallback) {
            auto* combo = new QComboBox(&dialog);
            for (const auto option : options) combo->addItem(QString::number(option), option);
            const auto index = combo->findData(current(id).toInt());
            combo->setCurrentIndex(index < 0 ? combo->findData(fallback) : index);
            form->addRow(text(translations_, key), combo);
            return combo;
        };
        const auto flag = [&](const char* id, const char* key, bool fallback) {
            auto* check = new QCheckBox(&dialog);
            const auto value = current(id);
            check->setChecked(value.isEmpty() ? fallback : value.compare(QStringLiteral("true"), Qt::CaseInsensitive) == 0);
            form->addRow(text(translations_, key), check);
            return check;
        };
        auto* rate = number(TapeSampleRates, "sampleRate", "Emulation.Tape.Pcm.SampleRate", defaults.sampleRate);
        auto* channels = number(TapeChannels, "channels", "Emulation.Tape.Pcm.Channels", defaults.channels);
        auto* size = number(TapeSampleSizes, "bitsPerSample", "Emulation.Tape.Pcm.Bits", defaults.bitsPerSample);
        auto* signedSamples = flag("signed", "Emulation.Tape.Pcm.Signed", defaults.signedSamples);
        auto* littleEndian = flag("littleEndian", "Emulation.Tape.Pcm.LittleEndian", defaults.littleEndian);
        auto* actions = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        connect(actions, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(actions, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        form->addRow(actions);
        if (dialog.exec() != QDialog::Accepted) return std::nullopt;
        return TapePcmSettings{rate->currentData().toInt(), channels->currentData().toInt(),
            size->currentData().toInt(), signedSamples->isChecked(), littleEndian->isChecked()};
    }
    void recordTape(const MediaSlot& slot) {
        const auto extensions = host_->tapeRecordingExtensions(slot);
        if (extensions.isEmpty()) return;
        auto* parent = fullScreen_ ? static_cast<QWidget*>(fullScreen_) : this;
        QFileDialog dialog(parent, text(translations_, TapeControls.front().label));
        dialog.setAcceptMode(QFileDialog::AcceptSave);
        dialog.setFileMode(QFileDialog::AnyFile);
        QStringList filters;
        for (const auto& extension : extensions) {
            const auto suffix = extension.mid(1);
            filters.append(QString::fromLatin1(TapeFormatFilter).arg(suffix.toUpper(), suffix));
        }
        const auto scope = mediaFolderScope(slot);
        RememberedFileDialog::prepare(dialog, workspace_, scope, workspace_.defaultImageDirectory());
        RememberedFileDialog::remember(dialog, workspace_, scope);
        dialog.setNameFilters(filters);
        dialog.setDefaultSuffix(extensions.front().mid(1));
        connect(&dialog, &QFileDialog::filterSelected, &dialog, [&](const QString& filter) {
            const auto index = filters.indexOf(filter);
            if (index >= 0) dialog.setDefaultSuffix(extensions[index].mid(1));
        });
        if (dialog.exec() != QDialog::Accepted || dialog.selectedFiles().isEmpty()) return;
        const QFileInfo file(dialog.selectedFiles().front());
        if (!extensions.contains(QLatin1Char('.') + file.suffix(), Qt::CaseInsensitive)) {
            surface_->setIndicator(text(translations_, ErrorKey));
            return;
        }
        std::optional<TapePcmSettings> pcm;
        if (file.suffix().compare(QLatin1String(TapeRawExtension), Qt::CaseInsensitive) == 0) {
            pcm = chooseTapePcm(slot, parent);
            if (!pcm) return;
        }
        const auto path = file.absoluteFilePath();
        if (!EmulationStoragePanel::confirmImageCreation(*parent, translations_, dialog.windowTitle(), path,
                [&](std::stop_token token) { host_->createBlankTape(path, pcm, token); }, true)) return;
        run([&] { return session_->recordPreparedTape(slot, path, pcm); }, MediaOperationFailedKey);
    }
    void rebuildMedia(const QList<MediaSlot>& devices) {
        while (auto* item = mediaLayout_->takeAt(0)) {
            if (auto* widget = item->widget()) { widget->hide(); widget->deleteLater(); }
            delete item;
        }
        rows_.clear();
        for (const auto& slot : devices) {
            auto* group = new QFrame(mediaStrip_);
            group->setObjectName(QString::fromLatin1(MediaDeviceObjectName));
            group->setStyleSheet(QString::fromLatin1(MediaDeviceStyle));
            auto* layout = new QVBoxLayout(group);
            layout->setContentsMargins(MediaDevicePadding);
            layout->setSpacing(0);
            auto* heading = new QHBoxLayout;
            MediaRow row{};
            row.indicator = new StatusIndicator(group);
            heading->addWidget(row.indicator);
            row.insert = new QToolButton(group);
            row.insert->setAutoRaise(true);
            row.insert->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
            row.insert->setIcon(mediaIcon(slot));
            row.insert->setIconSize({MediaIconSize, MediaIconSize});
            row.insert->setEnabled(canOpenMedia(slot));
            connect(row.insert, &QToolButton::clicked, this, [this, slot] { insertMedia(slot); });
            heading->addWidget(row.insert);
            row.label = new QLabel(group);
            heading->addWidget(row.label);
            if (canOpenMedia(slot)) {
                row.eject = new QToolButton(group);
                row.eject->setAutoRaise(true);
                row.eject->setText(QString(EjectGlyph));
                row.eject->setToolButtonStyle(Qt::ToolButtonTextOnly);
                row.eject->setFixedSize(EjectButtonSize);
                auto policy = row.eject->sizePolicy();
                policy.setRetainSizeWhenHidden(true);
                row.eject->setSizePolicy(policy);
                connect(row.eject, &QToolButton::clicked, this, [this, slot] { ejectMedia(slot); });
                heading->addWidget(row.eject);
            }
            layout->addLayout(heading);
            if (slot.kind == MediaKind::Tape) {
                row.canRecord = !host_->tapeRecordingExtensions(slot).isEmpty();
                auto* transportContainer = new QHBoxLayout;
                transportContainer->setContentsMargins(TapeTransportMargin);
                auto* frame = new QFrame(group);
                frame->setObjectName(QString::fromLatin1(TapeTransportObjectName));
                frame->setStyleSheet(QString::fromLatin1(TapeTransportStyle).arg(
                    theme_->color(ThemeConstants::ControlBrush).name(),
                    theme_->color(ThemeConstants::BorderBrush).name()));
                auto* transport = new QHBoxLayout(frame);
                transport->setContentsMargins(TapeTransportPadding, TapeTransportPadding,
                    TapeTransportPadding, TapeTransportPadding);
                transport->setSpacing(TapeTransportButtonSpacing);
                for (size_t i = 0; i < TapeControls.size(); ++i) {
                    const auto& definition = TapeControls[i];
                    auto* button = new TapeTransportButton(definition.command, frame);
                    connect(button, &QToolButton::clicked, this, [this, slot, command = definition.command] {
                        if (command == TapeCommand::Record) { recordTape(slot); return; }
                        run([&] { return session_->machine().tape(command, {}); }, MediaOperationFailedKey);
                    });
                    row.tape[i] = button;
                    transport->addWidget(button);
                }
                transportContainer->addWidget(frame);
                transportContainer->addStretch();
                layout->addLayout(transportContainer);
            }
            rows_.push_back(row);
            mediaLayout_->addWidget(group);
        }
        mediaLayout_->addStretch();
        mediaStrip_->setVisible(!devices.isEmpty());
        devices_ = devices;
    }
    void refreshMedia(bool rebuild = false) {
        updateFullscreenMute();
        try {
            auto& machine = session_->machine();
            if (osd_) osd_->setGameName(osdGameName());
            const auto devices = session_->mediaSlots();
            const auto& configuration = session_->configuration();
            if (rebuild || devices != devices_ || configuration.values != mediaConfigurationValues_) {
                mediaStorage_ = session_->storageSettings();
                mediaConfigurationValues_ = configuration.values;
                rebuildMedia(devices);
            }
            if (devices.isEmpty()) {
                tapeBlinkTimer_.stop();
                tapeBlinkVisible_ = true;
                return;
            }
            const auto& mounted = session_->mountedMedia();
            const auto state = machine.state();
            const bool powered = state == MachineState::Running || state == MachineState::Paused;
            const auto* activity = dynamic_cast<const IMachineMediaActivity*>(&machine);
            const auto activeSlots = powered && activity ? activity->activeMediaSlots() :
                QSet<std::pair<MediaKind, QString>>{};
            const auto now = std::chrono::steady_clock::now();
            bool blinking = false;
            for (qsizetype i = 0; i < devices.size(); ++i) {
                const auto& slot = devices[i];
                auto& row = rows_[i];
                const auto current = std::find_if(mounted.begin(), mounted.end(), [&](const auto& media) { return media.slot.kind == slot.kind && media.slot.id == slot.id; });
                const auto path = current == mounted.end() ? QString{} : current->path;
                QString label;
                const auto name = mediaName(slot);
                if (!powered) row.activeUntil = {};
                else if (activeSlots.contains({slot.kind, slot.id})) row.activeUntil = now + MediaActivityPersistence;
                const auto* device = mediaDevice(slot);
                const bool present = !path.isEmpty() || (device && !device->removable);
                const auto color = now < row.activeUntil ? ActiveIndicator : present ? MountedIndicator : EmptyIndicator;
                row.indicator->setStatus({}, QColor::fromRgb(color));
                row.insert->setText(name);
                row.insert->setAccessibleName(name);
                row.insert->setToolTip(path.isEmpty() ? (row.insert->isEnabled() ? text(translations_, InsertKey) : name) : path);
                if (row.eject) {
                    row.eject->setToolTip(text(translations_, EjectKey));
                    row.eject->setAccessibleName(row.eject->toolTip());
                    row.eject->setEnabled(!path.isEmpty());
                    row.eject->setVisible(!path.isEmpty());
                }
                row.label->setToolTip(path);
                if (slot.kind == MediaKind::Tape) {
                    const auto tape = machine.tapeTransportStatus();
                    if (const auto index = static_cast<size_t>(tape.state); index < TapeStates.size())
                        label = text(translations_, TapeStates[index]);
                    for (size_t buttonIndex = 0; buttonIndex < TapeControls.size(); ++buttonIndex) {
                        const auto& control = TapeControls[buttonIndex];
                        const auto command = control.command;
                        const auto presentation = tapeButtonState(tape, command, powered, row.canRecord);
                        blinking |= presentation.blinking;
                        auto* button = row.tape[buttonIndex];
                        button->present(presentation, tapeBlinkVisible_);
                        button->setToolTip(text(translations_, control.label));
                        button->setAccessibleName(button->toolTip());
                    }
                }
                row.label->setText(label);
            }
            if (blinking && !tapeBlinkTimer_.isActive()) tapeBlinkTimer_.start();
            if (!blinking) { tapeBlinkTimer_.stop(); tapeBlinkVisible_ = true; }
        } catch (...) {
            tapeBlinkTimer_.stop();
            surface_->setIndicator(text(translations_, ErrorKey));
        }
    }
    void showScreenshotError(const char* key, std::exception_ptr cause = {}) {
        auto& owner = fullScreen_ ? static_cast<QWidget&>(*fullScreen_) : *this;
        const EmulationMessageException failure({
            .category = MessageCategory::Machine,
            .code = MessageCode::UntranslatedEmulatorMessage,
            .originalText = text(translations_, key)
        }, std::move(cause));
        presentEmulationError(owner, theme_, translations_, failure,
            emulationMachineName(*host_, translations_, session_->moduleId(), session_->configuration()));
    }

    void screenshot() {
        QImage image;
        std::function<QImage()> capture;
        try {
            if (surface_->isVisible()) {
                image = surface_->currentImage();
            } else if (const auto frame = session_->machine().latestFrame()) {
                const auto& machine = session_->machine();
                const auto* nativeSegments = dynamic_cast<const IMachineNativeSegments*>(&machine);
                const auto* nativeVectors = dynamic_cast<const IMachineNativeVectors*>(&machine);
                const auto* fieldHistory = dynamic_cast<const IMachineVideoFieldHistory*>(&machine);
                const auto viewport = surface_->size() * surface_->devicePixelRatioF();
                capture = [frame, profile = profile_, viewport,
                    fields = fieldHistory ? fieldHistory->previousVideoFields(*frame) : QList<std::shared_ptr<const VideoFrame>>{},
                    segments = nativeSegments ? nativeSegments->segmentLevels(*frame) : std::nullopt,
                    vectors = nativeVectors ? nativeVectors->vectorSegments(*frame) : nullptr] {
                    return VideoSurface::softwareSnapshot(*frame, profile, viewport, fields, segments, vectors);
                };
            }
        } catch (...) {
            showScreenshotError(ErrorKey, std::current_exception());
            return;
        }
        if (image.isNull() && !capture) {
            showScreenshotError(ShortcutUnavailableKey);
            return;
        }
        const auto directory = resolveEmulationFolders(workspace_.emulationFolders()).captures;
        screenshotWorkers_.start([this, image = std::move(image), capture = std::move(capture), directory] {
            QString name;
            std::exception_ptr cause;
            try { name = saveEmulationScreenshot(capture ? capture() : image, directory); }
            catch (...) { cause = std::current_exception(); }
            QMetaObject::invokeMethod(this, [this, name = std::move(name), cause = std::move(cause)] {
                if (name.isEmpty()) showScreenshotError(ErrorKey, cause);
                else surface_->setIndicator(name);
            }, Qt::QueuedConnection);
        });
    }
    void toggleFullscreenStatistics() {
        run([this] {
            workspace_.setShowFullscreenEmulationStatistics(!workspace_.showFullscreenEmulationStatistics());
            return true;
        });
    }
    void updateFullscreenStatistics(const VideoFrame* frame, double frequency, const QString& fps,
            const QString& execution, std::optional<std::uint64_t> clockHz, const QLocale& locale) {
        if (!fullScreen_ || !workspace_.showFullscreenEmulationStatistics()) {
            if (statisticsBadge_) statisticsBadge_->hide();
            return;
        }
        if (!statisticsBadge_) {
            namespace Icons = EmulationCommandIcons;
            statisticsBadge_ = new VideoPresentation::FullscreenStatisticsBadge(*fullScreen_, {
                style()->standardIcon(QStyle::SP_ComputerIcon),
                style()->standardIcon(QStyle::SP_TitleBarMaxButton),
                Icons::icon(*this, Icons::Resume, Icons::Inactive),
                style()->standardIcon(QStyle::SP_BrowserReload),
                style()->standardIcon(QStyle::SP_DriveHDIcon)});
        }
        if (!osd_ || !statisticsBadge_->isVisible()) statisticsBadge_->setTransparency(workspace_.fullscreenOsdTransparency());
        const QString renderer = surface_->usesDirect3D11() ? text(translations_, StatisticsRendererKey)
            : surface_->usesVulkan() ? text(translations_, StatisticsVulkanKey)
            : surface_->usesOpenGl() ? text(translations_, StatisticsOpenGlKey) : QString::fromLatin1(SoftwareRendererName);
        const QString empty(QChar(0x2014));
        VideoPresentation::FullscreenStatisticsBadge::Values values{renderer, empty, empty, empty, empty};
        if (frame) {
            const auto height = frame->scanMode == ScanMode::Progressive ? qint64(frame->height) : 2LL * frame->height;
            values[1] = QString::fromUtf8(StatisticsSizeFormat).arg(locale.toString(frame->width), locale.toString(height));
            values[2] = fps;
            values[3] = QString::fromLatin1(StatisticsFrequencyFormat).arg(locale.toString(frequency, 'f', 1));
            if (!execution.isEmpty()) {
                const auto first = execution.indexOf(QChar(0x00b7));
                values[4] = (first < 0 ? execution : execution.mid(first + 1)).trimmed();
            } else if (clockHz && *clockHz > 0) {
                values[4] = QString::fromLatin1(StatisticsClockFormat).arg(locale.toString(*clockHz / HertzPerMegahertz, 'f', 2));
            }
        }
        statisticsBadge_->setValues(*surface_, values);
        if (!statisticsBadge_->isVisible()) {
            statisticsBadge_->show();
            statisticsBadge_->raise();
        }
    }

    bool saveVideoProfile(Profile draft, bool previewed) {
        if (!profiles_.save(session_->moduleId(), session_->configuration().id, draft)) {
            surface_->setIndicator(text(translations_, ErrorKey));
            return false;
        }
        const bool rendererChanged = profile_.renderer != draft.renderer;
        profile_ = std::move(draft);
        if (!previewed) surface_->setProfile(profile_);
        if (rendererChanged) {
            try {
                session_->setGraphicsApi(graphicsApi(profile_.renderer));
                refreshMachineSettings();
            } catch (...) {
                surface_->setIndicator(text(translations_, ErrorKey));
                return false;
            }
        }
        surface_->setIndicator({});
        return true;
    }

    std::vector<FullscreenOsd::Entry> videoOsdSettings() {
        using Entry = FullscreenOsd::Entry;
        std::vector<Entry> rows;
        const auto add = [&](const char* key, auto read, auto update, auto format, int low, int high,
                             bool slider, std::function<bool()> visible = {}, bool preview = true) {
            Entry entry;
            entry.label = text(translations_, key);
            entry.visible = std::move(visible);
            const auto withValue = [this, update](int value) {
                auto draft = profile_;
                update(draft, value);
                return draft;
            };
            entry.setting = FullscreenOsd::Setting{[this, read] { return read(profile_); }, std::move(format),
                [this, withValue, preview](int value) { return saveVideoProfile(withValue(value), preview); },
                low, high, slider};
            if (preview) entry.setting->preview = [this, withValue](int value) { surface_->setProfile(withValue(value)); };
            rows.push_back(std::move(entry));
        };
        const auto number = [&](const char* key, auto group, auto field, int low, int high, std::function<bool()> visible = {}) {
            add(key, [=](const Profile& p) { return (p.*group).*field; },
                [=](Profile& p, int value) { (p.*group).*field = value; },
                [](int value) { return QLocale().toString(value); }, low, high, true, std::move(visible));
        };
        const auto choices = [&](const char* key, auto options, auto read, auto write, std::function<bool()> visible = {}) {
            add(key, [options, read](const Profile& profile) {
                    const auto selected = std::invoke(read, profile);
                    const auto found = std::ranges::find(options, selected, [](const auto& entry) { return entry.first; });
                    return found == options.end() ? 0 : int(found - options.begin());
                },
                [options, write](Profile& profile, int index) { std::invoke(write, profile, options[size_t(index)].first); },
                [this, options](int index) {
                    const auto* label = options[size_t(index)].second;
                    return QLatin1String(label).startsWith(QLatin1String("Emulation.")) ?
                        text(translations_, label) : QString::fromLatin1(label);
                }, 0, int(options.size()) - 1, false, std::move(visible), false);
        };
        const auto enumField = [&](const char* key, const auto& options, auto group, auto field, auto visible) {
            choices(key, options, [=](const Profile& p) { return (p.*group).*field; },
                [=](Profile& p, auto value) { (p.*group).*field = value; }, visible);
        };
        const auto labelChoices = [&](const char* key, const auto& labels, auto group, auto field, auto visible) {
            using Value = std::remove_cvref_t<decltype((profile_.*group).*field)>;
            constexpr auto Count = std::tuple_size_v<std::remove_cvref_t<decltype(labels)>>;
            std::array<std::pair<Value, const char*>, Count> options{};
            for (size_t index = 0; index < options.size(); ++index) options[index] = {Value(index), labels[index]};
            enumField(key, options, group, field, visible);
        };
        const auto activeScreen = [this](ScreenType screen) {
            return [this, screen] { return profile_.screenType() == screen; };
        };
        const auto enabled = [this](DisplayTechnology technology) {
            return [this, technology] { return profile_.displayTechnology() == technology; };
        };
        const auto section = [&](const char* key, std::function<bool()> visible = {}) {
            Entry entry;
            entry.label = text(translations_, key);
            entry.sectionHeading = true;
            entry.enabled = [] { return false; };
            entry.visible = std::move(visible);
            rows.push_back(std::move(entry));
        };
        const auto signedValue = [](int value) {
            const QLocale locale;
            return (value > 0 ? locale.positiveSign() : QString{}) + locale.toString(value);
        };
        const auto hostDisplay = [this] { return profile_.usesHostDisplay(); };
        constexpr auto ProcessingSection = "Emulation.Video.Settings.RetroMediaLabProcessing";
        section(ProcessingSection);
        choices(VideoRendererKey, VideoRenderers, &Profile::renderer,
            [](Profile& p, Renderer value) { p.renderer = value; });
        using Adjustment = ImageAdjustmentControl;
        const auto showsAdjustments = [this] {
            constexpr std::array controls{Adjustment::Brightness, Adjustment::Contrast, Adjustment::Gamma,
                Adjustment::Saturation, Adjustment::Sharpness};
            return std::ranges::any_of(controls, [this](Adjustment control) { return profile_.showsImageAdjustment(control); });
        };
        section("Emulation.Video.Settings.General", showsAdjustments);
        constexpr std::array adjustments{
            std::tuple{"Emulation.Video.Parameter.Brightness", &ImageAdjustments::brightness, Adjustment::Brightness},
            std::tuple{"Emulation.Video.Parameter.Contrast", &ImageAdjustments::contrast, Adjustment::Contrast},
            std::tuple{"Emulation.Video.Parameter.Saturation", &ImageAdjustments::saturation, Adjustment::Saturation},
            std::tuple{"Emulation.Video.Parameter.Sharpness", &ImageAdjustments::sharpness, Adjustment::Sharpness}
        };
        const auto addAdjustment = [&](const auto& adjustment) {
            const auto& [key, field, control] = adjustment;
            add(key, [field](const Profile& p) { return p.imageAdjustments(p.screenType()).*field; },
                [field](Profile& p, int value) {
                    auto settings = p.imageAdjustments(p.screenType());
                    settings.*field = value;
                    p.setImageAdjustments(p.screenType(), settings);
                }, signedValue, ImageAdjustments::Minimum, ImageAdjustments::Maximum, true,
                [this, control] { return profile_.showsImageAdjustment(control); });
        };
        for (const auto& item : adjustments) addAdjustment(item);
        add("Emulation.Video.Gamma", [](const Profile& p) {
                const auto found = std::ranges::find(ImageAdjustments::GammaCurves, p.imageAdjustments(p.screenType()).gamma);
                return found == ImageAdjustments::GammaCurves.end() ? 0 : int(found - ImageAdjustments::GammaCurves.begin());
            }, [](Profile& p, int index) {
                auto settings = p.imageAdjustments(p.screenType());
                settings.gamma = ImageAdjustments::GammaCurves[size_t(index)];
                p.setImageAdjustments(p.screenType(), settings);
            },
            [this](int index) {
                const int curve = ImageAdjustments::GammaCurves[size_t(index)];
                if (curve == 0) return text(translations_, "Emulation.Video.Gamma.Original");
                if (curve == ImageAdjustments::Bt1886) return QStringLiteral("BT.1886");
                return QLocale().toString(curve / 100.0, 'f', 1);
            }, 0, int(ImageAdjustments::GammaCurves.size()) - 1, false,
            [this] { return profile_.showsImageAdjustment(Adjustment::Gamma); });
        section(ProcessingSection);
        choices("Emulation.Video.Technology", osdScreenTypes, &Profile::screenType, &Profile::selectScreen);
        const auto addFields = [&](const auto& fields, auto group, int maximum, auto visible, auto duration, int durationMaximum) {
            for (const auto& [key, field] : fields)
                number(key, group, field, 0, duration(field) ? durationMaximum : maximum, visible);
        };
        const auto noDuration = [](auto) { return false; };
        const auto crt = activeScreen(ScreenType::Crt);
        section("Emulation.Video.Screen.Crt", crt);
        constexpr auto StandardsKey = "Emulation.Video.Parameter.CrtColorStandards";
        constexpr auto FieldRateKey = "Emulation.Video.Parameter.CrtFieldRate";
        constexpr auto ConnectionKey = "Emulation.Video.Parameter.CrtConnection";
        constexpr auto ConnectionDegradationKey = "Emulation.Video.Parameter.CrtConnectionDegradation";
        constexpr auto MonitorPaletteKey = "Emulation.Video.Parameter.CrtMonitorPalette";
        enumField(StandardsKey, decoderChoices, &Profile::crt, &CrtProfile::colorStandards, crt);
        enumField(FieldRateKey, rateChoices, &Profile::crt, &CrtProfile::fieldRate, crt);
        enumField("Emulation.Video.Parameter.CrtMask", maskChoices, &Profile::crt, &CrtProfile::mask, crt);
        enumField(VectorPhosphorResource, phosphorChoices, &Profile::crt, &CrtProfile::phosphor, [this, crt] {
            return crt() && profile_.crt.mask == CrtMask::Monochrome;
        });
        add("Emulation.Video.Parameter.VectorBeamWidth", [](const Profile& p) { return p.crt.beamWidth; },
            [](Profile& p, int value) { p.crt.beamWidth = value; }, signedValue,
            ImageAdjustments::Minimum, ImageAdjustments::Maximum, true, crt);
        number("Emulation.Video.Parameter.CrtCurvature", &Profile::crt, &CrtProfile::curvature, 0, CrtProfile::MaximumCurvature, crt);
        enumField(ConnectionKey, connectionChoices, &Profile::crt, &CrtProfile::connection, crt);
        number(ConnectionDegradationKey, &Profile::crt, &CrtProfile::connectionDegradation, 0,
            DisplaySignalProfile::MaximumConnectionDegradation, [this, crt] { return crt() && profile_.crt.canDegrade(); });
        enumField(MonitorPaletteKey, palettes, &Profile::crt, &CrtProfile::monitorPalette, [this, crt] {
            return crt() && profile_.crt.connection == CrtConnection::LegacyMonitor;
        });

        const auto panelValue = [](const Profile& profile) -> const DisplaySignalProfile& {
            const auto* panel = profile.panelSignal();
            return panel ? *panel : profile.lcdSignal;
        };
        const auto panelChoice = [&](const char* key, const auto& options, auto field, auto visible) {
            choices(key, options, [=](const Profile& p) { return panelValue(p).*field; },
                [field](Profile& p, auto value) { if (auto* panel = p.panelSignal()) panel->*field = value; }, visible);
        };
        const auto panelSource = [&](const auto& definitions, auto current) {
            panelChoice(StandardsKey, decoderChoices, &DisplaySignalProfile::colorStandards, current);
            panelChoice(FieldRateKey, rateChoices, &DisplaySignalProfile::fieldRate, current);
            panelChoice(PanelDefinitionResource, definitions, &DisplaySignalProfile::definition, current);
        };
        const auto panelInput = [&](ScreenType screen, auto current) {
            panelChoice(ConnectionKey, orderedChoices(connectionChoices, displayConnectionChoices(screen)),
                &DisplaySignalProfile::connection, current);
            add(ConnectionDegradationKey, [panelValue](const Profile& p) { return panelValue(p).connectionDegradation; },
                [](Profile& p, int value) { if (auto* panel = p.panelSignal()) panel->connectionDegradation = value; },
                [](int value) { return QLocale().toString(value); }, 0, DisplaySignalProfile::MaximumConnectionDegradation,
                true, [this, current, panelValue] { return current() && panelValue(profile_).canDegrade(); });
            panelChoice(MonitorPaletteKey, palettes, &DisplaySignalProfile::monitorPalette, [this, current, panelValue] {
                return current() && panelValue(profile_).connection == CrtConnection::LegacyMonitor;
            });
        };
        const auto lcdControls = [&](ScreenType screen, auto member, const char* label) {
            const auto current = activeScreen(screen);
            section(label, current);
            panelSource(panelDefinitionChoices, current);
            labelChoices(LcdPixelStructureResource, lcdStructures, member, &LcdProfile::pixelStructure, current);
            labelChoices(LcdOrientationResource, lcdOrientations, member, &LcdProfile::orientation, [this, member, current] {
                const auto structure = (profile_.*member).pixelStructure;
                return current() && (structure == LcdPixelStructure::Rgb || structure == LcdPixelStructure::Bgr);
            });
            labelChoices(LcdPanelTypeResource, lcdPanelTypes, member, &LcdProfile::panelType, current);
            if (screen == ScreenType::LedBacklitLcd)
                labelChoices(LedBacklightTypeResource, ledBacklightTypes, &Profile::ledBacklight, &LedBacklightProfile::type, current);
            number("Emulation.Video.Parameter.FixedPixelBacklight", member, &LcdProfile::intensity, 0,
                FixedPixelProfile::PercentMaximum, current);
            panelInput(screen, current);
        };
        lcdControls(ScreenType::Lcd, &Profile::lcd, "Emulation.Video.FixedPixel.Technology.Lcd");
        lcdControls(ScreenType::LedBacklitLcd, &Profile::ledBacklight, "Emulation.Video.Led.Block");
        const auto oled = activeScreen(ScreenType::Oled);
        section("Emulation.Video.FixedPixel.Technology.Oled", oled);
        panelSource(panelDefinitionChoices, oled);
        labelChoices(LcdPanelTypeResource, oledPanels, &Profile::oled, &OledProfile::panelType, oled);
        labelChoices(LcdPixelStructureResource, oledStructures, &Profile::oled, &OledProfile::pixelStructure, oled);
        panelInput(ScreenType::Oled, oled);

        const auto plasma = activeScreen(ScreenType::Plasma);
        section("Emulation.Video.Technology.Plasma", plasma);
        panelSource(orderedChoices(panelDefinitionChoices, plasmaDefinitionChoices()), plasma);
        enumField(PlasmaCellStructureResource, plasmaCellChoices, &Profile::plasma, &PlasmaProfile::cellStructure, plasma);
        panelInput(ScreenType::Plasma, plasma);
        const auto vector = enabled(DisplayTechnology::Vector);
        section(VectorScreenResource, vector);
        labelChoices(VectorPhosphorResource, vectorColors, &Profile::vector, &VectorProfile::phosphorColor, vector);
        number(VectorBeamWidthResource, &Profile::vector, &VectorProfile::beamWidth,
            VectorProfile::MinimumBeamWidth, VectorProfile::MaximumBeamWidth, vector);
        number(VectorCurvatureResource, &Profile::vector, &VectorProfile::curvature, 0, CrtProfile::MaximumCurvature, vector);
        const auto vfd = enabled(DisplayTechnology::Vfd);
        section(VfdTechnologyResource, vfd);
        labelChoices(VfdColorResource, vfdColors, &Profile::vfd, &VfdProfile::color, vfd);
        enumField(PanelDefinitionResource, matrixDefinitionChoices, &Profile::vfd, &VfdProfile::definition, vfd);
        labelChoices(VfdShapeResource, dotShapes, &Profile::vfd, &VfdProfile::shape, vfd);
        const auto led = enabled(DisplayTechnology::LedMatrix);
        section(LedMatrixTechnologyResource, led);
        labelChoices("Emulation.Video.Parameter.LedMatrixColor", ledColors, &Profile::ledMatrix, &LedMatrixProfile::color, led);
        enumField(PanelDefinitionResource, matrixDefinitionChoices, &Profile::ledMatrix, &LedMatrixProfile::definition, led);
        labelChoices("Emulation.Video.Parameter.LedMatrixShape", ledShapes, &Profile::ledMatrix, &LedMatrixProfile::shape, led);
        const auto dot = enabled(DisplayTechnology::DotMatrix);
        section(DotMatrixTechnologyResource, dot);
        labelChoices(DotPaletteResource, dotPalettes, &Profile::dotMatrix, &DotMatrixProfile::palette, dot);
        enumField(PanelDefinitionResource, matrixDefinitionChoices, &Profile::dotMatrix, &DotMatrixProfile::definition, dot);
        labelChoices(DotShapeResource, dotShapes, &Profile::dotMatrix, &DotMatrixProfile::shape, dot);
        labelChoices(DotTechnologyResource, dotTechnologies, &Profile::dotMatrix, &DotMatrixProfile::technology, dot);
        const auto segments = enabled(DisplayTechnology::SegmentDisplay);
        section(SegmentDisplayResource, segments);
        labelChoices(SegmentLayoutResource, segmentLayoutLabels, &Profile::segmentDisplay, &SegmentDisplayProfile::layout, segments);
        choices(SegmentTechnologyResource, segmentTechnologies, [](const Profile& p) { return p.segmentDisplay.technology; },
            [](Profile& p, auto technology) {
                p.segmentDisplay.technology = technology;
                const auto colors = segmentColors(technology);
                if (std::ranges::find(colors, p.segmentDisplay.color) == colors.end()) p.segmentDisplay.color = colors.front();
            }, segments);
        for (const auto& [technology, key] : segmentTechnologies) {
            std::vector<std::pair<SegmentColor, const char*>> colors;
            for (const auto value : segmentColors(technology)) colors.emplace_back(value, segmentColorLabels[size_t(value)]);
            enumField(SegmentColorResource, colors, &Profile::segmentDisplay, &SegmentDisplayProfile::color,
                [this, segments, technology] { return segments() && profile_.segmentDisplay.technology == technology; });
        }
        const auto paper = enabled(DisplayTechnology::EPaper);
        section("Emulation.Video.Technology.EPaper", paper);
        labelChoices("Emulation.Video.Parameter.EPaperColorMode", inkModes, &Profile::ePaper, &EPaperProfile::colorMode, paper);
        enumField(PanelDefinitionResource, panelDefinitionChoices, &Profile::ePaper, &EPaperProfile::definition, paper);

        const auto projection = activeScreen(ScreenType::Projection);
        section("Emulation.Video.Technology.Projection", projection);
        panelSource(panelDefinitionChoices, projection);
        labelChoices(ProjectionTechnologyResource, projectionTechnologies, &Profile::projection, &ProjectionProfile::technology, projection);
        labelChoices(ProjectionSurfaceResource, projectionSurfaces, &Profile::projection, &ProjectionProfile::surface, projection);
        number("Emulation.Video.Parameter.ProjectionAmbientLight", &Profile::projection, &ProjectionProfile::ambientLight,
            0, ProjectionProfile::MaximumAmbientLight, projection);
        panelInput(ScreenType::Projection, projection);
        section(ProcessingSection, hostDisplay);
        choices(SamplingKey, videoSamplers(profile_.sampling), &Profile::sampling,
            [](Profile& p, Sampling value) { p.sampling = value; }, hostDisplay);
        appendOsdVideoModuleSettings(rows, VideoSettingsSection::Display);
        section("Emulation.Video.Settings.Restoration", [this, hostDisplay] {
            return profile_.supportsDedithering() || hostDisplay() || profile_.supportsDeinterlacing();
        });
        constexpr std::array restorationLevels{
            std::pair{ImageRestoration::DeditheringLevels[0], "Emulation.Video.Restoration.Level.None"},
            std::pair{ImageRestoration::DeditheringLevels[1], "Emulation.Video.Restoration.Level.Light"},
            std::pair{ImageRestoration::DeditheringLevels[2], "Emulation.Video.Restoration.Level.Medium"},
            std::pair{ImageRestoration::DeditheringLevels[3], "Emulation.Video.Restoration.Level.Strong"}
        };
        choices("Emulation.Video.Parameter.Dedithering", restorationLevels, [](const Profile& p) {
                const int level = qRound(double(p.restoration.dedithering) * (ImageRestoration::DeditheringLevels.size() - 1) / ImageRestoration::MaximumIntensity);
                return ImageRestoration::DeditheringLevels[size_t(level)];
            }, [](Profile& p, int value) { p.restoration.dedithering = value; },
            [this] { return profile_.supportsDedithering(); });
        number("Emulation.Video.Parameter.Debanding", &Profile::restoration, &ImageRestoration::debanding, 0, ImageRestoration::MaximumIntensity, hostDisplay);
        number("Emulation.Video.Parameter.FineDetails", &Profile::restoration, &ImageRestoration::detailRecovery, 0, ImageRestoration::MaximumIntensity, hostDisplay);
        choices("Emulation.Video.Parameter.Deinterlacing", modes, [](const Profile& p) {
                return p.deinterlacing == Deinterlacing::BobEvenLines || p.deinterlacing == Deinterlacing::BobOddLines
                    ? Deinterlacing::Bob : p.deinterlacing;
            }, [](Profile& p, Deinterlacing value) { p.deinterlacing = value; },
            [this] { return profile_.supportsDeinterlacing(); });
        appendOsdVideoModuleSettings(rows, VideoSettingsSection::Image);
        appendOsdVideoModuleSettings(rows, VideoSettingsSection::Effects);
        return rows;
    }

    QString moduleSettingText(const QString& key) const {
        const auto localized = host_->translate(session_->moduleId(), key, QString::fromStdString(translations_.language()));
        if (localized != key) return localized;
        const auto shared = QString::fromStdString(translations_.text(key.toStdString()));
        return shared.startsWith(QLatin1Char('[')) ? key : shared;
    }

    const SettingsField* osdField(const QString& blockId, const QString& fieldId) const {
        const auto block = std::ranges::find(osdBlocks_, blockId, &SettingsBlock::id);
        if (block == osdBlocks_.end() || !SettingsRules::blockVisible(osdDependencies_, osdValues_, *block) ||
            !SettingsRules::fieldVisible(osdDependencies_, osdValues_, fieldId)) return nullptr;
        const auto field = std::ranges::find(block->fields, fieldId, &SettingsField::id);
        return field == block->fields.end() || !field->visible ? nullptr : &*field;
    }

    void refreshOsdBlocks() {
        const auto config = host_->configurationForGraphicsApi(session_->moduleId(), session_->configuration(),
            graphicsApi(profile_.renderer));
        auto blocks = host_->describe(session_->moduleId(), config);
        auto ranges = host_->numericSettings(session_->moduleId(), config);
        auto videoSections = host_->videoSettingsSections(session_->moduleId(), config);
        auto dependencies = host_->settingsDependencies(session_->moduleId(), config);
        osdAudioMixerBlocks_ = host_->audioMixerBlocks(session_->moduleId(), config);
        osdValues_ = SettingsRules::values(blocks);
        osdDependencies_ = std::move(dependencies);
        osdBlocks_ = std::move(blocks);
        osdRanges_ = std::move(ranges);
        osdChoicePresentation_ = host_->choicePresentation(session_->moduleId(), config);
        osdVideoSections_ = std::move(videoSections);
    }

    bool saveOsdModuleSetting(const QString& id, QString value, bool changesAudio, const QString& enabledValue) {
        try {
            if (session_->updateSetting(id, value)) {
                refreshOsdBlocks();
                if (changesAudio) mute_->setChecked(value != enabledValue);
                surface_->setIndicator({});
                refreshMedia();
                return true;
            }
        } catch (...) {
            // Persistence can succeed before a runtime option is rejected.
            try { refreshOsdBlocks(); } catch (...) {}
        }
        surface_->setIndicator(text(translations_, ErrorKey));
        return false;
    }

    FullscreenOsd::Entry osdModuleField(const SettingsBlock& block, const SettingsField& field, bool fader = false) {
        using Entry = FullscreenOsd::Entry;
        Entry entry;
        entry.label = moduleSettingText(field.labelResourceKey);
        const auto current = [this, blockId = block.id, fieldId = field.id] { return osdField(blockId, fieldId); };
        entry.visible = [current] { return current() != nullptr; };
        entry.enabled = [current] { const auto* value = current(); return value && value->enabled; };
        const bool toggle = field.editor == SettingsEditor::Toggle;
        const bool audioToggle = toggle && block.tabResourceKey == QLatin1String(AudioTabKey) &&
            field.labelResourceKey == QLatin1String(ModuleAudioEnabledKey);
        const auto save = [this, current, audioToggle](const QString& value) {
            const auto* active = current();
            if (!active || !active->enabled) return false;
            const QString id = active->id;
            const QString enabled = active->enabledValue;
            return saveOsdModuleSetting(id, value, audioToggle, enabled);
        };
        const auto range = osdRanges_.constFind(field.id);
        const auto numeric = field.editor == SettingsEditor::Slider && range != osdRanges_.cend()
            ? ModuleNumericSetting::from(*range) : std::nullopt;
        const auto unitKey = [this, fieldId = field.id] {
            const auto choice = osdChoicePresentation_.constFind(fieldId);
            if (choice != osdChoicePresentation_.cend() && !choice->unitResourceKey.isEmpty())
                return choice->unitResourceKey;
            return osdRanges_.value(fieldId).unitResourceKey;
        };
        if (field.editor == SettingsEditor::Information || (!toggle && !numeric && field.choices.isEmpty())) {
            entry.enabled = [] { return false; };
            entry.status = [this, current, unitKey] {
                const auto* value = current();
                return value ? moduleFieldDisplay(*value, unitKey(),
                    [this](const QString& key) { return moduleSettingText(key); })
                    : QString{};
            };
            return entry;
        }
        if (numeric) {
            entry.setting = FullscreenOsd::Setting{
                [current, editor = *numeric] {
                    const auto* active = current();
                    return active ? editor.index(active->value) : 0;
                },
                [this, editor = *numeric, unitKey](int index) {
                    const auto key = unitKey();
                    return editor.display(index, text(translations_, AutomaticSettingKey),
                        key.isEmpty() ? QString{} : moduleSettingText(key));
                },
                [save, editor = *numeric](int index) { return save(editor.value(index)); },
                numeric->minimum(), numeric->maximum()};
        } else if (toggle) {
            entry.setting = FullscreenOsd::Setting{
                [current] { const auto* value = current(); return int(value && value->value == value->enabledValue); },
                [this](int index) { return text(translations_, index ? EnabledStatusKey : DisabledStatusKey); },
                [current, save](int index) {
                    const auto* field = current();
                    return field && save(index ? field->enabledValue : field->disabledValue);
                }, 0, 1, false};
        } else {
            auto choices = field.choices;
            const int unknown = int(choices.size());
            entry.setting = FullscreenOsd::Setting{
                [current, choices, unknown] {
                    const auto* value = current();
                    if (!value) return unknown;
                    const auto selected = std::ranges::find(choices, value->value, &SettingsChoice::id);
                    return int(selected - choices.begin());
                },
                [this, current, choices, unknown, unitKey](int index) {
                    if (index == unknown) {
                        const auto* value = current();
                        return value ? value->value : QString{};
                    }
                    return moduleChoiceDisplay(choices[index], unitKey(),
                        [this](const QString& key) { return moduleSettingText(key); });
                },
                [choices, save](int index) {
                    return index >= 0 && index < choices.size() && save(choices[index].id);
                }, 0, unknown - 1, fader};
        }
        return entry;
    }

    bool isMixerField(const SettingsBlock& block, const SettingsField& field) const {
        return osdAudioMixerBlocks_.contains(block.id) || block.titleResourceKey == QLatin1String(AudioMixerKey) ||
            block.titleResourceKey == QLatin1String(AudioPsgKey) || block.titleResourceKey == QLatin1String(AudioFmKey) ||
            field.id.endsWith(QLatin1String(MixerVolumeSuffix)) || field.id.endsWith(QLatin1String(MixerPreampSuffix)) ||
            field.id.contains(QLatin1String(MixerEqualizerInfix));
    }

    FullscreenOsd::Entry osdAudioMixer() {
        using Entry = FullscreenOsd::Entry;
        const auto title = text(translations_, AudioMixerKey);
        auto volume = osdVolumeSetting();
        volume.mixerSection = title;
        std::vector<Entry> faders;
        faders.push_back(std::move(volume));
        for (const auto& block : osdBlocks_) {
            if (block.tabResourceKey != QLatin1String(AudioTabKey)) continue;
            const bool separate = osdAudioMixerBlocks_.contains(block.id) ||
                block.titleResourceKey == QLatin1String(AudioPsgKey) || block.titleResourceKey == QLatin1String(AudioFmKey);
            const auto section = separate ? moduleSettingText(block.titleResourceKey) : title;
            for (const auto& field : block.fields) {
                if ((field.editor != SettingsEditor::Slider && field.editor != SettingsEditor::Choice) ||
                    !isMixerField(block, field)) continue;
                auto fader = osdModuleField(block, field, true);
                if (!fader.setting || !fader.setting->slider) continue;
                fader.mixerSection = section;
                faders.push_back(std::move(fader));
            }
        }
        Entry mixer{title, {}, std::move(faders), {}, osdCommandIcon(EmulationCommandIcons::Audio)};
        mixer.centeredMixer = true;
        return mixer;
    }

    std::vector<FullscreenOsd::Entry> osdModuleGroups(const QString& tab,
            std::optional<VideoSettingsSection> videoSection = {}) {
        std::vector<FullscreenOsd::Entry> groups;
        const auto audioOutputField = tab == QLatin1String(AudioTabKey)
            ? host_->audioOutputFieldId(session_->moduleId(), session_->configuration()) : QString{};
        for (const auto& block : osdBlocks_) {
            if (tab != block.tabResourceKey) continue;
            FullscreenOsd::Entry group;
            group.fullHeight = true;
            const bool selectsAudioOutput = !audioOutputField.isEmpty() &&
                std::ranges::find(block.fields, audioOutputField, &SettingsField::id) != block.fields.end();
            group.label = selectsAudioOutput ? text(translations_, AudioOutputKey) : moduleSettingText(block.titleResourceKey);
            QStringList fieldIds;
            for (const auto& field : block.fields) {
                if (tab == QLatin1String(AudioTabKey) && (isMixerField(block, field) ||
                    field.labelResourceKey == QLatin1String(ModuleAudioEnabledKey) || field.id == QLatin1String(Ym2413CoreId)))
                    continue;
                if (videoSection && osdVideoSections_.value(field.id, VideoSettingsSection::Display) != *videoSection) continue;
                if (field.editor != SettingsEditor::Information && field.editor != SettingsEditor::Toggle &&
                    field.editor != SettingsEditor::Choice && field.editor != SettingsEditor::Slider) continue;
                if (tab == QLatin1String(VideoKey) && field.labelResourceKey == QLatin1String("Emulation.Video.Settings.Rendering"))
                    continue;
                group.children.push_back(osdModuleField(block, field));
                fieldIds.append(field.id);
            }
            if (group.children.empty()) continue;
            group.visible = [this, tab, blockId = block.id, fieldIds] {
                return !osdDependencies_.hiddenTabs.contains(tab) &&
                    std::ranges::any_of(fieldIds, [&](const QString& field) { return osdField(blockId, field) != nullptr; });
            };
            groups.push_back(std::move(group));
        }
        return groups;
    }

    void appendOsdModuleSettings(std::vector<FullscreenOsd::Entry>& parent, const QString& tab) {
        auto groups = osdModuleGroups(tab);
        parent.insert(parent.end(), std::make_move_iterator(groups.begin()), std::make_move_iterator(groups.end()));
    }

    void appendOsdVideoModuleSettings(std::vector<FullscreenOsd::Entry>& rows, VideoSettingsSection section) {
        for (auto& group : osdModuleGroups(QLatin1String(VideoKey), section)) {
            std::vector<std::function<bool()>> visibility;
            for (auto& field : group.children) {
                if (!field.setting) continue;
                field.visible = [groupVisible = group.visible, visible = std::move(field.visible),
                        enabled = std::move(field.enabled)] {
                    return (!groupVisible || groupVisible()) && (!visible || visible()) && (!enabled || enabled());
                };
                visibility.push_back(field.visible);
            }
            if (visibility.empty()) continue;
            FullscreenOsd::Entry heading;
            heading.label = std::move(group.label);
            heading.sectionHeading = true;
            heading.enabled = [] { return false; };
            heading.visible = [visibility = std::move(visibility)] {
                return std::ranges::any_of(visibility, [](const auto& visible) { return visible(); });
            };
            rows.push_back(std::move(heading));
            for (auto& field : group.children)
                if (field.setting) rows.push_back(std::move(field));
        }
    }

    static FullscreenOsd::Entry osdReadOnlyControl(QString label, QString value) {
        FullscreenOsd::Entry entry;
        entry.label = std::move(label);
        entry.enabled = [] { return false; };
        entry.status = [value = std::move(value)] { return value; };
        return entry;
    }

    void appendOsdBindings(std::vector<FullscreenOsd::Entry>& rows, const InputBindingSet& bindings) const {
        for (const auto& definition : bindings.definitions) {
            const auto label = definition.invariantDisplayValue.isEmpty()
                ? moduleSettingText(definition.displayResourceKey) : definition.invariantDisplayValue;
            const auto value = bindings.values.value(definition.id, definition.defaultBinding);
            rows.push_back(osdReadOnlyControl(label, machineBindingLabel(value,
                [this](const QString& key) { return QString::fromStdString(translations_.text(key.toStdString())); })));
        }
    }

    FullscreenOsd::Entry osdControlsSettings() {
        using Entry = FullscreenOsd::Entry;
        const auto input = host_->inputSettings(session_->moduleId(), session_->configuration());
        Entry controls;
        controls.label = text(translations_, OsdControlsKey);
        controls.fullHeight = true;
        controls.icon = osdCommandIcon(EmulationCommandIcons::Controller);
        std::vector<std::function<bool()>> tabVisibility;
        for (const auto* key : OsdControlTabs) {
            const auto tab = QString::fromLatin1(key);
            auto rows = osdModuleGroups(tab);
            if (input && key == ControllerTabKey) {
                for (const auto& port : input->controllerPorts) {
                    Entry entry;
                    entry.label = port.displayName.isEmpty()
                        ? text(translations_, ControllerPortKey).arg(port.displayNumber.value_or(port.number))
                        : port.displayName;
                    const auto choice = std::ranges::find(port.controllerChoices, port.selectedControllerId, &ControllerChoice::id);
                    auto type = text(translations_, ControllerNoneKey);
                    if (choice != port.controllerChoices.end()) {
                        type = choice->invariantDisplayValue.isEmpty() ? moduleSettingText(choice->displayResourceKey)
                            : choice->invariantDisplayValue;
                    }
                    entry.fullHeight = true;
                    entry.children.push_back(osdReadOnlyControl(text(translations_, ControllerTypeKey), std::move(type)));
                    appendOsdBindings(entry.children, port.bindings);
                    rows.push_back(std::move(entry));
                }
            } else if (input) {
                const auto& bindings = key == KeyboardTabKey ? input->keyboard : input->mouse;
                if (bindings) appendOsdBindings(rows, *bindings);
            }
            if (rows.empty()) continue;
            Entry page{moduleSettingText(tab), {}, std::move(rows)};
            page.fullHeight = true;
            std::vector<std::function<bool()>> rowVisibility;
            for (const auto& row : page.children) rowVisibility.push_back(row.visible);
            page.visible = [this, tab, rowVisibility = std::move(rowVisibility)] {
                return !osdDependencies_.hiddenTabs.contains(tab) && std::ranges::any_of(rowVisibility,
                    [](const auto& visible) { return !visible || visible(); });
            };
            tabVisibility.push_back(page.visible);
            controls.children.push_back(std::move(page));
        }
        controls.visible = [tabVisibility = std::move(tabVisibility)] {
            return std::ranges::any_of(tabVisibility, [](const auto& visible) { return visible(); });
        };
        return controls;
    }

    FullscreenOsd::Entry osdMachineInformation() {
        using Entry = FullscreenOsd::Entry;
        std::vector<Entry> rows;
        std::vector<std::function<bool()>> sections;
        for (const auto& [tab, glyph] : OsdInformationTabs) {
            std::vector<Entry> fields;
            std::vector<std::function<bool()>> visibility;
            for (const auto& block : osdBlocks_) {
                if (block.tabResourceKey != QLatin1String(tab)) continue;
                for (const auto& field : block.fields) {
                    if (field.editor != SettingsEditor::Information) continue;
                    auto row = osdModuleField(block, field);
                    visibility.push_back(row.visible);
                    fields.push_back(std::move(row));
                }
            }
            if (fields.empty()) continue;
            Entry heading{moduleSettingText(QLatin1String(tab))};
            heading.icon = osdCommandIcon(glyph);
            heading.visible = [fields = std::move(visibility)] {
                return std::ranges::any_of(fields, [](const auto& visible) { return visible(); });
            };
            sections.push_back(heading.visible);
            rows.push_back(std::move(heading));
            std::ranges::move(fields, std::back_inserter(rows));
        }
        Entry information{text(translations_, OsdInformationKey), {}, std::move(rows), {},
            osdCommandIcon(EmulationCommandIcons::Information)};
        information.visible = [sections = std::move(sections)] {
            return std::ranges::any_of(sections, [](const auto& visible) { return visible(); });
        };
        information.centeredInformation = true;
        return information;
    }

    QString osdGameName() const {
        const auto state = session_->machine().state();
        if (state != MachineState::Running && state != MachineState::Paused) return {};
        const auto& mounted = session_->mountedMedia();
        auto preferred = std::ranges::find_if(mounted, [](const MountedMedia& media) {
            return media.slot.kind == MediaKind::Cartridge || media.slot.kind == MediaKind::OpticalDisc;
        });
        if (preferred == mounted.end())
            preferred = std::ranges::find_if(mounted, [](const MountedMedia& media) {
                return media.slot.kind != MediaKind::MemoryCard;
            });
        return preferred == mounted.end() ? QString{} : QFileInfo(preferred->path).completeBaseName();
    }

    FullscreenOsd::FooterHints osdFooterHints() const {
        namespace Keys = EmulationKeyboardShortcuts;
        namespace Physical = EmulationPhysicalShortcuts;
        const auto& configured = workspace_.emulationShortcuts();
        const auto translate = [this](const QString& key) {
            return QString::fromStdString(translations_.text(key.toStdString()));
        };
        const auto keycap = [](const QKeySequence& sequence) {
            if (sequence.count() == 1 && sequence[0].keyboardModifiers() == Qt::NoModifier) {
                switch (sequence[0].key()) {
                case Qt::Key_Up: return QStringLiteral("↑");
                case Qt::Key_Down: return QStringLiteral("↓");
                case Qt::Key_Left: return QStringLiteral("←");
                case Qt::Key_Right: return QStringLiteral("→");
                case Qt::Key_Enter:
                case Qt::Key_Return: return QStringLiteral("↵");
                default: break;
                }
            }
            return sequence.toString(QKeySequence::NativeText);
        };
        const auto caps = [&](const auto&... bindings) {
            QStringList labels;
            const auto add = [&](QString label) {
                if (!label.isEmpty() && !labels.contains(label, Qt::CaseInsensitive)) labels.append(std::move(label));
            };
            const auto append = [&](const Keys::Binding& binding) {
                const auto entry = std::ranges::find(configuredShortcuts_, &binding, &ConfiguredShortcut::binding);
                if (entry == configuredShortcuts_.end()) return;
                for (const auto& sequence : entry->keyboard->keys()) add(keycap(sequence));
                const auto key = QString::fromLatin1(binding.id);
                if (!entry->primary.isEmpty()) add(machineBindingLabel(configured.value(key).toString(), translate));
                if (!entry->alternative.isEmpty())
                    add(machineBindingLabel(configured.value(key + QLatin1String(Keys::AlternativeSuffix)).toString(), translate));
            };
            (append(bindings), ...);
            return labels;
        };
        return {caps(Keys::OsdUp, Keys::OsdDown), caps(Keys::OsdLeft, Keys::OsdRight),
            caps(Keys::OsdConfirm), caps(Keys::OsdCancel), text(translations_, OsdNavigateKey),
            text(translations_, OsdAdjustKey), text(translations_, OsdConfirmKey), text(translations_, OsdBackKey)};
    }

    void openOsd(bool pause) {
        if (!fullScreen_) return;
        if (osd_) {
            osd_->close();
            return;
        }
        bool pausedForOsd = false;
        const bool restorePointer = input_.pointerCaptured;
        const auto resumeAfterOsd = [this] {
            run([this] {
                auto& machine = session_->machine();
                if (machine.state() == MachineState::Paused) machine.execute(LifecycleCommand::Resume, {});
                return true;
            });
        };
        try {
            using Entry = FullscreenOsd::Entry;
            auto& machine = session_->machine();
            namespace Icons = EmulationCommandIcons;
            const auto action = [&](const char* key, std::function<void()> command, Icons::Glyph icon) {
                return Entry{text(translations_, key), [this, command = std::move(command)] {
                    try { command(); }
                    catch (...) { surface_->setIndicator(text(translations_, ErrorKey)); }
                }, {}, {}, osdCommandIcon(icon)};
            };
            refreshQuickStates();
            refreshOsdBlocks();
            Entry save{text(translations_, SaveStateKey), {}, osdStateSlots(true),
                [this] { return quickSave_->isEnabled(); }, osdCommandIcon(Icons::Save)};
            Entry load{text(translations_, LoadStateKey), {}, osdStateSlots(false),
                [this] { return quickLoad_->isEnabled(); }, osdCommandIcon(Icons::Load)};
            save.fullHeight = true;
            load.fullHeight = true;
            std::vector<Entry> entries;
            entries.push_back(action(OsdResumeKey, [this] { if (osd_) osd_->close(); }, Icons::Resume));
            entries.push_back({text(translations_, OsdSavedStatesKey), {}, {std::move(save), std::move(load)}, {},
                osdCommandIcon(Icons::Save)});
            entries.push_back({text(translations_, OsdVideoSettingsKey), {}, videoOsdSettings(), {},
                osdCommandIcon(Icons::Display), true});
            entries.back().fullHeight = true;
            std::vector<Entry> audioRows{osdVolumeSetting(), osdAudioMixer()};
            appendOsdModuleSettings(audioRows, QLatin1String(AudioTabKey));
            entries.push_back({text(translations_, OsdAudioKey), {}, std::move(audioRows), {},
                osdCommandIcon(Icons::Audio)});
            entries.back().fullHeight = true;
            entries.push_back(osdControlsSettings());
            QStringList tabs;
            for (const auto* key : OsdMachineTabs) tabs.append(QString::fromLatin1(key));
            for (const auto& block : osdBlocks_) {
                const auto& tab = block.tabResourceKey;
                const bool controlTab = std::ranges::any_of(OsdControlTabs,
                    [&](const char* key) { return tab == QLatin1String(key); });
                if (tab != QLatin1String(VideoKey) && tab != QLatin1String(AudioTabKey) && !controlTab && !tabs.contains(tab))
                    tabs.append(tab);
            }
            std::vector<Entry> machineTabs;
            for (const auto& tab : tabs) {
                auto groups = osdModuleGroups(tab);
                if (groups.empty()) continue;
                Entry page{moduleSettingText(tab), {}, std::move(groups)};
                page.fullHeight = true;
                std::vector<std::function<bool()>> groupVisibility;
                for (const auto& group : page.children) groupVisibility.push_back(group.visible);
                page.visible = [groupVisibility = std::move(groupVisibility)] {
                    return std::ranges::any_of(groupVisibility, [](const auto& visible) { return visible(); });
                };
                machineTabs.push_back(std::move(page));
            }
            Entry machineSettings{text(translations_, SettingsKey), {}, std::move(machineTabs), {},
                osdCommandIcon(Icons::Settings)};
            machineSettings.fullHeight = true;
            std::vector<std::function<bool()>> machineTabVisibility;
            for (const auto& page : machineSettings.children) machineTabVisibility.push_back(page.visible);
            machineSettings.visible = [pages = std::move(machineTabVisibility)] {
                return std::ranges::any_of(pages, [](const auto& visible) { return visible(); });
            };
            entries.push_back(std::move(machineSettings));
            entries.push_back(osdMachineInformation());
            entries.push_back(osdTransparencySetting());
            auto exit = action(OsdExitKey, [this] { toggleFullScreen(); }, Icons::Power);
            exit.separatorBefore = true;
            entries.push_back(std::move(exit));
            const auto footer = osdFooterHints();
            const QString media = osdGameName();
            if (pause && machine.state() == MachineState::Running) {
                machine.execute(LifecycleCommand::Pause, {});
                pausedForOsd = machine.state() == MachineState::Paused;
            }
            osdPassesInput_ = !pause;
            osd_ = new FullscreenOsd(*fullScreen_,
                emulationMachineName(*host_, translations_, session_->moduleId(), session_->configuration()), media, {},
                std::move(entries), [this, pausedForOsd, resumeAfterOsd, restorePointer] {
                    const bool wasPassingInput = std::exchange(osdPassesInput_, false);
                    liveOsdPointerPosition_.reset();
                    if (input_.pointerButtons != 0) {
                        input_.pointerButtons = 0;
                        sendInput();
                    }
                    osd_ = nullptr;
                    shortcutRepeat_.clear();
                    updateShaderBadge();
                    for (const auto& shortcut : configuredShortcuts_) shortcut.keyboard->setParent(surface_);
                    for (auto* shortcut : osdNavigationShortcuts_) shortcut->setEnabled(false);
                    if (!wasPassingInput) {
                        physicalKeys_.clear();
                        physicalKeypadKeys_.clear();
                        heldPointerButtons_ = static_cast<quint32>(QApplication::mouseButtons().toInt());
                        heldGamepads_.clear();
                        for (const auto& pad : gamepadSamples_) heldGamepads_.insert(pad.id, pad);
                    }
                    for (auto& held : heldGamepads_) {
                        for (auto& axis : held.axes) if (std::abs(axis) <= GamepadReleaseThreshold) axis = 0;
                        held.controls.removeIf([](QHash<QString, float>::iterator control) {
                            return Platform::RawGamepad::direction(control.key(), control.value()) == 0;
                        });
                    }
                    if (pausedForOsd) resumeAfterOsd();
                    surface_->setFocus();
                    if (restorePointer && !changingVideoHost_) {
                        run([this] { capturePointer(); return true; });
                        if (input_.pointerCaptured)
                            heldPointerButtons_ = static_cast<quint32>(QApplication::mouseButtons().toInt());
                    }
                });
            if (osdPassesInput_) {
                releasePointer();
                heldPointerButtons_ = static_cast<quint32>(QApplication::mouseButtons().toInt());
                liveOsdPointerPosition_ = QCursor::pos();
            } else {
                releaseInput();
            }
            osd_->setFooterHints(footer);
            osd_->setExternalKeyboardNavigation(true);
            for (const auto& shortcut : configuredShortcuts_) {
                if (EmulationPhysicalShortcuts::isOsdShortcut(QString::fromLatin1(shortcut.binding->id)))
                    shortcut.keyboard->setParent(osd_);
            }
            for (auto* shortcut : osdNavigationShortcuts_) shortcut->setEnabled(true);
            previewOsdTransparency(workspace_.fullscreenOsdTransparency());
            osd_->activate();
            updateShaderBadge();
        } catch (...) {
            osdPassesInput_ = false;
            if (pausedForOsd) resumeAfterOsd();
            surface_->setIndicator(text(translations_, ErrorKey));
        }
    }
    void toggleFullScreen() {
        const bool restorePointer = input_.pointerCaptured;
        releasePointer();
        if (fullScreen_) {
            changingVideoHost_ = true;
            const bool closed = fullScreen_->close();
            changingVideoHost_ = false;
            if (restorePointer && closed) restorePointerAfterHostChange();
            return;
        }
        fullScreen_ = new QDialog(this);
        fullScreen_->setAttribute(Qt::WA_DeleteOnClose);
        auto* layout = new QVBoxLayout(fullScreen_);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(surface_);
        connect(fullScreen_, &QDialog::finished, this, [this] {
            if (osd_) osd_->close();
            layout_->addWidget(surface_, 1);
            delete volumeBadge_.data();
            delete muteBadge_.data();
            delete statisticsBadge_.data();
            fullScreen_ = nullptr;
            midiInput_.reset();
            midiSamples_.clear();
            surface_->show();
            surface_->setFocus();
            updateShaderBadge();
            updateShaderStatusRows();
        });
        auto* actions = new QMenu(surface_);
        actions->addAction(text(translations_, OsdPauseKey), this, [this] { openOsd(true); });
        actions->addAction(text(translations_, OsdLiveKey), this, [this] { openOsd(false); });
        surface_->setContextMenuPolicy(Qt::CustomContextMenu);
        auto connection = connect(surface_, &QWidget::customContextMenuRequested, actions,
            [this, actions](const QPoint& point) { actions->popup(surface_->mapToGlobal(point)); });
        connect(fullScreen_, &QDialog::finished, actions, [this, actions, connection] {
            disconnect(connection);
            surface_->setContextMenuPolicy(Qt::DefaultContextMenu);
            actions->deleteLater();
        });
        fullScreen_->showFullScreen();
        updateShaderStatusRows();
        surface_->show();
        updateFullscreenMute();
        surface_->setFocus();
        updateShaderBadge();
        if (restorePointer) restorePointerAfterHostChange();
    }
    void restorePointerAfterHostChange() {
        QTimer::singleShot(0, this, [this] {
            run([this] { capturePointer(); return true; });
        });
    }
    void poll() {
        try {
            const auto revision = session_->runtimeRevision();
            if (presentedRuntime_ != revision) {
                presentedRuntime_ = revision;
                refreshMachineInput();
                surface_->clear();
                executionRate_.reset();
            }
            const auto inputState = session_->machine().state();
            if (inputState == MachineState::Running || inputState == MachineState::Paused || fullScreen_) pollControllers();
            auto& machine = session_->machine();
            const auto state = machine.state();
            const bool running = state == MachineState::Running;
            const bool paused = state == MachineState::Paused;
            if (gamepadSource_) {
                const bool active = running && physicalInputActive() && (!osd_ || osdPassesInput_);
                controllerFeedback_.update(*gamepadSource_,
                    active ? dynamic_cast<const IMachineControllerFeedback*>(&machine) : nullptr,
                    workspace_.logsDirectory, QString::fromStdString(translations_.language()));
            }
            if (input_.pointerCaptured && (!surface_->window()->isActiveWindow() || !pointerCaptureAllowed()))
                releasePointer();
            power_->setEnabled(state == MachineState::Created || state == MachineState::Stopped
                || running || paused || state == MachineState::Faulted);
            pause_->setEnabled(running || paused);
            mute_->setEnabled(running || paused);
            refreshControllerPointer(running || paused);
            updateCommandInputStatus(commandInputStatus_.second);
            if (lifecycleState_ != state) {
                namespace Icons = EmulationCommandIcons;
                power_->setIcon(Icons::icon(*power_, Icons::Power, running || paused ? Icons::Powered : Icons::Unpowered));
                pause_->setIcon(Icons::icon(*pause_, paused ? Icons::Resume : Icons::Pause,
                    paused ? Icons::Powered : Icons::PauseColor));
                lifecycleState_ = state;
            }
            reset_->setEnabled(running || paused);
            hardReset_->setEnabled(running || paused);
            screenshotButton_->setEnabled(running || paused);
            save_->setEnabled((running || paused) && machine.supportsSavedStates());
            load_->setEnabled(save_->isEnabled());
            surface_->setEmulationRunning(running);
            const auto frame = running || paused ? machine.latestFrame() : nullptr;
            if (!frame && !paused) surface_->submit(nullptr);
            const bool visible = surface_->isVisible();
            if (!running || !visible) executionRate_.reset();
            std::optional<std::uint64_t> clockHz;
            if (frame && visible) {
                const auto* nativeSegments = dynamic_cast<const IMachineNativeSegments*>(&machine);
                const auto* nativeVectors = dynamic_cast<const IMachineNativeVectors*>(&machine);
                const auto* fieldHistory = frame->scanMode == ScanMode::Progressive
                    ? nullptr : dynamic_cast<const IMachineVideoFieldHistory*>(&machine);
                surface_->submit(frame, nativeSegments ? nativeSegments->segmentLevels(*frame) : std::nullopt,
                    nativeVectors ? nativeVectors->vectorSegments(*frame) : nullptr,
                    fieldHistory ? fieldHistory->previousVideoFields(*frame) : QList<std::shared_ptr<const VideoFrame>>{});
                if (running || (fullScreen_ && workspace_.showFullscreenEmulationStatistics())) {
                    if (const auto* status = dynamic_cast<const IMachineCoreRuntimeStatus*>(&machine))
                        clockHz = status->coreRuntimeStatus().processorClockHz;
                    if (!clockHz)
                        if (const auto* clock = dynamic_cast<const IMachineProcessorClock*>(&machine))
                            clockHz = clock->processorClockHz();
                    if (running)
                        executionRate_.observe(frame->sequence, frame->timestamp, ExecutionRateWindow::Clock::now(), clockHz);
                }
            }
            const bool audible = running && surface_->isVisible() && !mute_->isChecked();
            session_->pumpAudio(audible);
            QString statistics, fps, execution;
            double frequency = 0;
            const QLocale locale;
            if (!osd_) surface_->setOsdTransparency(fullScreen_ ? workspace_.fullscreenOsdTransparency()
                : DisplayConstants::MinimumOsdTransparency);
            const bool showStatistics = fullScreen_ ? workspace_.showFullscreenEmulationStatistics() : statistics_->isChecked();
            if (showStatistics && frame && (running || paused)) {
                const auto expected = machine.framesPerSecond();
                const auto measured = executionRate_.measuredFramesPerSecond();
                frequency = std::isfinite(expected) && expected > 0 ? expected : measured;
                fps = text(translations_, FpsKey).arg(locale.toString(measured, 'f', 1));
                const auto imageHeight = frame->scanMode == ScanMode::Progressive ? qint64(frame->height) : 2LL * frame->height;
                statistics = QString::fromUtf8(FrameStatisticsFormat).arg(locale.toString(frame->width),
                    locale.toString(imageHeight), locale.toString(frequency, 'f', 1), fps);
                if (const auto& rate = executionRate_.rate(); rate && rate->effectiveMegahertz) {
                    execution = text(translations_, ExecutionRateKey).arg(
                        locale.toString(*rate->effectiveMegahertz, 'f', 2),
                        locale.toString(rate->speedRatio * PercentScale, 'f', 0));
                }
            }
            if (!fullScreen_ && !execution.isEmpty()) statistics += execution;
            updateFullscreenStatistics(frame.get(), frequency, fps, execution, clockHz, locale);
            surface_->setStatistics(fullScreen_ ? QString{} : std::move(statistics));
        } catch (...) {
            if (gamepadSource_) gamepadSource_->stopRumble();
            session_->stopAudio();
            surface_->setIndicator(text(translations_, ErrorKey));
        }
    }

    std::shared_ptr<Session> session_;
    std::shared_ptr<Host> host_;
    Translation& translations_;
    WorkspaceState& workspace_;
    std::shared_ptr<const ThemeData> theme_;
    VideoPresentation::ProfileStore profiles_;
    VideoPresentation::Profile profile_;
    OpenMachineSettings openMachineSettings_;
    Translation::Subscription subscription_;
    VideoSurface* surface_ = nullptr;
    QVBoxLayout* layout_ = nullptr;
    QDialog* fullScreen_ = nullptr;
    QPointer<FullscreenOsd> osd_;
    bool osdPassesInput_ = false;
    QSet<int> heldOsdKeys_;
    QPointer<VideoPresentation::ShaderLoadingBadge> shaderBadge_;
    QPointer<VideoPresentation::FullscreenVolumeBadge> volumeBadge_;
    QPointer<VideoPresentation::FullscreenMuteBadge> muteBadge_;
    QPointer<VideoPresentation::FullscreenStatisticsBadge> statisticsBadge_;
    QList<std::pair<QPointer<QWidget>, QPointer<QLabel>>> shaderStatusRows_;
    QList<QShortcut*> osdNavigationShortcuts_;
    QList<ConfiguredShortcut> configuredShortcuts_;
    QList<Platform::GamepadState> gamepadSamples_;
    std::optional<Platform::MidiInput> midiInput_;
    QList<Platform::GamepadState> midiSamples_;
    QSet<int> physicalKeys_, physicalKeypadKeys_;
    bool physicalShortcutsPrimed_ = false;
    bool dispatchingPhysicalShortcut_ = false;
    WorkspaceState::DirectorySubscription shortcutSubscription_;
    QList<SettingsBlock> osdBlocks_;
    SettingsDependencies osdDependencies_;
    QSet<QString> osdAudioMixerBlocks_;
    SettingsRules::Values osdValues_;
    QHash<QString, SettingsNumericRange> osdRanges_;
    QHash<QString, SettingsChoicePresentation> osdChoicePresentation_;
    QHash<QString, VideoSettingsSection> osdVideoSections_;
    QToolButton *power_, *pause_, *reset_, *hardReset_;
    std::optional<MachineState> lifecycleState_;
    quint64 presentedRuntime_ = 0;
    QList<std::pair<QToolButton*, const char*>> commandButtons_;
    QHash<QToolButton*, const EmulationKeyboardShortcuts::Binding*> commandShortcuts_;
    QPushButton *save_, *load_;
    QToolButton *quickSave_, *quickLoad_, *screenshotButton_;
    QThreadPool screenshotWorkers_;
    QToolButton *mute_, *controllerPointer_;
    std::optional<bool> controllerPointerMode_;
    QCheckBox* statistics_;
    QLabel *controllerStatus_, *pointerStatus_;
    std::pair<bool, bool> commandInputStatus_{};
    QList<std::pair<QPushButton*, const char*>> buttons_;
    struct MediaRow {
        StatusIndicator* indicator = nullptr;
        QLabel* label = nullptr;
        QToolButton *insert = nullptr, *eject = nullptr;
        std::array<TapeTransportButton*, TapeControls.size()> tape{};
        bool canRecord = false;
        std::chrono::steady_clock::time_point activeUntil{};
    };
    QWidget* mediaStrip_ = nullptr;
    QHBoxLayout* mediaLayout_ = nullptr;
    QList<MediaSlot> devices_;
    std::optional<StorageSettings> mediaStorage_;
    QJsonObject mediaConfigurationValues_;
    QList<MediaRow> rows_;
    bool tapeBlinkVisible_ = true;
    QTimer timer_;
    QTimer mediaTimer_;
    ExecutionRateWindow executionRate_;
    QTimer tapeBlinkTimer_;
    InputSnapshot input_;
    EmulationHostKeyboard hostKeyboard_;
    EmulationKeyboardModifiers shortcutModifiers_;
    EmulationPhysicalShortcuts::ShortcutRepeat shortcutRepeat_;
    quint32 heldPointerButtons_ = 0;
    Qt::MouseButtons bindingMouseButtons_;
    bool bindingWheelPending_ = false;
    std::optional<QPointF> liveOsdPointerPosition_;
    bool changingVideoHost_ = false;
    std::unique_ptr<Platform::GamepadSource> gamepadSource_;
    EmulationControllerFeedback controllerFeedback_;
    QHash<quint64, Platform::GamepadState> heldGamepads_;
    QList<RawControllerState> rawControllerInput_;
};
}

namespace {
class SelectorButton final : public QToolButton {
public:
    std::function<void()> doubleClicked;

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override {
        if (event->button() != Qt::LeftButton || !doubleClicked) {
            QToolButton::mouseDoubleClickEvent(event);
            return;
        }
        event->accept();
        doubleClicked();
    }
};
}

struct EmulationPage::Data {
    Translation& translations;
    WorkspaceState& state;
    VideoPresentation::ProfileStore profiles;
    std::shared_ptr<const ThemeData> theme;
    QLabel *brandLabel = nullptr, *machineLabel = nullptr, *collapsedLabel = nullptr;
    QLabel *empty = nullptr, *error = nullptr;
    QToolButton* selectorToggle = nullptr;
    QPushButton* open = nullptr;
    QWidget* selectorRows = nullptr;
    FlowLayout *brands = nullptr, *machines = nullptr;
    QTabWidget* tabs = nullptr;
    QList<ModuleInfo> modules;
    QHash<QString, QList<Configuration>> saved;
    QList<std::pair<QToolButton*, QString>> machineButtons;
    Translation::Subscription subscription;

    QString translated(const QString& module, const QString& key) const {
        const auto label = state.emulation->translate(module, key, QString::fromStdString(translations.language()));
        if (!label.isEmpty() && label != key) return label;
        return QString::fromStdString(translations.text(key.toStdString()));
    }
    void showError(const QString& message) {
        error->setText(message);
        error->setVisible(!message.isEmpty());
    }

    void showMachineError(QWidget& owner, const EmulationMessageException& failure,
        const QString& module, const Configuration& configuration) {
        showError(presentEmulationError(owner, theme, translations, failure,
            emulationMachineName(*state.emulation, translations, module, configuration)));
    }

    Configuration choice(const QString& machine = {}) const {
        const auto selectedMachine = machine.isEmpty() ? state.emulationMachineId : machine;
        if (selectedMachine.isEmpty()) return {};
        const auto record = saved.constFind(state.emulationModuleId);
        if (record == saved.cend()) return {};
        const auto current = std::ranges::find_if(*record, [&](const auto& configuration) {
            return configuration.machineId == selectedMachine && configuration.id == state.emulationConfigurationId;
        });
        if (current != record->cend()) return *current;
        const auto first = std::ranges::find(*record, selectedMachine, &Configuration::machineId);
        return first == record->cend() ? Configuration{} : *first;
    }

    int activeTab(const QString& module, const QString& machine, const QUuid& configuration = {}) const {
        for (int i = 1; i < tabs->count(); ++i) {
            const auto& session = static_cast<SessionView*>(tabs->widget(i))->session();
            if (session->moduleId() != module || session->configuration().machineId != machine) continue;
            if (configuration.isNull() || session->configuration().id == configuration) return i;
        }
        return -1;
    }

    void updateSelection() {
        const auto config = choice();
        const bool canOpen = !config.id.isNull() && activeTab(state.emulationModuleId, config.machineId, config.id) < 0;
        open->setVisible(selectorToggle->isChecked() && canOpen);
        open->setEnabled(canOpen);
        for (const auto& [button, id] : machineButtons) button->setChecked(id == state.emulationMachineId);
    }

    void openMachine() {
        const auto config = choice();
        if (config.id.isNull()) return;
        try {
            const auto profile = profiles.load(state.emulationModuleId, config.id);
            if (auto session = state.emulation->openWithGraphicsApi(state.emulationModuleId, config,
                    graphicsApi(profile.renderer))) {
                addSession(session, true);
                updateSelection();
                return;
            }
        } catch (const EmulationMessageException& failure) {
            showMachineError(*tabs, failure, state.emulationModuleId, config);
            return;
        } catch (...) {}
        showError(text(translations, ErrorKey));
    }

    void selectMachine(const QString& id, bool openImmediately) {
        const auto config = choice(id);
        if (config.id.isNull()) return;
        const auto sessionTab = activeTab(state.emulationModuleId, id, openImmediately ? config.id : QUuid{});
        if (sessionTab >= 0) {
            const auto& session = static_cast<SessionView*>(tabs->widget(sessionTab))->session();
            state.emulationMachineId = id;
            state.emulationConfigurationId = session->configuration().id;
            tabs->setCurrentIndex(sessionTab);
            updateSelection();
            return;
        }
        const bool clear = !openImmediately && state.emulationMachineId == id;
        state.emulationMachineId = clear ? QString{} : id;
        state.emulationConfigurationId = clear ? QUuid{} : config.id;
        if (openImmediately) openMachine();
        updateSelection();
    }

    static void clearTiles(FlowLayout* row) {
        while (auto* item = row->takeAt(0)) {
            if (auto* widget = item->widget()) {
                widget->hide();
                widget->deleteLater();
            }
            delete item;
        }
    }

    QToolButton* tile(FlowLayout* row, const QString& path, const QString& title, bool selected) {
        if (path.trimmed().isEmpty()) return nullptr;
        const QPixmap image(QDir(QCoreApplication::applicationDirPath()).filePath(path));
        if (image.isNull()) return nullptr;
        constexpr QSize TileSize(60, 40);
        constexpr QSize ImageSize(42, 26);
        auto* button = new SelectorButton;
        button->setFixedSize(TileSize);
        button->setIconSize(ImageSize);
        button->setIcon(QIcon(image));
        button->setToolTip(title);
        button->setAccessibleName(title);
        button->setCheckable(true);
        button->setChecked(selected);
        row->addWidget(button);
        return button;
    }

    void renderSelector() {
        clearTiles(brands);
        clearTiles(machines);
        machineButtons.clear();
        for (const auto& module : modules) {
            const auto configurations = saved.constFind(module.id);
            if (configurations == saved.cend()) continue;
            const bool selected = module.id == state.emulationModuleId;
            const auto title = module.displayResourceKey.isEmpty() ? module.id : translated(module.id, module.displayResourceKey);
            if (auto* button = tile(brands, module.brandImagePath, title, selected)) {
                QObject::connect(button, &QToolButton::clicked, button, [this, id = module.id] {
                    state.emulationModuleId = id == state.emulationModuleId ? QString{} : id;
                    state.emulationMachineId.clear();
                    state.emulationConfigurationId = {};
                    renderSelector();
                });
            }
            if (!selected) continue;
            for (const auto& machine : module.machines) {
                if (std::ranges::find(*configurations, machine.id, &Configuration::machineId) == configurations->cend()) continue;
                auto* button = tile(machines, machine.imagePath,
                    translated(module.id, machine.displayResourceKey), machine.id == state.emulationMachineId);
                if (!button) continue;
                machineButtons.append({button, machine.id});
                QObject::connect(button, &QToolButton::clicked, button, [this, id = machine.id] { selectMachine(id, false); });
                static_cast<SelectorButton*>(button)->doubleClicked = [this, id = machine.id] { selectMachine(id, true); };
            }
        }
        updateSelection();
    }

    void editConfiguration(QWidget* parent, const std::shared_ptr<Session>& session = {}) {
        const auto module = session ? session->moduleId() : state.emulationModuleId;
        auto config = session ? session->configuration() : choice();
        try {
            if (session) {
                const auto savedConfigurations = state.emulation->configurations(module);
                const auto latest = std::ranges::find(savedConfigurations, config.id, &Configuration::id);
                if (latest != savedConfigurations.end()) config = *latest;
            }
            if (config.id.isNull()) return;
            QString selectedTab;
            while (editConfigurationPage(parent, session, module, config, selectedTab)) {}
        } catch (const EmulationMessageException& failure) {
            if (session) throw;
            showMachineError(*parent, failure, module, config);
        } catch (...) {
            if (session) throw;
            showError(text(translations, ErrorKey));
        }
    }
    void editSavedConfiguration(QWidget* parent, const QString& module, Configuration config) {
        const auto& sessions = state.emulation->sessions();
        const auto active = std::ranges::find_if(sessions, [&](const auto& session) {
            return session->moduleId() == module && session->configuration().id == config.id;
        });
        const auto session = active == sessions.end() ? std::shared_ptr<Session>{} : *active;
        try {
            QString selectedTab;
            while (editConfigurationPage(parent, session, module, config, selectedTab)) {}
        } catch (const EmulationMessageException& failure) {
            showMachineError(*parent, failure, module, config);
        }
    }

    void editModuleOptions(QWidget* parent, const QString& id) {
        const auto installed = state.emulation->modules();
        const auto module = std::ranges::find(installed, id, &ModuleInfo::id);
        if (module == installed.cend() || module->machines.isEmpty()) return;
        auto machine = module->machines.front().id;
        if (state.emulationModuleId == id &&
            std::ranges::find(module->machines, state.emulationMachineId, &MachineDefinition::id) != module->machines.cend())
            machine = state.emulationMachineId;
        try {
            while (!machine.isEmpty()) {
                const auto configurations = state.emulation->configurations(id);
                const auto existing = std::ranges::find(configurations, machine, &Configuration::machineId);
                const bool creating = existing == configurations.cend();
                const auto previousDraft = creating ? state.emulationDrafts->find(id, machine) : std::nullopt;
                auto draft = previousDraft ? *previousDraft : MachineConfigurationDraft{
                    creating ? state.emulation->newConfiguration(id, machine) : *existing, std::nullopt};
                auto& config = draft.configuration;
                if (config.id.isNull()) return;
                std::shared_ptr<Session> session;
                for (const auto& candidate : state.emulation->sessions()) {
                    if (candidate->moduleId() == id && candidate->configuration().id == config.id) {
                        session = candidate;
                        break;
                    }
                }
                QString next;
                QString selectedTab;
                while (editConfigurationPage(parent, session, id, config, selectedTab, &*module, &next, creating ? &draft : nullptr)) {}
                machine = std::move(next);
            }
            refreshConfigurations();
        } catch (const EmulationMessageException& failure) {
            Configuration selected;
            selected.machineId = machine;
            showMachineError(*parent, failure, id, selected);
        } catch (...) {
            showError(text(translations, ErrorKey));
        }
    }

    bool editConfigurationPage(QWidget* parent, const std::shared_ptr<Session>& session, const QString& module,
        Configuration& config, QString& selectedTab, const ModuleInfo* optionsModule = nullptr,
        QString* nextMachine = nullptr, MachineConfigurationDraft* storedDraft = nullptr) {
        auto profile = profiles.load(module, config.id);
        const auto described = state.emulation->configurationForGraphicsApi(module, config, graphicsApi(profile.renderer));
        const auto describedBlocks = state.emulation->describe(module, described);
        auto ranges = state.emulation->numericSettings(module, described);
        auto editableSettings = state.emulation->editableNumericSettings(module, described);
        auto presentation = state.emulation->settingsPresentation(module, described);
        auto dependencies = state.emulation->settingsDependencies(module, described);
        auto blocks = machineSettingsEditorBlocks(describedBlocks, dependencies);
        auto refreshFields = state.emulation->settingsRefreshFields(module, described);
        auto choicePresentation = state.emulation->choicePresentation(module, described);
        auto audioOutputField = state.emulation->audioOutputFieldId(module, described);
        auto feedbackPorts = state.emulation->controllerFeedbackPorts(module, config);
        const auto firmwareDirectory = state.emulation->firmwareDirectory(module, config.machineId);
        QList<FirmwareCandidate> firmware;
        QSet<int> editedPorts;
        std::unique_ptr<Platform::GamepadSource> rumbleSource;
        QWidget* feedbackPage = nullptr;
        auto inputDraft = storedDraft && storedDraft->input
            ? storedDraft->input : state.emulation->inputSettings(module, config);
        QList<MachineInputBindings*> inputEditors;
        MachineControllerPorts* controllerPorts = nullptr;
        QSet<QWidget*> inputPages;
        QPushButton* saveButton = nullptr;
        QLabel* saveError = nullptr;
        const auto inputValid = [&] {
            return (!controllerPorts || controllerPorts->valid()) &&
                std::ranges::all_of(inputEditors, &MachineInputBindings::valid);
        };
        auto currentValues = SettingsRules::values(describedBlocks);
        QDialog dialog(parent);
        QTimer autosave;
        autosave.setSingleShot(true);
        QTimer refreshDescription;
        refreshDescription.setSingleShot(true);
        bool rebuilding = false;
        bool interactive = false, inputDirty = false, storageDirty = false;
        const bool saveWhileEditing = optionsModule && !storedDraft;
        const auto scheduleSave = [&] {
            if (interactive && saveWhileEditing && !rebuilding && !refreshDescription.isActive()) autosave.start(0);
        };
        dialog.setWindowTitle(text(translations, SettingsKey));
        auto* layout = new QVBoxLayout(&dialog);
        auto* body = new QHBoxLayout;
        layout->addLayout(body, 1);
        if (optionsModule) {
            constexpr int MachineListWidth = 250;
            constexpr int EditorSpacing = 16;
            body->setSpacing(EditorSpacing);
            auto* machineList = new QListWidget(&dialog);
            machineList->setFixedWidth(MachineListWidth);
            for (const auto& definition : optionsModule->machines) {
                auto* item = new QListWidgetItem(translated(module, definition.displayResourceKey), machineList);
                item->setData(Qt::UserRole, definition.id);
                if (definition.id == config.machineId) machineList->setCurrentItem(item);
            }
            connect(machineList, &QListWidget::currentItemChanged, &dialog,
                [nextMachine, &dialog, &inputValid, machineList](QListWidgetItem* item, QListWidgetItem* previous) {
                if (!item) return;
                if (!inputValid()) {
                    const QSignalBlocker block(machineList);
                    machineList->setCurrentItem(previous);
                    return;
                }
                *nextMachine = item->data(Qt::UserRole).toString();
                dialog.reject();
            });
            body->addWidget(machineList);
        }
        auto* sections = new QTabWidget(&dialog);
        body->addWidget(sections, 1);
        QHash<QString, QVBoxLayout*> sectionLayouts;
        QHash<QString, QWidget*> sectionPages;
        struct SettingControl {
            QString id;
            QFormLayout* form;
            QWidget* control;
            std::function<QString()> read;
            std::function<void(const QString&)> write;
        };
        struct SettingsGroup {
            const SettingsBlock* block;
            QWidget* widget;
            QWidget* page;
            bool videoHardware;
        };
        QList<SettingControl> controls;
        QList<SettingsGroup> groups;
        QList<CompactSettingsCards*> compactRuns;
        QHash<QString, CompactSettingsCards*> openCompactRuns;
        QHash<QVBoxLayout*, int> moduleInsertionPoints;
        QJsonObject changes;
        QString requestedEmulator;
        std::optional<Profile> requestedVideoProfile;
        QWidget* videoPage = nullptr;
        QGroupBox* videoHardwarePanel = nullptr;
        QVBoxLayout* videoHardwareLayout = nullptr;
        EmulatorPanel* emulatorPage = nullptr;
        if (const auto selection = state.emulation->emulatorSelection(module, config)) {
            auto scopedSelection = *selection;
            if (session) scopedSelection.locked = true;
            emulatorPage = new EmulatorPanel(translations, state.emulation, module, config, scopedSelection,
                [&](const QString& id) {
                    if (id.isEmpty()) return;
                    requestedEmulator = id;
                    dialog.accept();
                }, sections);
            sections->addTab(emulatorPage, text(translations, "Emulation.Core.Name"));
        }
        EmulationStoragePanel* storagePage = nullptr;
        if (auto storage = state.emulation->storageSettings(module, described)) {
            constexpr auto StorageScope = "EmulationStorage/%1/%2";
            storagePage = new EmulationStoragePanel(std::move(*storage), state, translations,
                QString::fromLatin1(StorageScope).arg(module, config.machineId),
                [&](const QString& key) { return translated(module, key); }, presentation.detailedHelpResourceKeys, choicePresentation, sections);
            storagePage->onChanged([&] {
                storageDirty = true;
                scheduleSave();
            });
            sections->addTab(storagePage, text(translations, "Emulation.Storage.Device.List"));
        }
        const auto refreshVisibility = [&] {
            auto visiblePages = inputPages;
            if (feedbackPage) visiblePages.insert(feedbackPage);
            if (emulatorPage) visiblePages.insert(emulatorPage);
            if (storagePage) visiblePages.insert(storagePage);
            if (videoPage && !dependencies.hiddenTabs.contains(QLatin1String(VideoKey))) visiblePages.insert(videoPage);
            bool showVideoHardware = false;
            for (const auto& group : groups) {
                const bool visible = SettingsRules::blockVisible(dependencies, currentValues, *group.block) &&
                    (!group.videoHardware || std::ranges::any_of(group.block->fields, [&](const auto& field) {
                        return field.visible && SettingsRules::fieldVisible(dependencies, currentValues, field.id);
                    }));
                showVideoHardware |= visible && group.videoHardware;
                group.widget->setVisible(visible);
                if (visible) visiblePages.insert(group.page);
            }
            if (videoHardwarePanel) videoHardwarePanel->setVisible(showVideoHardware);
            for (const auto& field : controls) {
                field.form->setRowVisible(field.control, SettingsRules::fieldVisible(dependencies, currentValues, field.id));
            }
            for (auto* compact : compactRuns) compact->reflow();
            for (int index = 0; index < sections->count(); ++index)
                sections->setTabVisible(index, visiblePages.contains(sections->widget(index)));
        };
        const auto applyChange = [&](const QString& id, const QString& value) {
            if (rebuilding) return;
            bool describeAgain = false;
            const auto patch = SettingsRules::changesFor(dependencies, currentValues, id, value);
            for (auto field = patch.constBegin(); field != patch.constEnd(); ++field) {
                const auto next = field.value().toString();
                describeAgain |= refreshFields.contains(field.key()) && currentValues.value(field.key()) != next;
                currentValues.insert(field.key(), next);
                changes.insert(field.key(), next);
                firmware.removeIf([&](const auto& candidate) {
                    return candidate.destinationFieldId == field.key() && candidate.path != next;
                });
                if (field.key() == id) continue;
                const auto target = std::ranges::find(controls, field.key(), &SettingControl::id);
                if (target != controls.end() && target->write) target->write(next);
            }
            refreshVisibility();
            if (interactive && describeAgain) {
                autosave.stop();
                refreshDescription.start(0);
            }
        };
        const int sessionIndex = session ? activeTab(module, config.machineId, config.id) : -1;
        auto* sessionView = sessionIndex > 0 ? static_cast<SessionView*>(tabs->widget(sessionIndex)) : nullptr;
        const auto saveProfile = [&](const Profile& updated) {
            return sessionView ? sessionView->applyVideoProfile(updated) : profiles.save(module, config.id, updated);
        };
        const auto buildModuleSettings = [&] {
            QHash<QVBoxLayout*, int> nextInsertion;
            for (const auto& block : blocks) {
                if (!block.visible) continue;
                auto* section = sectionLayouts.value(block.tabResourceKey);
                if (!section) {
                    auto* scroll = new QScrollArea(sections);
                    scroll->setWidgetResizable(true);
                    auto* content = new QWidget(scroll);
                    section = new QVBoxLayout(content);
                    if (block.tabResourceKey == QLatin1String(VideoKey)) {
                        videoPage = scroll;
                        if (sessionView) section->addWidget(sessionView->createShaderStatusRow(content));
                        auto* videoRow = new QHBoxLayout;
                        auto* rendering = new QFormLayout;
                        videoRow->addLayout(rendering, 2);
                        videoHardwarePanel = new QGroupBox(text(translations, MachineVideoHardwareKey), content);
                        videoHardwareLayout = new QVBoxLayout(videoHardwarePanel);
                        videoRow->addWidget(videoHardwarePanel, 1, Qt::AlignTop);
                        section->addLayout(videoRow);
                        auto* renderer = new QComboBox(content);
                        for (const auto& [value, key] : VideoRenderers)
                            renderer->addItem(text(translations, key), static_cast<int>(value));
                        const auto previous = renderer->findData(static_cast<int>(profile.renderer));
                        renderer->setCurrentIndex(previous >= 0 ? previous :
                            renderer->findData(static_cast<int>(VideoPresentation::Renderer::Software)));
                        const auto initialIndex = renderer->currentIndex();
                        QObject::connect(renderer, &QComboBox::activated, &dialog, [&, renderer, initialIndex](int index) {
                            if (index == initialIndex) return;
                            if (!inputValid()) {
                                renderer->setCurrentIndex(initialIndex);
                                return;
                            }
                            requestedVideoProfile = profile;
                            requestedVideoProfile->renderer = static_cast<VideoPresentation::Renderer>(renderer->itemData(index).toInt());
                            dialog.accept();
                        });
                        addModuleSettingRow(*rendering, renderer, text(translations, VideoRendererKey),
                            text(translations, "Emulation.Help.Video.Rendering.Short"),
                            text(translations, "Emulation.Help.Video.Rendering.Detailed"));
                        auto* processing = new QPushButton(text(translations, "Emulation.Video.Settings.RetroMediaLabProcessing"), content);
                        QObject::connect(processing, &QPushButton::clicked, &dialog, [&] {
                            if (!inputValid()) return;
                            std::function<QWidget*(QWidget*)> status;
                            if (sessionView) status = [sessionView](QWidget* parent) { return sessionView->createShaderStatusRow(parent); };
                            auto updated = editVideoProfile(&dialog, translations, profile, status);
                            if (!updated) return;
                            if (updated->renderer != profile.renderer) {
                                requestedVideoProfile = std::move(updated);
                                dialog.accept();
                                return;
                            }
                            try {
                                if (!saveProfile(*updated)) throw std::runtime_error(ErrorKey);
                                profile = std::move(*updated);
                                saveError->hide();
                            } catch (...) {
                                saveError->setText(text(translations, ErrorKey));
                                saveError->show();
                            }
                        });
                        rendering->addRow(processing);
                    }
                    scroll->setWidget(content);
                    sections->addTab(scroll, translated(module, block.tabResourceKey));
                    sectionLayouts.insert(block.tabResourceKey, section);
                    sectionPages.insert(block.tabResourceKey, scroll);
                    moduleInsertionPoints.insert(section, section->count());
                }
                const auto addModuleWidget = [&](QWidget* widget) {
                    if (!moduleInsertionPoints.contains(section)) moduleInsertionPoints.insert(section, 0);
                    auto position = nextInsertion.value(section, moduleInsertionPoints.value(section));
                    section->insertWidget(position++, widget);
                    nextInsertion.insert(section, position);
                };
                constexpr int MaximumCompactCpuFields = 8;
                constexpr auto CpuTab = "Emulation.Tab.Cpu";
                const bool videoHardware = block.tabResourceKey == QLatin1String(VideoKey) &&
                    block.titleResourceKey == QLatin1String(MachineVideoHardwareKey);
                const bool compact = !videoHardware && presentation.compactBlocks.contains(block.id) &&
                    (block.tabResourceKey != QLatin1String(CpuTab) ||
                        std::ranges::count_if(block.fields, &SettingsField::visible) <= MaximumCompactCpuFields);
                auto* compactRun = openCompactRuns.value(block.tabResourceKey);
                if (compact && !compactRun) {
                    compactRun = new CompactSettingsCards(section->parentWidget());
                    addModuleWidget(compactRun);
                    openCompactRuns.insert(block.tabResourceKey, compactRun);
                    compactRuns.append(compactRun);
                } else if (!compact) {
                    openCompactRuns.remove(block.tabResourceKey);
                    compactRun = nullptr;
                }
                QWidget* group = nullptr;
                if (videoHardware) {
                    group = new QWidget(videoHardwarePanel);
                    videoHardwareLayout->addWidget(group);
                } else {
                    auto* frame = new QGroupBox(translated(module, block.titleResourceKey),
                        compactRun ? compactRun : section->parentWidget());
                    if (compactRun) compactRun->append(frame);
                    else addModuleWidget(frame);
                    group = frame;
                }
                auto* form = new QFormLayout(group);
                if (videoHardware) form->setContentsMargins({});
                groups.append({&block, group, sectionPages.value(block.tabResourceKey), videoHardware});
                for (const auto& field : block.fields) {
                    if (!field.visible) continue;
                    const auto addRow = [&](QWidget* control) {
                        const auto detailed = presentation.detailedHelpResourceKeys.value(field.id);
                        addModuleSettingRow(*form, control, translated(module, field.labelResourceKey),
                            field.explanationResourceKey.isEmpty() ? QString{} : translated(module, field.explanationResourceKey),
                            detailed.isEmpty() ? QString{} : translated(module, detailed));
                    };
                    QWidget* control = nullptr;
                    std::function<QString()> value;
                    std::function<void(const QString&)> write;
                    QWidget* input = nullptr;
                    ModuleEditableSlider* editable = nullptr;
                    ModuleNumberEdit* numberEdit = nullptr;
                    ModuleIpv4Edit* addressEdit = nullptr;
                    ModuleRadioChoices* radios = nullptr;
                    const auto choiceStyle = choicePresentation.value(field.id);
                    const auto localizeChoice = [&](const QString& key) { return translated(module, key); };
                    const auto range = ranges.constFind(field.id);
                    const auto numeric = field.editor == SettingsEditor::Slider && range != ranges.cend() ?
                        ModuleNumericSetting::from(*range) : std::nullopt;
                    if (field.editor == SettingsEditor::OpenDirectory) {
                        constexpr auto OpenFolderKey = "Common.OpenFolder";
                        const auto caption = text(translations, OpenFolderKey);
                        const auto path = field.value;
                        auto* open = new QPushButton(caption, group);
                        open->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
                        open->setEnabled(field.enabled && QFileInfo(path).isAbsolute() && !path.startsWith(u':'));
                        QObject::connect(open, &QPushButton::clicked, &dialog, [&, path, caption] {
                            if (!QDir().mkpath(path) || !QDesktopServices::openUrl(QUrl::fromLocalFile(path)))
                                QMessageBox::warning(&dialog, caption, text(translations, ErrorKey));
                        });
                        addRow(open);
                        controls.append({field.id, form, open, {}, {}});
                        continue;
                    }
                    if (field.editor == SettingsEditor::Information) {
                        auto* status = new QLabel(moduleFieldDisplay(field, choiceStyle.unitResourceKey.isEmpty()
                            ? ranges.value(field.id).unitResourceKey : choiceStyle.unitResourceKey,
                            [&](const QString& key) { return translated(module, key); }), group);
                        status->setTextFormat(Qt::PlainText);
                        status->setWordWrap(true);
                        addRow(status);
                        controls.append({field.id, form, status, {}, {}});
                        continue;
                    }
                    const auto editableOptions = editableSettings.constFind(field.id);
                    if (numeric && editableOptions != editableSettings.cend())
                        editable = ModuleEditableSlider::create(*range, *editableOptions, field.value,
                            text(translations, AutomaticSettingKey), group);
                    if (editable) {
                        control = editable;
                        value = [editable] { return editable->value(); };
                        write = [editable](const QString& value) { editable->setValue(value); };
                    } else if (numeric) {
                        auto* row = new QWidget(group);
                        auto* rowLayout = new QHBoxLayout(row);
                        rowLayout->setContentsMargins(0, 0, 0, 0);
                        auto* slider = new QSlider(numeric->vertical() ? Qt::Vertical : Qt::Horizontal, row);
                        slider->setRange(numeric->minimum(), numeric->maximum());
                        const int initial = numeric->index(field.value);
                        slider->setValue(initial);
                        auto* caption = new QLabel(row);
                        caption->setTextFormat(Qt::PlainText);
                        const auto& unitKey = choiceStyle.unitResourceKey.isEmpty()
                            ? range->unitResourceKey : choiceStyle.unitResourceKey;
                        const auto unit = unitKey.isEmpty() ? QString{} : localizeChoice(unitKey);
                        const auto automatic = text(translations, AutomaticSettingKey);
                        const auto update = [caption, editor = *numeric, automatic, unit](int index) {
                            caption->setText(editor.dialogDisplay(index, automatic, unit));
                        };
                        update(initial);
                        QObject::connect(slider, &QSlider::valueChanged, row, update);
                        rowLayout->addWidget(slider, 1);
                        rowLayout->addWidget(caption);
                        control = row;
                        input = slider;
                        write = [slider, editor = *numeric, update](const QString& value) {
                            const QSignalBlocker block(slider);
                            slider->setValue(editor.index(value));
                            update(slider->value());
                        };
                        value = [slider, editor = *numeric, initial, original = field.value] {
                            return slider->value() == initial ? original : editor.value(slider->value());
                        };
                    } else if (field.editor == SettingsEditor::Number) {
                        numberEdit = new ModuleNumberEdit(range == ranges.cend() ? nullptr : &*range, field.value,
                            QLocale(QString::fromStdString(translations.language())), group);
                        control = input = numberEdit;
                        value = [numberEdit] { return numberEdit->value(); };
                        write = [numberEdit](const QString& value) { numberEdit->setValue(value); };
                    } else if (field.editor == SettingsEditor::IPv4) {
                        addressEdit = new ModuleIpv4Edit(field.value, group);
                        control = input = addressEdit;
                        value = [addressEdit] { return addressEdit->value(); };
                        write = [addressEdit](const QString& value) { addressEdit->setValue(value); };
                    } else if (field.editor == SettingsEditor::Toggle) {
                        auto* toggle = new QCheckBox(group);
                        toggle->setChecked(field.value == field.enabledValue);
                        control = input = toggle;
                        write = [toggle, enabled = field.enabledValue](const QString& value) {
                            const QSignalBlocker block(toggle);
                            toggle->setChecked(value == enabled);
                        };
                        value = [toggle, field] { return toggle->isChecked() ? field.enabledValue : field.disabledValue; };
                    } else if (field.editor == SettingsEditor::Choice ||
                        (field.editor == SettingsEditor::Slider && !field.choices.isEmpty())) {
                        const bool declaredPair = field.editor == SettingsEditor::Choice && field.choices.size() == 2 &&
                            field.id != audioOutputField;
                        if (!field.choices.isEmpty() && (choiceStyle.useRadioButtons || declaredPair)) {
                            radios = new ModuleRadioChoices(field.choices, field.value, choiceStyle.unitResourceKey, localizeChoice, group);
                            control = radios;
                            write = [radios](const QString& value) { radios->setValue(value); };
                            value = [radios] { return radios->value(); };
                        } else {
                            auto* combo = new QComboBox(group);
                            for (const auto& item : field.choices)
                                combo->addItem(moduleChoiceDisplay(item, choiceStyle.unitResourceKey, localizeChoice), item.id);
                            const auto index = combo->findData(field.value);
                            combo->setCurrentIndex(index >= 0 ? index : (combo->count() > 0 ? 0 : -1));
                            control = input = combo;
                            write = [combo](const QString& value) {
                                const QSignalBlocker block(combo);
                                combo->setCurrentIndex(combo->findData(value));
                            };
                            value = [combo, field] { return combo->currentIndex() < 0 ? field.value : combo->currentData().toString(); };
                        }
                    } else if (field.editor == SettingsEditor::Multiline) {
                        auto* multiline = new QPlainTextEdit(field.value, group);
                        control = input = multiline;
                        write = [multiline](const QString& value) {
                            const QSignalBlocker block(multiline);
                            multiline->setPlainText(value);
                        };
                        value = [multiline] { return multiline->toPlainText(); };
                    } else {
                        auto* line = new QLineEdit(field.value, group);
                        control = input = line;
                        write = [line](const QString& value) {
                            const QSignalBlocker block(line);
                            line->setText(value);
                        };
                        value = [line] { return line->text(); };
                        const bool firmwareField = field.editor == SettingsEditor::FirmwareFile;
                        if (firmwareField || field.editor == SettingsEditor::File || field.editor == SettingsEditor::Folder) {
                            auto* row = new QWidget(group);
                            auto* rowLayout = new QHBoxLayout(row);
                            rowLayout->setContentsMargins(0, 0, 0, 0);
                            rowLayout->addWidget(line);
                            auto* browse = new QPushButton(text(translations, OpenKey), row);
                            rowLayout->addWidget(browse);
                            const auto label = translated(module, field.labelResourceKey);
                            QObject::connect(browse, &QPushButton::clicked, row, [&, row, line, field, label, firmwareField] {
                                if (firmwareField && !firmwareDirectory.isEmpty()) {
                                    try {
                                        auto draftChanges = changes;
                                        for (const auto& candidate : firmware) draftChanges.remove(candidate.destinationFieldId);
                                        auto draft = state.emulation->applySettings(module, config, draftChanges);
                                        for (const auto& candidate : firmware)
                                            draft = state.emulation->useFirmware(module, std::move(draft), candidate);
                                        auto chosen = pickFirmware(row, translations, state.emulation, module, draft,
                                            field, line->text(), firmwareDirectory);
                                        if (!chosen) return;
                                        firmware.removeIf([&](const auto& candidate) { return candidate.destinationFieldId == field.id; });
                                        line->setText(chosen->path);
                                        applyChange(field.id, chosen->path);
                                        if (chosen->compatibility != FirmwareCompatibility::Unknown)
                                            firmware.append(std::move(*chosen));
                                        scheduleSave();
                                    } catch (...) {
                                        line->setToolTip(text(translations, ErrorKey));
                                    }
                                    return;
                                }
                                constexpr auto SettingsPathScope = "EmulationSettings/%1/%2/%3";
                                const auto scope = QString::fromLatin1(SettingsPathScope).arg(module, config.machineId, field.id);
                                const bool chooseFolder = field.editor == SettingsEditor::Folder;
                                const QFileInfo initial(line->text());
                                QFileDialog chooser(row, label);
                                chooser.setFileMode(chooseFolder ? QFileDialog::Directory : QFileDialog::ExistingFile);
                                if (chooseFolder) chooser.setOption(QFileDialog::ShowDirsOnly);
                                RememberedFileDialog::prepare(chooser, state, scope, line->text());
                                if (!chooseFolder && !initial.fileName().isEmpty() && !initial.isDir() &&
                                    chooser.directory() == initial.absoluteDir())
                                    chooser.selectFile(initial.fileName());
                                RememberedFileDialog::remember(chooser, state, scope);
                                if (chooser.exec() != QDialog::Accepted) return;
                                const auto path = chooser.selectedFiles().value(0);
                                if (!path.isEmpty()) {
                                    line->setText(path);
                                    scheduleSave();
                                }
                            });
                            control = row;
                        }
                    }
                    control->setEnabled(field.enabled);
                    addRow(control);
                    if (field.enabled) {
                        const auto onChanged = [&, id = field.id, value] { applyChange(id, value()); };
                        const auto commit = [onChanged, &scheduleSave] {
                            onChanged();
                            scheduleSave();
                        };
                        if (radios) radios->onChanged(commit);
                        else if (editable) editable->onValueChanged(commit);
                        else if (numberEdit) {
                            numberEdit->onValueChanged(onChanged);
                            QObject::connect(numberEdit, &QLineEdit::editingFinished, &dialog, scheduleSave);
                        }
                        else if (addressEdit) {
                            addressEdit->onValueChanged(onChanged);
                            QObject::connect(addressEdit, &QLineEdit::editingFinished, &dialog, scheduleSave);
                        }
                        else if (auto* slider = qobject_cast<QSlider*>(input)) {
                            QObject::connect(slider, &QSlider::valueChanged, &dialog, [&, slider, onChanged] {
                                onChanged();
                                if (!slider->isSliderDown()) scheduleSave();
                            });
                            QObject::connect(slider, &QSlider::sliderReleased, &dialog, scheduleSave);
                        } else if (auto* toggle = qobject_cast<QCheckBox*>(input))
                            QObject::connect(toggle, &QCheckBox::toggled, &dialog, commit);
                        else if (auto* combo = qobject_cast<QComboBox*>(input))
                            QObject::connect(combo, &QComboBox::currentIndexChanged, &dialog, commit);
                        else if (auto* line = qobject_cast<QLineEdit*>(input)) {
                            QObject::connect(line, &QLineEdit::textChanged, &dialog, onChanged);
                            QObject::connect(line, &QLineEdit::editingFinished, &dialog, scheduleSave);
                        } else if (auto* multiline = qobject_cast<QPlainTextEdit*>(input))
                            QObject::connect(multiline, &QPlainTextEdit::textChanged, &dialog, [&, onChanged] {
                                onChanged();
                                constexpr int TextIdleMilliseconds = 350;
                                if (interactive && saveWhileEditing && !rebuilding && !refreshDescription.isActive())
                                    autosave.start(TextIdleMilliseconds);
                            });
                    }
                    controls.append({field.id, form, control, field.enabled ? std::move(value) : nullptr, std::move(write)});
                }
            }
        };
        buildModuleSettings();
        for (auto& port : feedbackPorts) {
            if (inputDraft && std::ranges::any_of(inputDraft->controllerPorts, [&](const auto& candidate) {
                    return candidate.number == port.number && controllerModelFeedback(candidate).has_value();
                })) continue;
            if (port.availableCommands.isEmpty() || port.number <= 0 ||
                static_cast<unsigned>(port.number) > ControllerFeedbackCommands::MaximumPortCount) continue;
            auto* section = sectionLayouts.value(QString::fromLatin1(ControllerTabKey));
            if (!section) {
                auto* scroll = new QScrollArea(sections);
                scroll->setWidgetResizable(true);
                auto* content = new QWidget(scroll);
                section = new QVBoxLayout(content);
                scroll->setWidget(content);
                sections->addTab(scroll, text(translations, ControllerTabKey));
                sectionLayouts.insert(QString::fromLatin1(ControllerTabKey), section);
                sectionPages.insert(QString::fromLatin1(ControllerTabKey), scroll);
            }
            feedbackPage = sectionPages.value(QString::fromLatin1(ControllerTabKey));
            if (!rumbleSource) rumbleSource = std::make_unique<Platform::GamepadSource>();
            auto* group = new QGroupBox(port.displayName.isEmpty() ? text(translations, ControllerPortKey).arg(port.number)
                : port.displayName, section->parentWidget());
            auto* form = new QFormLayout(group);
            section->addWidget(group);
            addControllerOutputs(*form, port, *rumbleSource,
                [&](const QString& key) { return QString::fromStdString(translations.text(key.toStdString())); },
                [&](const QString& key) { return translated(module, key); },
                [&, number = port.number] {
                    editedPorts.insert(number);
                    scheduleSave();
                });
        }
        if (inputDraft) {
            const auto globalText = [&](const QString& key) {
                return QString::fromStdString(translations.text(key.toStdString()));
            };
            const auto moduleText = [&](const QString& key) { return translated(module, key); };
            const auto changed = [&] {
                inputDirty = true;
                if (saveButton) saveButton->setEnabled(inputValid());
                scheduleSave();
            };
            const auto sectionFor = [&](const char* tab) {
                const auto key = QString::fromLatin1(tab);
                auto* section = sectionLayouts.value(key);
                if (!section) {
                    auto* scroll = new QScrollArea(sections);
                    scroll->setWidgetResizable(true);
                    auto* content = new QWidget(scroll);
                    section = new QVBoxLayout(content);
                    scroll->setWidget(content);
                    sections->addTab(scroll, text(translations, tab));
                    sectionLayouts.insert(key, section);
                    sectionPages.insert(key, scroll);
                }
                inputPages.insert(sectionPages.value(key));
                return section;
            };
            const auto addBindings = [&](auto& bindingSet, const char* tab, bool keyboard) {
                if (!bindingSet || (bindingSet->definitions.isEmpty() && bindingSet->defaultVisualId.isEmpty())) return;
                auto* section = sectionFor(tab);
                auto* parent = section->parentWidget();
                if (keyboard) {
                    if (auto* artwork = ControllerArtwork::create(bindingSet->defaultVisualId, {}, {}, parent))
                        section->addWidget(artwork);
                }
                if (bindingSet->definitions.isEmpty()) return;
                auto* editor = new MachineInputBindings(*bindingSet, globalText, moduleText, changed, parent);
                inputEditors.append(editor);
                auto commands = keyboard ? ControllerArtwork::Commands{} : bindingSet->visualCommandIds;
                commands.removeIf([&](const auto& association) {
                    return std::ranges::find(bindingSet->definitions, association.value(), &InputBindingDefinition::id) ==
                        bindingSet->definitions.end();
                });
                if (!keyboard && bindingSet->compatibleVisualIds &&
                    bindingSet->compatibleVisualIds->contains(bindingSet->defaultVisualId)) {
                    if (auto* artwork = ControllerArtwork::create(bindingSet->defaultVisualId, std::move(commands),
                            [editor](const QString& id) { editor->assign(id); }, parent)) {
                        if (!rumbleSource) rumbleSource = std::make_unique<Platform::GamepadSource>();
                        artwork->observeInput(*rumbleSource, *editor);
                        constexpr int BindingWeight = 3, ArtworkWeight = 2;
                        auto* row = new QHBoxLayout;
                        row->addWidget(editor, BindingWeight);
                        row->addWidget(artwork, ArtworkWeight, Qt::AlignTop);
                        section->addLayout(row);
                        return;
                    }
                }
                section->addWidget(editor);
            };
            addBindings(inputDraft->keyboard, KeyboardTabKey, true);
            addBindings(inputDraft->mouse, MouseTabKey, false);
            if (!inputDraft->controllerPorts.isEmpty()) {
                auto* section = sectionFor(ControllerTabKey);
                if (!rumbleSource) rumbleSource = std::make_unique<Platform::GamepadSource>();
                controllerPorts = new MachineControllerPorts(inputDraft->controllerPorts, *rumbleSource, globalText, moduleText,
                    changed, section->parentWidget());
                section->addWidget(controllerPorts);
            }
        }
        for (auto* section : sectionLayouts) section->addStretch();
        for (const auto& rule : dependencies.rules) {
            if (rule.category != SettingsRuleCategory::MutuallyExclusive) continue;
            const auto source = currentValues.constFind(rule.sourceFieldId);
            const auto target = currentValues.constFind(rule.targetFieldId);
            if (source != currentValues.cend() && target != currentValues.cend() &&
                *source != rule.comparedValue && *target != rule.comparedValue)
                applyChange(rule.targetFieldId, *target);
        }
        refreshVisibility();
        if (selectedTab == QLatin1String(VideoKey) && videoPage) sections->setCurrentWidget(videoPage);
        if (selectedTab == QLatin1String("Emulation.Core.Name") && emulatorPage)
            sections->setCurrentWidget(emulatorPage);
        sections->setVisible(sections->count() > 0);
        auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
        saveButton = buttons->button(QDialogButtonBox::Save);
        constexpr auto CreateKey = "Common.Create";
        saveButton->setText(text(translations, storedDraft ? CreateKey : SaveKey));
        saveButton->setEnabled(inputValid());
        saveButton->setVisible(!saveWhileEditing);
        buttons->button(QDialogButtonBox::Cancel)->setText(text(translations, optionsModule ? "Preferences.Close" : CloseKey));
        QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        layout->addWidget(buttons);
        saveError = new QLabel(&dialog);
        saveError->setTextFormat(Qt::PlainText);
        saveError->setWordWrap(true);
        saveError->hide();
        layout->insertWidget(layout->count() - 1, saveError);
        const auto pending = [&] {
            return !changes.isEmpty() || !firmware.isEmpty() || !editedPorts.isEmpty() || inputDirty || storageDirty;
        };
        const auto commitSettings = [&](MachineSettingsSave mode, bool captureAll, const QString& emulator) {
            auto outputs = feedbackPorts;
            outputs.removeIf([&](const auto& port) { return !editedPorts.contains(port.number); });
            auto patch = changes;
            if (captureAll && !session) {
                for (const auto& field : controls)
                    if (field.read && !patch.contains(field.id)) patch.insert(field.id, field.read());
            }

            const bool stagedInput = storedDraft && storedDraft->input.has_value();
            const auto* input = emulator.isEmpty() && inputDraft && (inputDirty || stagedInput) ? &*inputDraft : nullptr;
            const auto* storage = storageDirty && storagePage ? &storagePage->settings() : nullptr;
            const auto outcome = saveMachineSettings(*state.emulation, session.get(), module, config, std::move(patch),
                outputs, firmware, storage, input, emulator, mode, graphicsApi(profile.renderer));
            if (outcome == MachineSettingsResult::Rejected) throw std::runtime_error(ErrorKey);
            if (sessionView && outcome == MachineSettingsResult::Saved) {
                if (input) sessionView->refreshMachineInput();
                sessionView->refreshMachineSettings();
            }
            if (storedDraft && outcome != MachineSettingsResult::Saved) {
                if (inputDraft && inputDirty) storedDraft->input = inputDraft;
                state.emulationDrafts->set(module, *storedDraft);
            }
            if (outcome != MachineSettingsResult::EmulatorChanged) {
                changes = {};
                firmware.clear();
                editedPorts.clear();
                inputDirty = false;
                storageDirty = false;
            }
            return outcome;
        };
        connect(&refreshDescription, &QTimer::timeout, &dialog, [&] {
            try {
                auto draftChanges = changes;
                for (const auto& candidate : firmware) draftChanges.remove(candidate.destinationFieldId);
                auto draft = draftChanges.isEmpty() ? config : state.emulation->applySettings(module, config, draftChanges);
                for (const auto& candidate : firmware)
                    draft = state.emulation->useFirmware(module, std::move(draft), candidate);
                if (draft.id != config.id || draft.machineId != config.machineId) throw std::runtime_error(ErrorKey);
                draft = state.emulation->configurationForGraphicsApi(module, draft, graphicsApi(profile.renderer));
                auto updated = state.emulation->describe(module, draft);
                auto nextDependencies = state.emulation->settingsDependencies(module, draft);
                auto nextBlocks = machineSettingsEditorBlocks(updated, nextDependencies);
                auto nextRanges = state.emulation->numericSettings(module, draft);
                auto nextEditable = state.emulation->editableNumericSettings(module, draft);
                auto nextPresentation = state.emulation->settingsPresentation(module, draft);
                auto nextRefresh = state.emulation->settingsRefreshFields(module, draft);
                auto nextChoices = state.emulation->choicePresentation(module, draft);
                auto nextAudioOutputField = state.emulation->audioOutputFieldId(module, draft);

                {
                    const QScopedValueRollback guard(rebuilding, true);
                    auto* selectedPage = sections->currentWidget();
                    const auto previousSections = sectionLayouts;
                    controls.clear();
                    for (const auto& group : groups) {
                        if (!dynamic_cast<CompactSettingsCards*>(group.widget->parentWidget()))
                            delete group.widget;
                    }
                    groups.clear();
                    for (auto* run : compactRuns) delete run;
                    compactRuns.clear();
                    openCompactRuns.clear();
                    currentValues = SettingsRules::values(updated);
                    dependencies = std::move(nextDependencies);
                    blocks = std::move(nextBlocks);
                    ranges = std::move(nextRanges);
                    editableSettings = std::move(nextEditable);
                    presentation = std::move(nextPresentation);
                    refreshFields = std::move(nextRefresh);
                    choicePresentation = std::move(nextChoices);
                    audioOutputField = std::move(nextAudioOutputField);
                    buildModuleSettings();
                    for (auto it = sectionLayouts.cbegin(); it != sectionLayouts.cend(); ++it)
                        if (!previousSections.contains(it.key())) it.value()->addStretch();
                    refreshVisibility();
                    const int selectedIndex = sections->indexOf(selectedPage);
                    if (selectedIndex >= 0 && sections->isTabVisible(selectedIndex)) sections->setCurrentIndex(selectedIndex);
                    sections->setVisible(sections->count() > 0);
                }
                saveError->hide();
                scheduleSave();
            } catch (...) {
                saveError->setText(text(translations, ErrorKey));
                saveError->show();
            }
        });
        connect(&autosave, &QTimer::timeout, &dialog, [&] {
            if (!inputValid() || !pending()) return;
            try {
                commitSettings(MachineSettingsSave::Persist, false, {});
                saveError->hide();
            } catch (...) {
                saveError->setText(text(translations, ErrorKey));
                saveError->show();
            }
        });
        interactive = true;
        const auto result = dialog.exec();
        interactive = false;
        refreshDescription.stop();
        autosave.stop();
        if (emulatorPage) emulatorPage->finish();
        const bool accepted = result == QDialog::Accepted;
        if ((!accepted && !optionsModule) || (requestedEmulator.isEmpty() && !inputValid())) return false;
        if (!accepted && !storedDraft && !pending()) return false;
        if (accepted && requestedVideoProfile) {
            if (pending()) commitSettings(storedDraft ? MachineSettingsSave::KeepDraft : MachineSettingsSave::Persist, false, {});
            if (!saveProfile(*requestedVideoProfile)) throw std::runtime_error(ErrorKey);
            selectedTab = QLatin1String(VideoKey);
            return true;
        }
        const auto save = storedDraft && !accepted ? MachineSettingsSave::KeepDraft : MachineSettingsSave::Persist;
        const auto outcome = commitSettings(save, !saveWhileEditing, requestedEmulator);
        if (outcome == MachineSettingsResult::EmulatorChanged) {
            selectedTab = QLatin1String("Emulation.Core.Name");
            return true;
        }
        if (outcome == MachineSettingsResult::Drafted) return false;
        if (storedDraft) state.emulationDrafts->remove(module, config.machineId);
        if (nextMachine && accepted) *nextMachine = config.machineId;
        if (state.emulationModuleId == module && state.emulationMachineId == config.machineId)
            state.emulationConfigurationId = config.id;
        if (!session || (state.emulationModuleId == module && state.emulationMachineId == config.machineId))
            refreshConfigurations();
        return false;
    }
    void refreshConfigurations() {
        modules = state.emulation->modules();
        saved.clear();
        const auto& issues = state.emulation->diagnostics();
        const auto issue = std::ranges::find_if(issues, [](const auto& diagnostic) {
            return diagnostic.issue != Emulation::ModuleIssue::LooseLibrary;
        });
        showError(issue == issues.cend() ? QString{} : text(translations, EmulationModuleIssueKey).arg(issue->directory));
        for (const auto& module : modules) {
            try {
                auto configurations = state.emulation->configurations(module.id);
                configurations.removeIf([](const auto& config) { return config.id.isNull(); });
                if (!configurations.isEmpty()) saved.insert(module.id, std::move(configurations));
            } catch (...) {
                showError(text(translations, ErrorKey));
            }
        }
        if (!saved.contains(state.emulationModuleId)) state.emulationModuleId.clear();
        const auto configured = choice();
        state.emulationMachineId = configured.machineId;
        state.emulationConfigurationId = configured.id;
        renderSelector();
    }
    void addSession(const std::shared_ptr<Session>& session, bool select) {
        for (int i = 1; i < tabs->count(); ++i) {
            auto* view = static_cast<SessionView*>(tabs->widget(i));
            if (view->session() == session) { if (select) tabs->setCurrentIndex(i); return; }
        }
        auto* view = new SessionView(session, state.emulation, translations, state, profiles, theme,
            [this](QWidget* parent, const auto& activeSession) { editConfiguration(parent, activeSession); }, tabs);
        const auto& configuration = session->configuration();
        const int index = tabs->addTab(view, themeIcon(QLatin1String(AppConstants::Emulation), tabs->palette().color(QPalette::WindowText)),
            emulationMachineName(*state.emulation, translations, session->moduleId(), configuration));
        if (select) tabs->setCurrentIndex(index);
    }
    void localize() {
        brandLabel->setText(text(translations, BrandKey));
        machineLabel->setText(text(translations, MachineKey));
        collapsedLabel->setText(text(translations, SelectorKey));
        selectorToggle->setToolTip(text(translations, SelectorToggleKey));
        selectorToggle->setAccessibleName(selectorToggle->toolTip());
        open->setText(text(translations, OpenKey));
        tabs->setTabText(0, text(translations, WelcomeKey));
        empty->setText(text(translations, WelcomeTextKey));
        refreshConfigurations();
    }

};

EmulationPage::EmulationPage(ThemeService& themes, Translation& translations, WorkspaceState& state, QWidget* parent)
    : QWidget(parent), data_(std::make_unique<Data>(Data{translations, state, emulationVideoProfiles(), themes.data()})) {
    auto& view = *data_;
    ensureEmulationHost(state);
    constexpr int PageMargin = 16, CardSpacing = 12, InsetX = 8, InsetY = 4;
    constexpr int SelectorHeight = 124, RowHeight = 56, ToggleSize = 28, TileSpacing = 6;
    constexpr int OpenInsetX = 12, ActionInsetY = 4, OpenMinimumWidth = 130;
    constexpr int CollapsedSelectorHeight = ToggleSize + 2 * (InsetY + ActionInsetY);
    constexpr int LabelTopInset = 11, WelcomeMargin = 32, WelcomeMaxWidth = 680, WelcomeFontPixels = 18;
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(PageMargin, PageMargin, PageMargin, PageMargin);
    layout->setSpacing(CardSpacing);
    auto* selector = new Card(themes.data(), this);
    selector->setFixedHeight(SelectorHeight);
    auto* header = new QHBoxLayout(selector);
    header->setContentsMargins(InsetX, InsetY, InsetX, InsetY);
    view.selectorRows = new QWidget(selector);
    auto* rows = new QVBoxLayout(view.selectorRows);
    rows->setContentsMargins(0, 0, 0, 0);
    rows->setSpacing(0);
    const auto addRow = [&](QLabel*& label, FlowLayout*& items) {
        auto* line = new QWidget(view.selectorRows);
        line->setMinimumHeight(RowHeight);
        auto* row = new QHBoxLayout(line);
        row->setContentsMargins(0, 0, 0, 0);
        row->setSpacing(CardSpacing);
        label = new QLabel(line);
        auto font = label->font();
        font.setWeight(QFont::DemiBold);
        label->setFont(font);
        label->setContentsMargins(0, LabelTopInset, 0, 0);
        row->addWidget(label, 0, Qt::AlignTop);
        auto* tiles = new QWidget(line);
        items = new FlowLayout(tiles);
        constexpr int TileRowInset = 2;
        items->setContentsMargins(0, TileRowInset, 0, TileRowInset);
        items->setSpacing(TileSpacing);
        row->addWidget(tiles, 1);
        rows->addWidget(line);
    };
    addRow(view.brandLabel, view.brands);
    addRow(view.machineLabel, view.machines);
    header->addWidget(view.selectorRows, 1);
    view.collapsedLabel = new QLabel(selector);
    auto collapsedFont = view.collapsedLabel->font();
    collapsedFont.setWeight(QFont::DemiBold);
    view.collapsedLabel->setFont(collapsedFont);
    view.collapsedLabel->hide();
    header->addWidget(view.collapsedLabel, 1);
    auto* openLayout = new QVBoxLayout;
    openLayout->setContentsMargins(OpenInsetX, ActionInsetY, 0, ActionInsetY);
    view.open = new QPushButton(selector);
    view.open->setMinimumWidth(OpenMinimumWidth);
    view.open->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    openLayout->addWidget(view.open);
    header->addLayout(openLayout);
    view.selectorToggle = new QToolButton(selector);
    view.selectorToggle->setFixedSize(ToggleSize, ToggleSize);
    view.selectorToggle->setCheckable(true);
    view.selectorToggle->setChecked(true);
    view.selectorToggle->setArrowType(Qt::UpArrow);
    auto* toggleLayout = new QVBoxLayout;
    constexpr int ToggleInset = 4;
    toggleLayout->setContentsMargins(ToggleInset, ToggleInset, 0, ToggleInset);
    toggleLayout->addWidget(view.selectorToggle, 0, Qt::AlignTop);
    toggleLayout->addStretch();
    header->addLayout(toggleLayout);
    connect(view.selectorToggle, &QToolButton::toggled, this, [&view, selector](bool expanded) {
        selector->setFixedHeight(expanded ? SelectorHeight : CollapsedSelectorHeight);
        view.selectorRows->setVisible(expanded);
        view.collapsedLabel->setVisible(!expanded);
        view.selectorToggle->setArrowType(expanded ? Qt::UpArrow : Qt::DownArrow);
        view.updateSelection();
    });
    connect(view.open, &QPushButton::clicked, this, [&view] { view.openMachine(); });
    layout->addWidget(selector);
    view.error = new QLabel(this);
    view.error->setTextFormat(Qt::PlainText);
    view.error->setWordWrap(true);
    view.error->setTextFormat(Qt::PlainText);
    layout->addWidget(view.error);
    view.tabs = new EmulationMachineTabs(this);
    view.tabs->setTabsClosable(true);
    auto* welcome = new QWidget(view.tabs);
    view.empty = new QLabel(welcome);
    view.empty->setAlignment(Qt::AlignCenter);
    view.empty->setWordWrap(true);
    view.empty->setMaximumWidth(WelcomeMaxWidth);
    auto welcomeFont = view.empty->font();
    welcomeFont.setPixelSize(WelcomeFontPixels);
    view.empty->setFont(welcomeFont);
    constexpr int WelcomeSideStretch = 1, WelcomeTextStretch = 10;
    auto* welcomeLayout = new QHBoxLayout(welcome);
    welcomeLayout->setContentsMargins(WelcomeMargin, WelcomeMargin, WelcomeMargin, WelcomeMargin);
    welcomeLayout->addStretch(WelcomeSideStretch);
    welcomeLayout->addWidget(view.empty, WelcomeTextStretch);
    welcomeLayout->addStretch(WelcomeSideStretch);
    view.tabs->addTab(welcome, style()->standardIcon(QStyle::SP_DirHomeIcon), {});
    view.tabs->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
    view.tabs->tabBar()->setTabButton(0, QTabBar::LeftSide, nullptr);
    layout->addWidget(view.tabs, 1);
    for (const auto& session : state.emulation->sessions()) view.addSession(session, false);
    connect(view.tabs, &QTabWidget::currentChanged, this, [&view](int index) {
        if (index <= 0) return;
        const auto& session = static_cast<SessionView*>(view.tabs->widget(index))->session();
        view.state.emulationModuleId = session->moduleId();
        view.state.emulationMachineId = session->configuration().machineId;
        view.state.emulationConfigurationId = session->configuration().id;
        view.renderSelector();
    });
    connect(view.tabs, &QTabWidget::tabCloseRequested, this, [&view](int index) {
        if (index <= 0) return;
        auto* sessionView = static_cast<SessionView*>(view.tabs->widget(index));
        const auto session = sessionView->session();
        view.tabs->removeTab(index);
        sessionView->deleteLater();
        try {
            view.state.emulation->close(session);
        } catch (const EmulationMessageException& failure) {
            view.showMachineError(*view.tabs, failure, session->moduleId(), session->configuration());
        } catch (...) {
            view.showError(text(view.translations, ErrorKey));
        }
        view.updateSelection();
    });
    view.subscription = translations.subscribe([&view] { view.localize(); });
    view.localize();
}
EmulationPage::~EmulationPage() = default;
void EmulationPage::openModuleOptions(const QString& module) {
    data_->editModuleOptions(this, module);
}
void EmulationPage::openSettings() {
    EmulationPreferencesDialog::open(this, data_->translations, data_->state, data_->profiles,
        [this](QWidget* parent, const QString& module, const Configuration& configuration) {
            data_->editSavedConfiguration(parent, module, configuration);
        });
    data_->refreshConfigurations();
}
}
