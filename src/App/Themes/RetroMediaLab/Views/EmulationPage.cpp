#include "EmulationPage.h"
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
constexpr auto OsdMutedStatus = u"✓";
constexpr auto OsdUnmutedStatus = u"—";
using VideoPresentation::FullscreenOsd;
using VideoPresentation::VideoSurface;
using Translation = Translations::TranslationService;

constexpr int FramePollMilliseconds = 16;
constexpr int MediaPollMilliseconds = 200;
constexpr int TapeBlinkMilliseconds = 400;
constexpr auto VideoKey = "Emulation.Tab.Video";
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
constexpr auto OsdAudioKey = "Emulation.Shortcut.OsdAudioSettings";
constexpr auto OsdMuteKey = "Emulation.Shortcut.Mute";
constexpr auto AutomaticSettingKey = "Emulation.Controller.Automatic";
constexpr auto ModuleAudioEnabledKey = "Emulation.Audio.Enabled";
constexpr auto AudioOutputKey = "Emulation.Audio.Output";
constexpr auto OsdExitKey = "Emulation.Shortcut.OsdExitFullscreen";
constexpr auto ScreenshotKey = "Emulation.Shortcut.Screenshot";
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
constexpr auto ModuleIssueKey = "Emulation.Native.ModuleIssue";
constexpr auto PowerKey = "Emulation.Shortcut.Power";
constexpr auto PauseResumeKey = "Emulation.Shortcut.PauseResume";
constexpr auto SoftResetKey = "Emulation.Native.SoftReset";
constexpr auto HardResetKey = "Emulation.Native.HardReset";
constexpr auto FullScreenKey = "Emulation.Native.FullScreen";
constexpr auto StatisticsKey = "Emulation.Native.Statistics";
constexpr auto FpsKey = "Emulation.Native.Fps";
constexpr auto ExecutionRateKey = "Emulation.Status.ExecutionRate";
constexpr auto FrameStatisticsFormat = "%1 × %2 · %3 Hz · %4";
constexpr double PercentScale = 100;
constexpr auto QuickSaveKey = "Emulation.Shortcut.QuickSave";
constexpr auto QuickLoadKey = "Emulation.Shortcut.QuickLoad";
constexpr auto SaveStateKey = "Emulation.Native.SaveState";
constexpr auto LoadStateKey = "Emulation.Native.LoadState";
constexpr auto ErrorKey = "Emulation.Error.EmulatorRejected";
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
    QStyle::StandardPixmap icon;
};
constexpr std::array TapeControls{
    TapeControl{TapeCommand::Record, "Emulation.Tape.Command.Record", QStyle::SP_DialogSaveButton},
    TapeControl{TapeCommand::Play, "Emulation.Tape.Command.Play", QStyle::SP_MediaPlay},
    TapeControl{TapeCommand::Rewind, "Emulation.Tape.Command.Rewind", QStyle::SP_MediaSeekBackward},
    TapeControl{TapeCommand::FastForward, "Emulation.Tape.Command.FastForward", QStyle::SP_MediaSeekForward},
    TapeControl{TapeCommand::Stop, "Emulation.Tape.Command.Stop", QStyle::SP_MediaStop},
    TapeControl{TapeCommand::Pause, "Emulation.Tape.Command.Pause", QStyle::SP_MediaPause}
};
constexpr std::array TapeStates{"Emulation.Tape.State.Empty", "Emulation.Tape.State.Stopped",
    "Emulation.Tape.State.Playing", "Emulation.Tape.State.Paused", "Emulation.Tape.State.Recording",
    "Emulation.Tape.State.EndOfTape"};

using namespace VideoPresentation;

constexpr std::array segmentFields{
    std::pair{"Emulation.Video.Parameter.SegmentDisplayCellSize", &SegmentDisplayProfile::cellSize},
    std::pair{"Emulation.Video.Parameter.SegmentDisplayHorizontalGap", &SegmentDisplayProfile::horizontalGap},
    std::pair{"Emulation.Video.Parameter.SegmentDisplayVerticalGap", &SegmentDisplayProfile::verticalGap},
    std::pair{"Emulation.Video.Parameter.SegmentDisplayThickness", &SegmentDisplayProfile::thickness},
    std::pair{"Emulation.Video.Parameter.SegmentDisplaySegmentGap", &SegmentDisplayProfile::segmentGap},
    std::pair{"Emulation.Video.Parameter.SegmentDisplayBrightness", &SegmentDisplayProfile::brightness},
    std::pair{"Emulation.Video.Parameter.SegmentDisplayActivationThreshold", &SegmentDisplayProfile::activationThreshold},
    std::pair{"Emulation.Video.Parameter.SegmentDisplayContrast", &SegmentDisplayProfile::contrast},
    std::pair{"Emulation.Video.Parameter.SegmentDisplayOffSegmentVisibility", &SegmentDisplayProfile::offSegmentVisibility},
    std::pair{"Emulation.Video.Parameter.SegmentDisplayBlackDepth", &SegmentDisplayProfile::blackDepth},
    std::pair{"Emulation.Video.Parameter.SegmentDisplayGlow", &SegmentDisplayProfile::glow},
    std::pair{"Emulation.Video.Parameter.SegmentDisplayHaloRadius", &SegmentDisplayProfile::haloRadius},
    std::pair{"Emulation.Video.Parameter.SegmentDisplayResponseTime", &SegmentDisplayProfile::responseTimeMilliseconds},
    std::pair{"Emulation.Video.Parameter.SegmentDisplayPersistence", &SegmentDisplayProfile::persistenceMilliseconds}
};
constexpr auto DotCellsResource = "Emulation.Video.DotMatrix.Group.Cells";
constexpr auto DotLightResource = "Emulation.Video.DotMatrix.Group.Light";
constexpr auto DotTimingResource = "Emulation.Video.DotMatrix.Group.Temporal";
constexpr size_t DotCellFields = 3, DotLightFields = 3;
constexpr std::array dotFields{
    std::pair{"Emulation.Video.Parameter.DotMatrixCellSize", &DotMatrixProfile::cellSize},
    std::pair{"Emulation.Video.Parameter.DotMatrixDotSize", &DotMatrixProfile::dotSize},
    std::pair{"Emulation.Video.Parameter.DotMatrixCellGap", &DotMatrixProfile::cellGap},
    std::pair{"Emulation.Video.Parameter.DotMatrixBrightness", &DotMatrixProfile::brightness},
    std::pair{"Emulation.Video.Parameter.DotMatrixContrast", &DotMatrixProfile::contrast},
    std::pair{"Emulation.Video.Parameter.DotMatrixHaloIntensity", &DotMatrixProfile::haloIntensity},
    std::pair{"Emulation.Video.Parameter.DotMatrixResponseTime", &DotMatrixProfile::responseTimeMilliseconds},
    std::pair{"Emulation.Video.Parameter.DotMatrixPersistence", &DotMatrixProfile::persistenceMilliseconds}
};
constexpr auto LedMatrixStructureResource = "Emulation.Video.LedMatrix.Group.Structure";
constexpr auto LedMatrixEmissionResource = "Emulation.Video.LedMatrix.Group.Emission";
constexpr size_t LedStructureFields = 2;
constexpr std::array ledFields{
    std::pair{"Emulation.Video.Parameter.LedMatrixCellSize", &LedMatrixProfile::cellSize},
    std::pair{"Emulation.Video.Parameter.LedMatrixCellGap", &LedMatrixProfile::cellGap},
    std::pair{"Emulation.Video.Parameter.LedMatrixBrightness", &LedMatrixProfile::brightness},
    std::pair{"Emulation.Video.Parameter.LedMatrixDiffusion", &LedMatrixProfile::diffusion},
    std::pair{"Emulation.Video.Parameter.LedMatrixHaloRadius", &LedMatrixProfile::haloRadius},
    std::pair{"Emulation.Video.Parameter.LedMatrixBlackDepth", &LedMatrixProfile::blackDepth}
};
constexpr auto VfdPhosphorResource = "Emulation.Video.Vfd.Group.PhosphorAndGlass";
constexpr auto VfdAnodesResource = "Emulation.Video.Vfd.Group.Anodes";
constexpr auto VfdResponseResource = "Emulation.Video.Vfd.Group.GlowAndResponse";
constexpr auto VfdColorResource = "Emulation.Video.Parameter.VfdColor";
constexpr auto VfdStructureResource = "Emulation.Video.Parameter.VfdStructure";
constexpr size_t VfdPhosphorFields = 3, VfdAnodeFields = 2;
constexpr std::array vfdFields{
    std::pair{"Emulation.Video.Parameter.VfdPhosphorIntensity", &VfdProfile::phosphorIntensity},
    std::pair{"Emulation.Video.Parameter.VfdEmissionThreshold", &VfdProfile::emissionThreshold},
    std::pair{"Emulation.Video.Parameter.VfdGlassDarkening", &VfdProfile::glassDarkening},
    std::pair{"Emulation.Video.Parameter.VfdCellSize", &VfdProfile::cellSize},
    std::pair{"Emulation.Video.Parameter.VfdCellGap", &VfdProfile::cellGap},
    std::pair{"Emulation.Video.Parameter.VfdHaloIntensity", &VfdProfile::haloIntensity},
    std::pair{"Emulation.Video.Parameter.VfdHaloRadius", &VfdProfile::haloRadius},
    std::pair{"Emulation.Video.Parameter.VfdPersistence", &VfdProfile::persistenceMilliseconds}
};
constexpr auto VectorDrawingResource = "Emulation.Video.Vector.Group.Drawing";
constexpr auto VectorGlowResource = "Emulation.Video.Vector.Group.Glow";
constexpr auto VectorPhosphorResource = "Emulation.Video.Parameter.VectorPhosphorColor";
constexpr size_t VectorDrawingFields = 4;
constexpr std::array vectorFields{
    std::pair{"Emulation.Video.Parameter.VectorLineThreshold", &VectorProfile::lineThreshold},
    std::pair{"Emulation.Video.Parameter.VectorLineIntensity", &VectorProfile::lineIntensity},
    std::pair{"Emulation.Video.Parameter.VectorBeamWidth", &VectorProfile::beamWidth},
    std::pair{"Emulation.Video.Parameter.VectorBeamFocus", &VectorProfile::beamFocus},
    std::pair{"Emulation.Video.Parameter.VectorHaloIntensity", &VectorProfile::haloIntensity},
    std::pair{"Emulation.Video.Parameter.VectorHaloRadius", &VectorProfile::haloRadius},
    std::pair{"Emulation.Video.Parameter.VectorPersistence", &VectorProfile::persistenceIntensity}
};
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
constexpr std::array segmentEndShapeLabels{"Emulation.Video.SegmentDisplay.EndShape.Beveled", "Emulation.Video.SegmentDisplay.EndShape.Rounded", "Emulation.Video.SegmentDisplay.EndShape.Straight"};
constexpr std::array segmentColorLabels{"Emulation.Video.SegmentDisplay.Color.Red", "Emulation.Video.SegmentDisplay.Color.Green", "Emulation.Video.SegmentDisplay.Color.Amber", "Emulation.Video.SegmentDisplay.Color.Blue", "Emulation.Video.SegmentDisplay.Color.White"};
constexpr std::array vectorColors{"Emulation.Video.Crt.Color.Color", "Emulation.Video.Crt.Color.Green",
    "Emulation.Video.Crt.Color.Amber", "Emulation.Video.Crt.Color.White", "Emulation.Video.Crt.Color.Gray"};
constexpr std::array inkModes{"Emulation.Video.EPaper.ColorMode.Monochrome",
    "Emulation.Video.EPaper.ColorMode.Grayscale16", "Emulation.Video.EPaper.ColorMode.Color4096"};
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

constexpr std::array vfdColors{"Emulation.Video.Vfd.Color.Blue", "Emulation.Video.Vfd.Color.Green",
    "Emulation.Video.Vfd.Color.Amber", "Emulation.Video.Vfd.Color.Red"};
constexpr std::array vfdStructures{"Emulation.Video.Vfd.Structure.Graphic", "Emulation.Video.Vfd.Structure.DotMatrix"};
constexpr std::array ledColors{"Emulation.Video.LedMatrix.Color.Rgb", "Emulation.Video.LedMatrix.Color.Red",
    "Emulation.Video.LedMatrix.Color.Green", "Emulation.Video.LedMatrix.Color.Amber", "Emulation.Video.LedMatrix.Color.Blue",
    "Emulation.Video.LedMatrix.Color.White"};
constexpr std::array ledShapes{"Emulation.Video.LedMatrix.Shape.Round", "Emulation.Video.LedMatrix.Shape.Square"};
constexpr std::array dotPalettes{"Emulation.Video.DotMatrix.Palette.Green", "Emulation.Video.DotMatrix.Palette.Gray",
    "Emulation.Video.DotMatrix.Palette.Amber", "Emulation.Video.DotMatrix.Palette.Blue", "Emulation.Video.DotMatrix.Palette.Rgb",
    "Emulation.Video.DotMatrix.Palette.Red", "Emulation.Video.DotMatrix.Palette.White"};
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

class SessionView final : public QWidget {
public:
    using OpenMachineSettings = std::function<void(QWidget*, const std::shared_ptr<Session>&)>;

    SessionView(std::shared_ptr<Session> session, std::shared_ptr<Host> host, Translation& translations,
        WorkspaceState& workspace, const VideoPresentation::ProfileStore& profiles,
        const std::shared_ptr<const ThemeData>& theme, OpenMachineSettings openMachineSettings, QWidget* parent)
        : QWidget(parent), session_(std::move(session)), host_(std::move(host)), translations_(translations),
          workspace_(workspace), profiles_(profiles),
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
        quickSave_ = commandButton(states, QuickSaveKey, Icons::Save, [this] { quickState(true); });
        quickLoad_ = commandButton(states, QuickLoadKey, Icons::Load, [this] { quickState(false); });
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
        bindCommandShortcut(Keys::QuickSave, quickSave_);
        bindCommandShortcut(Keys::QuickLoad, quickLoad_);
        bindCommandShortcut(Keys::Screenshot, screenshotButton_);
        bindCommandShortcut(Keys::ToggleFullscreen, fullscreenButton);
        bindShortcut(Keys::ReleaseMouse, [this] { releasePointer(); });
        bindCommandShortcut(Keys::ToggleMute, mute_);
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
            osdRepeat_.clear();
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
    }
    ~SessionView() override {
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

    void configureVideo() {
        using namespace VideoPresentation;
        QDialog dialog(fullScreen_ ? static_cast<QWidget*>(fullScreen_) : this);
        dialog.setWindowTitle(text(translations_, VideoKey));
        auto* container = new QVBoxLayout(&dialog);
        auto* tabs = new QTabWidget(&dialog);
        container->addWidget(tabs);
        auto* appearance = new QWidget(tabs);
        tabs->addTab(appearance, text(translations_, "Emulation.Video.Tab.Rendering"));
        auto* form = new QFormLayout(appearance);
        form->addRow(createShaderStatusRow(appearance));
        auto* renderer = new QComboBox(&dialog);
#ifdef Q_OS_WIN
        renderer->addItem(text(translations_, "Emulation.Video.Renderer.Direct3D11"), static_cast<int>(Renderer::Direct3D11));
#endif
        renderer->addItem(QStringLiteral("Qt"), static_cast<int>(Renderer::Software));
        renderer->addItem(text(translations_, "Emulation.Video.Renderer.Vulkan"), static_cast<int>(Renderer::Vulkan));
        renderer->addItem(text(translations_, "Emulation.Video.Renderer.OpenGL"), static_cast<int>(Renderer::OpenGL));
        const auto configuredRenderer = renderer->findData(static_cast<int>(profile_.renderer));
        renderer->setCurrentIndex(configuredRenderer >= 0 ? configuredRenderer : renderer->findData(static_cast<int>(Renderer::Software)));
        const auto currentRenderer = static_cast<Renderer>(renderer->currentData().toInt());
        form->addRow(text(translations_, "Emulation.Video.Settings.Rendering"), renderer);
        auto* aspect = new QComboBox(&dialog);
        auto aspectDraft = profile_.aspectRatio;
        connect(aspect, &QComboBox::activated, &dialog, [aspect, &aspectDraft] {
            aspectDraft = AspectRatio(aspect->currentData().toInt());
        });
        const auto updateAspects = [&](ScreenType screen) {
            aspect->clear();
            for (const auto ratio : aspectRatioChoices(screen)) {
                const auto dimensions = aspectRatioDimensions(ratio);
                const auto label = ratio == AspectRatio::Original
                    ? text(translations_, "Emulation.Native.AspectOriginal")
                    : QString::number(dimensions.width()) + QLatin1Char('/') + QString::number(dimensions.height());
                aspect->addItem(label, int(ratio));
            }
            const int configured = aspect->findData(int(aspectDraft));
            aspect->setCurrentIndex(configured >= 0 ? configured : aspect->findData(int(AspectRatio::Original)));
        };
        form->addRow(text(translations_, AspectKey), aspect);
        auto* sampling = new QComboBox(&dialog);
        for (const auto& [mode, label] : videoSamplers(profile_.sampling))
            sampling->addItem(text(translations_, label), static_cast<int>(mode));
        sampling->setCurrentIndex(sampling->findData(static_cast<int>(profile_.sampling)));
        form->addRow(text(translations_, SamplingKey), sampling);
        auto* blackFrame = new QCheckBox(&dialog);
        blackFrame->setChecked(profile_.blackFrameInsertion);
        form->addRow(text(translations_, BlackFrameKey), blackFrame);
        auto* deinterlacing = new QComboBox(&dialog);
        for (const auto& [mode, label] : modes) deinterlacing->addItem(text(translations_, label), static_cast<int>(mode));
        const auto configured = profile_.deinterlacing;
        const auto visibleMode = configured == Deinterlacing::BobEvenLines || configured == Deinterlacing::BobOddLines
            ? Deinterlacing::Bob : configured;
        deinterlacing->setCurrentIndex(deinterlacing->findData(static_cast<int>(visibleMode)));
        form->addRow(text(translations_, "Emulation.Video.Parameter.Deinterlacing"), deinterlacing);
        constexpr std::array levels{"Emulation.Video.Restoration.Level.None", "Emulation.Video.Restoration.Level.Light",
            "Emulation.Video.Restoration.Level.Medium", "Emulation.Video.Restoration.Level.Strong"};
        auto* dedithering = new QComboBox(&dialog);
        for (size_t index = 0; index < levels.size(); ++index)
            dedithering->addItem(text(translations_, levels[index]), ImageRestoration::DeditheringLevels[index]);
        const int level = qRound(double(profile_.restoration.dedithering) * (levels.size() - 1) / ImageRestoration::MaximumIntensity);
        dedithering->setCurrentIndex(level);
        form->addRow(text(translations_, "Emulation.Video.Parameter.Dedithering"), dedithering);
        const auto restorationValue = [&](const char* key, int value) {
            auto* spin = new QSpinBox(&dialog);
            spin->setRange(0, ImageRestoration::MaximumIntensity);
            spin->setValue(value);
            form->addRow(text(translations_, key), spin);
            return spin;
        };
        auto* debanding = restorationValue("Emulation.Video.Parameter.Debanding", profile_.restoration.debanding);
        auto* detailRecovery = restorationValue("Emulation.Video.Parameter.DetailRecovery", profile_.restoration.detailRecovery);
        detailRecovery->setToolTip(text(translations_, "Emulation.Video.Parameter.FineDetails"));
        const auto adjustment = [&](const char* key, int value) {
            auto* edit = new QSpinBox(&dialog);
            edit->setRange(ImageAdjustments::Minimum, ImageAdjustments::Maximum);
            edit->setValue(value);
            form->addRow(text(translations_, key), edit);
            return edit;
        };
        auto adjustmentDraft = profile_;
        auto adjustmentScreen = adjustmentDraft.screenType();
        const auto initialAdjustments = adjustmentDraft.imageAdjustments(adjustmentScreen);
        auto* brightness = adjustment("Emulation.Video.Parameter.Brightness", initialAdjustments.brightness);
        auto* contrast = adjustment("Emulation.Video.Parameter.Contrast", initialAdjustments.contrast);
        auto* gamma = new QComboBox(&dialog);
        for (const auto curve : ImageAdjustments::GammaCurves) {
            const auto label = curve == 0 ? text(translations_, "Emulation.Video.Gamma.Original") :
                curve == ImageAdjustments::Bt1886 ? QStringLiteral("BT.1886") : QLocale().toString(curve / 100.0, 'f', 1);
            gamma->addItem(label, curve);
        }
        gamma->setCurrentIndex(gamma->findData(initialAdjustments.gamma));
        form->addRow(text(translations_, "Emulation.Video.Gamma"), gamma);
        auto* saturation = adjustment("Emulation.Video.Parameter.Saturation", initialAdjustments.saturation);
        auto* sharpness = adjustment("Emulation.Video.Parameter.Sharpness", initialAdjustments.sharpness);
        auto* signalPage = new QWidget(tabs);
        tabs->addTab(signalPage, text(translations_, "Emulation.Video.Tab.Signal"));
        auto* signalForm = new QFormLayout(signalPage);
        const auto signalChoice = [&](const auto& choices, auto active, const char* label) {
            auto* combo = new QComboBox(signalPage);
            for (const auto& [value, key] : choices) combo->addItem(text(translations_, key), static_cast<int>(value));
            combo->setCurrentIndex(combo->findData(static_cast<int>(active)));
            signalForm->addRow(text(translations_, label), combo);
            return combo;
        };
        const auto intensity = [&](const char* key, int value) {
            auto* spin = new QSpinBox(signalPage);
            spin->setRange(0, SignalSimulation::MaximumIntensity);
            spin->setValue(value);
            signalForm->addRow(text(translations_, key), spin);
            return spin;
        };
        const auto* connection = signalChoice(connections, profile_.signal.connection, "Emulation.Video.Parameter.SignalConnection");
        const auto* connectionIntensity = intensity("Emulation.Video.Parameter.SignalConnectionIntensity", profile_.signal.connectionIntensity);
        const auto* standard = signalChoice(standards, profile_.signal.standard, "Emulation.Video.Parameter.SignalStandard");
        const auto* standardIntensity = intensity("Emulation.Video.Parameter.SignalStandardIntensity", profile_.signal.standardIntensity);
        auto* effectsPage = new QWidget(tabs);
        tabs->addTab(effectsPage, text(translations_, "Emulation.Video.Settings.Stylistic"));
        auto* effectsForm = new QFormLayout(effectsPage);
        const auto effectControl = [&](const char* key, int selected) {
            auto* spin = new QSpinBox(effectsPage);
            spin->setRange(0, StylisticEffects::MaximumIntensity);
            spin->setValue(selected);
            effectsForm->addRow(text(translations_, key), spin);
            return spin;
        };
        const auto* grain = effectControl("Emulation.Video.Parameter.Grain", profile_.stylistic.grain);
        const auto* vhs = effectControl("Emulation.Video.Parameter.Vhs", profile_.stylistic.vhs);
        const auto* aberration = effectControl("Emulation.Video.Parameter.ChromaticAberration", profile_.stylistic.chromaticAberration);
        const auto* bloom = effectControl("Emulation.Video.Parameter.Bloom", profile_.stylistic.bloom);
        auto* sepia = new QCheckBox(text(translations_, "Emulation.Video.Parameter.Sepia"), effectsPage);
        sepia->setChecked(profile_.stylistic.sepia);
        effectsForm->addRow(sepia);
        auto* crtPage = new QWidget(tabs);
        tabs->addTab(crtPage, text(translations_, "Emulation.Video.Tab.Technology"));
        auto* technologyLayout = new QVBoxLayout(crtPage);
        auto* displayType = new QComboBox(crtPage);
        for (const auto& [technology, key] : displayTypes)
            displayType->addItem(text(translations_, key), int(technology));
        displayType->setCurrentIndex(displayType->findData(int(profile_.displayTechnology())));
        technologyLayout->addWidget(displayType);
        auto* crtSettings = new QWidget(crtPage);
        technologyLayout->addWidget(crtSettings);
        auto* crtForm = new QFormLayout(crtSettings);
        const auto displayInputChoice = [&](QFormLayout* placement, const auto& options, auto selected, const char* key) {
            auto* combo = new QComboBox(placement->parentWidget());
            for (const auto& [value, label] : options) combo->addItem(text(translations_, label), static_cast<int>(value));
            combo->setCurrentIndex(combo->findData(static_cast<int>(selected)));
            placement->addRow(text(translations_, key), combo);
            return combo;
        };
        const auto crtChoice = [&](const auto& options, auto selected, const char* key) {
            return displayInputChoice(crtForm, options, selected, key);
        };
        const auto* crtmask = crtChoice(maskChoices, profile_.crt.mask, "Emulation.Video.Parameter.CrtMask");
        const auto* crtphosphor = crtChoice(phosphorChoices, profile_.crt.phosphor, "Emulation.Video.Parameter.VectorPhosphorColor");
        const auto* crtrate = crtChoice(rateChoices, profile_.crt.fieldRate, "Emulation.Video.Parameter.CrtFieldRate");
        const auto* crtdecoder = crtChoice(decoderChoices, profile_.crt.colorStandards, "Emulation.Video.Parameter.CrtColorStandards");
        auto* crtconnection = crtChoice(connectionChoices, profile_.crt.connection, "Emulation.Video.Parameter.CrtConnection");
        auto* crtPalette = crtChoice(palettes, profile_.crt.monitorPalette, "Emulation.Video.Parameter.CrtMonitorPalette");
        const auto crtValue = [&](const char* key, int value, int low, int high) {
            auto* spin = new QSpinBox(crtPage);
            spin->setRange(low, high);
            spin->setValue(value);
            crtForm->addRow(text(translations_, key), spin);
            return spin;
        };
        auto* crtDegradation = crtValue("Emulation.Video.Parameter.CrtConnectionDegradation", profile_.crt.connectionDegradation,
            0, CrtProfile::MaximumConnectionDegradation);
        const auto updateConnection = [crtconnection, crtPalette, crtDegradation] {
            const auto mode = static_cast<CrtConnection>(crtconnection->currentData().toInt());
            crtPalette->setEnabled(mode == CrtConnection::LegacyMonitor);
            crtDegradation->setEnabled(mode >= CrtConnection::Scart && mode <= CrtConnection::AnalogRf);
        };
        connect(crtconnection, &QComboBox::currentIndexChanged, &dialog, updateConnection);
        updateConnection();
        const auto* crtBeam = crtValue("Emulation.Video.Parameter.VectorBeamWidth", profile_.crt.beamWidth,
            ImageAdjustments::Minimum, ImageAdjustments::Maximum);
        const auto* crtCurvature = crtValue("Emulation.Video.Parameter.CrtCurvature", profile_.crt.curvature, 0, CrtProfile::MaximumCurvature);
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
        const auto segmentGroup = [&](const char* key, int row, int column) {
            auto* group = new QGroupBox(text(translations_, key), segmentSettings);
            segmentGrid->addWidget(group, row, column);
            return new QFormLayout(group);
        };
        auto* segmentCells = segmentGroup("Emulation.Video.SegmentDisplay.Group.Cells", 0, 0);
        auto* segmentGeometry = segmentGroup("Emulation.Video.SegmentDisplay.Group.Geometry", 0, 1);
        auto* segmentEmission = segmentGroup("Emulation.Video.SegmentDisplay.Group.Emission", 1, 0);
        auto* segmentLightAndResponse = segmentGroup("Emulation.Video.SegmentDisplay.Group.LightAndResponse", 1, 1);
        const auto segmentChoice = [&](QFormLayout* placement, const auto& labels, int selected, const char* key) {
            auto* combo = new QComboBox(placement->parentWidget());
            for (const auto* label : labels) combo->addItem(text(translations_, label));
            combo->setCurrentIndex(selected);
            placement->addRow(text(translations_, key), combo);
            return combo;
        };
        const auto addTechnologyGroup = [&](QGridLayout* grid, const char* key, int row, int column, int span = 1) {
            auto* group = new QGroupBox(text(translations_, key), grid->parentWidget());
            grid->addWidget(group, row, column, 1, span);
            return new QFormLayout(group);
        };
        const auto addTechnologyValue = [&](QFormLayout* form, const char* key, int value, int maximum) {
            auto* field = new QSpinBox(form->parentWidget());
            field->setRange(0, maximum);
            field->setValue(value);
            form->addRow(text(translations_, key), field);
            return field;
        };
        auto* ePaperGrid = new QGridLayout(ePaperSettings);
        auto* ink = addTechnologyGroup(ePaperGrid, "Emulation.Video.EPaper.Group.InkAndColor", 0, 0);
        auto* paper = addTechnologyGroup(ePaperGrid, "Emulation.Video.EPaper.Group.PaperSurface", 0, 1);
        auto* refresh = addTechnologyGroup(ePaperGrid, "Emulation.Video.EPaper.Group.Refresh", 1, 0, 2);
        const auto* inkMode = segmentChoice(ink, inkModes, int(profile_.ePaper.colorMode), "Emulation.Video.Parameter.EPaperColorMode");
        auto* projectionForm = new QFormLayout(projectionSettings);
        const auto* ePaperInkDensity = addTechnologyValue(ink, "Emulation.Video.Parameter.EPaperInkDensity",
            profile_.ePaper.inkDensity, EPaperProfile::MaximumIntensity);
        const auto* ePaperContrast = addTechnologyValue(ink, "Emulation.Video.Parameter.EPaperContrast",
            profile_.ePaper.contrast, EPaperProfile::MaximumIntensity);
        const auto* ePaperDithering = addTechnologyValue(ink, "Emulation.Video.Parameter.EPaperDithering",
            profile_.ePaper.dithering, EPaperProfile::MaximumIntensity);
        auto* ePaperColorSaturation = addTechnologyValue(ink, "Emulation.Video.Parameter.EPaperColorSaturation",
            profile_.ePaper.colorSaturation, EPaperProfile::MaximumIntensity);
        const auto* ePaperPaperBrightness = addTechnologyValue(paper, "Emulation.Video.Parameter.EPaperPaperBrightness",
            profile_.ePaper.paperBrightness, EPaperProfile::MaximumIntensity);
        const auto* ePaperPaperWarmth = addTechnologyValue(paper, "Emulation.Video.Parameter.EPaperPaperWarmth",
            profile_.ePaper.paperWarmth, EPaperProfile::MaximumIntensity);
        const auto* ePaperSurfaceTexture = addTechnologyValue(paper, "Emulation.Video.Parameter.EPaperSurfaceTexture",
            profile_.ePaper.surfaceTexture, EPaperProfile::MaximumIntensity);
        const auto* ePaperEdgeSoftness = addTechnologyValue(paper, "Emulation.Video.Parameter.EPaperEdgeSoftness",
            profile_.ePaper.edgeSoftness, EPaperProfile::MaximumIntensity);
        const auto* ePaperRefreshTime = addTechnologyValue(refresh, "Emulation.Video.Parameter.EPaperRefreshTime",
            profile_.ePaper.refreshTimeMilliseconds, EPaperProfile::MaximumDurationMilliseconds);
        const auto* ePaperGhosting = addTechnologyValue(refresh, "Emulation.Video.Parameter.EPaperGhosting",
            profile_.ePaper.ghosting, EPaperProfile::MaximumIntensity);
        const auto* projectionTechnology = segmentChoice(projectionForm, projectionTechnologies,
            int(profile_.projection.technology), ProjectionTechnologyResource);
        const auto* projectionSurface = segmentChoice(projectionForm, projectionSurfaces,
            int(profile_.projection.surface), ProjectionSurfaceResource);
        const auto* projectionAmbientLight = addTechnologyValue(projectionForm, "Emulation.Video.Parameter.ProjectionAmbientLight",
            profile_.projection.ambientLight, ProjectionProfile::MaximumAmbientLight);
        const auto updateInkSaturation = [=](int index) {
            ink->setRowVisible(ePaperColorSaturation, index == int(EPaperColorMode::Color4096));
        };
        QObject::connect(inkMode, &QComboBox::currentIndexChanged, ePaperSettings, updateInkSaturation);
        updateInkSaturation(inkMode->currentIndex());
        const auto* segmentLayoutChoice = segmentChoice(segmentCells, segmentLayoutLabels,
            int(profile_.segmentDisplay.layout), "Emulation.Video.Parameter.SegmentDisplayLayout");
        const auto* segmentEndShapeChoice = segmentChoice(segmentGeometry, segmentEndShapeLabels,
            int(profile_.segmentDisplay.endShape), "Emulation.Video.Parameter.SegmentDisplayEndShape");
        const auto* segmentColorChoice = segmentChoice(segmentEmission, segmentColorLabels,
            int(profile_.segmentDisplay.color), "Emulation.Video.Parameter.SegmentDisplayColor");
        auto* decimalPoint = new QCheckBox(text(translations_, "Emulation.Video.Parameter.SegmentDisplayDecimalPoint"), segmentSettings);
        auto* colon = new QCheckBox(text(translations_, "Emulation.Video.Parameter.SegmentDisplayColon"), segmentSettings);
        decimalPoint->setChecked(profile_.segmentDisplay.decimalPoint);
        colon->setChecked(profile_.segmentDisplay.colon);
        segmentGeometry->addRow(decimalPoint);
        segmentGeometry->addRow(colon);
        const std::array segmentGroups{segmentCells, segmentCells, segmentCells, segmentGeometry, segmentGeometry, segmentEmission, segmentEmission, segmentEmission, segmentEmission, segmentEmission, segmentLightAndResponse, segmentLightAndResponse, segmentLightAndResponse, segmentLightAndResponse};
        std::array<QSpinBox*, segmentFields.size()> segmentControls{};
        for (size_t index = 0; index < segmentFields.size(); ++index) {
            const auto& [key, field] = segmentFields[index];
            auto* placement = segmentGroups[index];
            auto* spin = new QSpinBox(placement->parentWidget());
            const bool duration = field == &SegmentDisplayProfile::responseTimeMilliseconds ||
                field == &SegmentDisplayProfile::persistenceMilliseconds;
            spin->setRange(0, duration ? SegmentDisplayProfile::MaximumDurationMilliseconds : SegmentDisplayProfile::MaximumIntensity);
            spin->setValue(profile_.segmentDisplay.*field);
            placement->addRow(text(translations_, key), spin);
            segmentControls[index] = spin;
        }
        const auto displayGroups = [](QWidget* parent) {
            auto* layout = new QHBoxLayout(parent);
            layout->setContentsMargins(0, 0, 0, 0);
            return layout;
        };
        const auto displayGroup = [&](QHBoxLayout* layout, const char* key) {
            auto* group = new QGroupBox(text(translations_, key), layout->parentWidget());
            layout->addWidget(group, 1);
            return new QFormLayout(group);
        };
        const auto displayChoice = [&](QFormLayout* place, const auto& labels, int selected, const char* key) {
            auto* control = new QComboBox(place->parentWidget());
            for (const auto* label : labels) control->addItem(text(translations_, label));
            control->setCurrentIndex(selected);
            place->addRow(text(translations_, key), control);
            return control;
        };
        const auto displaySlider = [&](QFormLayout* place, const char* key, int value, int maximum) {
            auto* row = new QWidget(place->parentWidget());
            auto* rowLayout = new QHBoxLayout(row);
            rowLayout->setContentsMargins(0, 0, 0, 0);
            auto* control = new QSlider(Qt::Horizontal, row);
            control->setRange(0, maximum);
            control->setValue(value);
            if (maximum > FixedPixelProfile::PercentMaximum) {
                constexpr int DurationStepMilliseconds = 5;
                control->setSingleStep(DurationStepMilliseconds);
                connect(control, &QSlider::actionTriggered, control, [control] {
                    const int position = control->sliderPosition();
                    control->setSliderPosition(
                        (position + DurationStepMilliseconds / 2) / DurationStepMilliseconds * DurationStepMilliseconds);
                });
            }
            auto* number = new QLabel(QLocale().toString(control->value()), row);
            number->setMinimumWidth(number->fontMetrics().horizontalAdvance(QLocale().toString(maximum)));
            number->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            connect(control, &QSlider::valueChanged, number, [number](int current) {
                number->setText(QLocale().toString(current));
            });
            rowLayout->addWidget(control, 1);
            rowLayout->addWidget(number);
            auto* label = new QLabel(text(translations_, key), place->parentWidget());
            label->setBuddy(control);
            control->setAccessibleName(label->text());
            place->addRow(label, row);
            return control;
        };

        auto* dotLayout = displayGroups(dotSettings);
        auto* dotCells = displayGroup(dotLayout, DotCellsResource);
        auto* dotLight = displayGroup(dotLayout, DotLightResource);
        auto* dotTiming = displayGroup(dotLayout, DotTimingResource);
        const auto* dotPalette = displayChoice(dotLight, dotPalettes, int(profile_.dotMatrix.palette), "Emulation.Video.Parameter.DotMatrixPalette");
        const auto* dotShape = displayChoice(dotCells, dotShapes, int(profile_.dotMatrix.shape), "Emulation.Video.Parameter.DotMatrixShape");
        const std::array dotGroups{dotCells, dotCells, dotCells, dotLight, dotLight, dotLight, dotTiming, dotTiming};
        std::array<QSlider*, dotFields.size()> dotControls{};
        for (size_t index = 0; index < dotFields.size(); ++index) {
            const auto& [key, field] = dotFields[index];
            auto* groupForm = dotGroups[index];
            dotControls[index] = displaySlider(groupForm, key, profile_.dotMatrix.*field,
                groupForm == dotTiming ? DotMatrixProfile::MaximumDurationMilliseconds : DotMatrixProfile::MaximumIntensity);
        }

        auto* ledLayout = displayGroups(ledSettings);
        auto* ledStructure = displayGroup(ledLayout, LedMatrixStructureResource);
        auto* ledEmission = displayGroup(ledLayout, LedMatrixEmissionResource);
        const auto* ledShape = displayChoice(ledStructure, ledShapes, int(profile_.ledMatrix.shape), "Emulation.Video.Parameter.LedMatrixShape");
        const auto* ledColor = displayChoice(ledEmission, ledColors, int(profile_.ledMatrix.color), "Emulation.Video.Parameter.LedMatrixColor");
        std::array<QSlider*, ledFields.size()> ledControls{};
        for (size_t index = 0; index < ledFields.size(); ++index) {
            const auto& [key, field] = ledFields[index];
            auto* groupForm = index < LedStructureFields ? ledStructure : ledEmission;
            ledControls[index] = displaySlider(groupForm, key, profile_.ledMatrix.*field, LedMatrixProfile::MaximumIntensity);
        }

        auto* vfdLayout = displayGroups(vfdSettings);
        auto* vfdPhosphor = displayGroup(vfdLayout, VfdPhosphorResource);
        auto* vfdAnodes = displayGroup(vfdLayout, VfdAnodesResource);
        auto* vfdResponse = displayGroup(vfdLayout, VfdResponseResource);
        const auto* vfdColor = displayChoice(vfdPhosphor, vfdColors, int(profile_.vfd.color), VfdColorResource);
        auto* vfdStructure = displayChoice(vfdAnodes, vfdStructures, int(profile_.vfd.structure), VfdStructureResource);
        const std::array vfdGroups{vfdPhosphor, vfdPhosphor, vfdPhosphor, vfdAnodes, vfdAnodes, vfdResponse, vfdResponse, vfdResponse};
        std::array<QSlider*, vfdFields.size()> vfdControls{};
        for (size_t index = 0; index < vfdFields.size(); ++index) {
            const auto& [key, field] = vfdFields[index];
            const int maximum = field == &VfdProfile::persistenceMilliseconds ? VfdProfile::MaximumPersistenceMilliseconds :
                VfdProfile::MaximumIntensity;
            vfdControls[index] = displaySlider(vfdGroups[index], key, profile_.vfd.*field, maximum);
        }
        const auto updateVfdCells = [=] {
            const bool enabled = vfdStructure->currentIndex() == int(VfdStructure::DotMatrix);
            for (auto* control : std::span(vfdControls).subspan<VfdPhosphorFields, VfdAnodeFields>()) {
                auto* row = control->parentWidget();
                row->setEnabled(enabled);
                vfdAnodes->labelForField(row)->setEnabled(enabled);
            }
        };
        connect(vfdStructure, &QComboBox::currentIndexChanged, &dialog, updateVfdCells);
        updateVfdCells();

        auto* vectorGroups = displayGroups(vectorSettings);
        auto* vectorDrawingForm = displayGroup(vectorGroups, VectorDrawingResource);
        auto* vectorGlowForm = displayGroup(vectorGroups, VectorGlowResource);
        std::array<QSlider*, vectorFields.size()> vectorControls{};
        for (size_t index = 0; index < vectorFields.size(); ++index) {
            const auto& [key, field] = vectorFields[index];
            auto* groupForm = index < VectorDrawingFields ? vectorDrawingForm : vectorGlowForm;
            vectorControls[index] = displaySlider(groupForm, key, profile_.vector.*field, VectorProfile::MaximumIntensity);
        }
        auto* vectorPhosphor = displayChoice(vectorDrawingForm, vectorColors, int(profile_.vector.phosphorColor), VectorPhosphorResource);
        auto* plasmaForm = new QFormLayout(plasmaSettings);
        auto* plasmaStructure = displayInputChoice(plasmaForm, plasmaCellChoices, profile_.plasma.cellStructure,
            PlasmaCellStructureResource);
        auto* fixedForm = new QFormLayout(fixedSettings);
        const auto fixedChoice = [&](const auto& labels, int selected, const char* key) {
            auto* combo = new QComboBox(fixedSettings);
            for (const auto* label : labels) combo->addItem(text(translations_, label));
            combo->setCurrentIndex(selected);
            fixedForm->addRow(text(translations_, key), combo);
            return combo;
        };
        auto fixedProfiles = profile_.fixedPixelProfiles;
        auto fixedDraft = profile_.fixedPixel;
        auto lcd = profile_.lcd;
        auto ledBacklight = profile_.ledBacklight;
        auto* fixedType = fixedChoice(fixedTypes, int(fixedDraft.technology), "Emulation.Video.Parameter.FixedPixelTechnology");
        auto* lcdStructure = fixedChoice(lcdStructures, int(lcd.pixelStructure), LcdPixelStructureResource);
        auto* lcdOrientation = fixedChoice(lcdOrientations, int(lcd.orientation), LcdOrientationResource);
        auto* lcdPanel = fixedChoice(lcdPanelTypes, int(lcd.panelType), LcdPanelTypeResource);
        auto* backlight = addTechnologyValue(fixedForm, "Emulation.Video.Parameter.FixedPixelBacklight",
            lcd.intensity, FixedPixelProfile::PercentMaximum);
        auto* ledBacklightType = fixedChoice(ledBacklightTypes, int(ledBacklight.type), LedBacklightTypeResource);
        auto* oledPanel = fixedChoice(oledPanels, int(profile_.oled.panelType), LcdPanelTypeResource);
        auto* oledStructure = fixedChoice(oledStructures, int(profile_.oled.pixelStructure), LcdPixelStructureResource);
        const auto currentLcd = [&]() -> LcdProfile& {
            return fixedDraft.technology == FixedPixelTechnology::LedBacklitLcd ? static_cast<LcdProfile&>(ledBacklight) : lcd;
        };
        const auto showOrientation = [&] {
            const auto structure = LcdPixelStructure(lcdStructure->currentIndex());
            fixedForm->setRowVisible(lcdOrientation, fixedDraft.technology != FixedPixelTechnology::Oled &&
                (structure == LcdPixelStructure::Rgb || structure == LcdPixelStructure::Bgr));
        };
        connect(lcdStructure, &QComboBox::currentIndexChanged, &dialog, showOrientation);
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
        connect(fixedType, &QComboBox::currentIndexChanged, &dialog, [&](int index) {
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
        connect(displayType, &QComboBox::currentIndexChanged, &dialog, showTechnology);
        showTechnology();
        using Adjustment = ImageAdjustmentControl;
        auto* panelSettings = new QGroupBox(text(translations_, "Emulation.Video.Tab.Signal"), crtPage);
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
        connect(panelConnection, &QComboBox::currentIndexChanged, &dialog, updatePanelFields);
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
                            panelDefinition->addItem(text(translations_, key), int(definition));
                    };
                    if (adjustmentScreen == ScreenType::Plasma) addDefinition(plasmaDefinitions);
                    else addDefinition(panelDefinitionChoices);
                    panelDefinition->setCurrentIndex(std::max(0, panelDefinition->findData(int(displayedSignal->definition))));
                }
                {
                    const QSignalBlocker block(panelConnection);
                    panelConnection->clear();
                    for (const auto& [connection, key] : orderedChoices(connectionChoices, displayConnectionChoices(adjustmentScreen)))
                        panelConnection->addItem(text(translations_, key), int(connection));
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
            form->setRowVisible(dedithering, adjustmentDraft.supportsDedithering());
            form->setRowVisible(deinterlacing, adjustmentDraft.supportsDeinterlacing());
            form->setRowVisible(debanding, adjustmentDraft.usesHostDisplay());
            form->setRowVisible(detailRecovery, adjustmentDraft.usesHostDisplay());
            form->setRowVisible(brightness, adjustmentDraft.showsImageAdjustment(Adjustment::Brightness));
            form->setRowVisible(contrast, adjustmentDraft.showsImageAdjustment(Adjustment::Contrast));
            form->setRowVisible(gamma, adjustmentDraft.showsImageAdjustment(Adjustment::Gamma));
            form->setRowVisible(saturation, adjustmentDraft.showsImageAdjustment(Adjustment::Saturation));
            form->setRowVisible(sharpness, adjustmentDraft.showsImageAdjustment(Adjustment::Sharpness));
        };
        for (const auto* control : std::array<const QComboBox*, 4>{displayType, fixedType, ledColor, dotPalette})
            connect(control, &QComboBox::currentIndexChanged, &dialog, updateAdjustments);
        updateAdjustments();
        auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
        buttons->button(QDialogButtonBox::Save)->setText(text(translations_, SaveKey));
        buttons->button(QDialogButtonBox::Cancel)->setText(text(translations_, CloseKey));
        container->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        if (dialog.exec() != QDialog::Accepted) return;
        auto updated = profile_;
        const auto chosenRenderer = static_cast<Renderer>(renderer->currentData().toInt());
        if (chosenRenderer != currentRenderer) updated.renderer = chosenRenderer;
        updated.aspectRatio = aspectDraft;
        updated.sampling = static_cast<Sampling>(sampling->currentData().toInt());
        updated.blackFrameInsertion = blackFrame->isChecked();
        const auto mode = static_cast<Deinterlacing>(deinterlacing->currentData().toInt());
        if (mode != visibleMode) updated.deinterlacing = mode;
        updated.restoration = {dedithering->currentIndex() == level ? profile_.restoration.dedithering : dedithering->currentData().toInt(),
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
        updated.ePaper.inkDensity = ePaperInkDensity->value();
        updated.ePaper.contrast = ePaperContrast->value();
        updated.ePaper.dithering = ePaperDithering->value();
        updated.ePaper.colorSaturation = ePaperColorSaturation->value();
        updated.ePaper.paperBrightness = ePaperPaperBrightness->value();
        updated.ePaper.paperWarmth = ePaperPaperWarmth->value();
        updated.ePaper.surfaceTexture = ePaperSurfaceTexture->value();
        updated.ePaper.edgeSoftness = ePaperEdgeSoftness->value();
        updated.ePaper.refreshTimeMilliseconds = ePaperRefreshTime->value();
        updated.ePaper.ghosting = ePaperGhosting->value();
        updated.projection.technology = ProjectionTechnology(projectionTechnology->currentIndex());
        updated.projection.surface = ProjectionSurface(projectionSurface->currentIndex());
        updated.projection.ambientLight = projectionAmbientLight->value();
        updated.segmentDisplay.decimalPoint = decimalPoint->isChecked();
        updated.segmentDisplay.colon = colon->isChecked();
        updated.segmentDisplay.layout = SegmentLayout(segmentLayoutChoice->currentIndex());
        updated.segmentDisplay.endShape = SegmentEndShape(segmentEndShapeChoice->currentIndex());
        updated.segmentDisplay.color = SegmentColor(segmentColorChoice->currentIndex());
        for (size_t index = 0; index < segmentFields.size(); ++index)
            updated.segmentDisplay.*(segmentFields[index].second) = segmentControls[index]->value();
        updated.dotMatrix.palette = DotMatrixPalette(dotPalette->currentIndex());
        updated.dotMatrix.shape = DotMatrixShape(dotShape->currentIndex());
        for (size_t index = 0; index < dotFields.size(); ++index)
            updated.dotMatrix.*(dotFields[index].second) = dotControls[index]->value();
        updated.ledMatrix.color = LedMatrixColor(ledColor->currentIndex());
        updated.ledMatrix.shape = LedMatrixShape(ledShape->currentIndex());
        for (size_t index = 0; index < ledFields.size(); ++index)
            updated.ledMatrix.*(ledFields[index].second) = ledControls[index]->value();
        updated.stylistic = {grain->value(), vhs->value(), aberration->value(), bloom->value(), sepia->isChecked()};
        updated.vfd.color = VfdColor(vfdColor->currentIndex());
        updated.vfd.structure = VfdStructure(vfdStructure->currentIndex());
        for (size_t index = 0; index < vfdFields.size(); ++index)
            updated.vfd.*(vfdFields[index].second) = vfdControls[index]->value();
        for (size_t index = 0; index < vectorFields.size(); ++index)
            updated.vector.*(vectorFields[index].second) = vectorControls[index]->value();
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
        if (profiles_.save(session_->moduleId(), session_->configuration().id, updated)) {
            profile_ = std::move(updated);
            surface_->setProfile(profile_);
            surface_->setIndicator({});
        } else {
            surface_->setIndicator(text(translations_, ErrorKey));
        }
    }
protected:
    bool eventFilter(QObject* object, QEvent* event) override {
        const auto type = event->type();
        if (shaderBadge_ && shaderBadge_->isVisible() && (object == surface_ || object == surface_->window()) &&
            (type == QEvent::Move || type == QEvent::Resize))
            shaderBadge_->centerOn(*surface_);
        const bool buttonEvent = type == QEvent::MouseButtonPress || type == QEvent::MouseButtonRelease
            || type == QEvent::MouseButtonDblClick;
        const bool keyEvent = type == QEvent::KeyPress || type == QEvent::KeyRelease || type == QEvent::ShortcutOverride;
        if (buttonEvent || type == QEvent::Wheel || keyEvent) {
            const auto* widget = qobject_cast<QWidget*>(object);
            const bool inputTarget = widget && (widget == surface_ || surface_->isAncestorOf(widget) ||
                (osd_ && (widget == osd_ || osd_->isAncestorOf(widget))));
            if (inputTarget && keyEvent && trackPhysicalKey(*static_cast<QKeyEvent*>(event))) return true;
            if (inputTarget && !keyEvent) {
                const auto buttons = buttonEvent ? static_cast<QMouseEvent*>(event)->buttons()
                    : static_cast<QWheelEvent*>(event)->buttons();
                const auto wheel = buttonEvent ? QPoint{} : static_cast<QWheelEvent*>(event)->angleDelta();
                if (dispatchPhysicalShortcuts(buttons, wheel)) return true;
            }
        }
        if (object != surface_) return QWidget::eventFilter(object, event);
        if (type == QEvent::FocusOut || type == QEvent::Hide) {
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
        shaderBadge_->setCaption(caption);
        shaderBadge_->centerOn(*surface_);
        shaderBadge_->show();
        shaderBadge_->raise();
    }

    void localize() {
        if (surface_->isShaderLoading()) updateShaderBadge();
        updateShaderStatusRows();
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
        entry.primary = Physical::chord(stored.isUndefined() && direction
            ? QKeySequence(entry.binding->defaultKey).toString(QKeySequence::PortableText) : stored.toString(), direction);
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
        connect(shortcut, &QShortcut::activated, this, [this, action = std::move(action)] {
            if (surface_->hasFocus() || osd_) action();
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
        const bool fullscreenOnly = binding == &OsdWithPause || binding == &OsdWithoutPause || binding == &OsdStatistics;
        return entry.keyboard->isEnabled() && (!fullscreenOnly || fullScreen_);
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
        for (auto& entry : configuredShortcuts_) {
            const bool pressed = Physical::pressed(entry.primary, current) || Physical::pressed(entry.alternative, current);
            const bool available = shortcutAvailable(entry);
            if (!action && physicalShortcutsPrimed_ && available && pressed && !entry.held)
                action = entry.action;
            entry.held = Physical::pressed(entry.primary, held) || Physical::pressed(entry.alternative, held);
            if (!repeating && editing && available && entry.held && EmulationKeyboardShortcuts::isOsdDirection(*entry.binding))
                repeating = &entry;
        }
        if (osdRepeat_.due(repeating ? repeating->binding : nullptr, Physical::OsdDirectionRepeat::Clock::now()) && !action)
            action = repeating->action;
        physicalShortcutsPrimed_ = true;
        if (!action) return false;
        const QScopedValueRollback dispatching(dispatchingPhysicalShortcut_, true);
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

    void sendInput() {
        try {
            auto& machine = session_->machine();
            machine.setInput(input_);
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
        shortcutModifiers_.clear();
        osdRepeat_.clear();
        if (!input_.pointerCaptured && input_.keys.isEmpty() && input_.pointerButtons == 0 && input_.controllers.isEmpty() && rawControllerInput_.isEmpty()) return;
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
            osdRepeat_.clear();
            lastGamepads_.clear();
            gamepadSamples_.clear();
            midiSamples_.clear();
            if (!input_.controllers.isEmpty() || !rawControllerInput_.isEmpty()) {
                input_.controllers.clear();
                rawControllerInput_.clear();
                sendInput();
            }
            return;
        }

        if (!gamepadSource_) gamepadSource_ = std::make_unique<Platform::GamepadSource>();
        gamepadSamples_ = gamepadSource_->read();
        if (midiInput_) midiSamples_ = midiInput_->read();
        QHash<quint64, Platform::GamepadState> latest;
        QList<InputSnapshot::Controller> forwarded;
        QList<RawControllerState> raw;
        for (const auto& current : gamepadSamples_) {
            latest.insert(current.id, current);
            if (osd_) continue;

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
            if (latest.contains(held.key())) ++held;
            else held = heldGamepads_.erase(held);
        }
        lastGamepads_ = std::move(latest);
        if (input_.controllers != forwarded || rawControllerInput_ != raw) {
            input_.controllers = std::move(forwarded);
            rawControllerInput_ = std::move(raw);
            sendInput();
        }
        dispatchPhysicalShortcuts(QApplication::mouseButtons());
    }

    template<class Operation>
    void run(Operation operation) {
        const auto previous = session_->runtimeRevision();
        try {
            surface_->setIndicator(operation() ? QString{} : text(translations_, ErrorKey));
        } catch (...) {
            surface_->setIndicator(text(translations_, ErrorKey));
        }
        if (previous != session_->runtimeRevision()) {
            executionRate_.reset();
            for (auto& row : rows_) row.activeUntil = {};
            session_->stopAudio();
            sendInput();
        }
        poll();
        refreshMedia();
        refreshQuickStates();
    }
    void refreshQuickStates() {
        try {
            quickSave_->setEnabled(session_->canQuickSave());
            quickLoad_->setEnabled(session_->canQuickLoad());
        } catch (...) {
            quickSave_->setEnabled(false);
            quickLoad_->setEnabled(false);
        }
    }
    void quickState(bool save) {
        run([this, save] {
            if (save) return !session_->canQuickSave() || session_->quickSave();
            return !session_->canQuickLoad() || session_->quickLoad();
        });
    }
    void stateFile(bool save) {
        const auto title = text(translations_, save ? SaveStateKey : LoadStateKey);
        auto* owner = fullScreen_ ? static_cast<QWidget*>(fullScreen_) : this;
        const auto directory = resolveEmulationFolders(workspace_.emulationFolders()).states;
        const auto path = RememberedFileDialog::choose(owner, workspace_, title, QLatin1String(EmulationStateScope),
            directory, save ? QFileDialog::AnyFile : QFileDialog::ExistingFile, save);
        if (!path.isEmpty()) run([&] { return save ? session_->machine().saveState(path, {}) : session_->machine().loadState(path, {}); });
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
            else run([&] { return session_->insertMedia({slot, *selected, false}); });
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
        run([&] { return session_->insertMedia({slot, file.absoluteFilePath(), protect}); });
    }
    void ejectMedia(const MediaSlot& slot) {
        run([&] { return session_->ejectMedia(slot); });
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
        run([&] { return session_->recordTape(slot, file.absoluteFilePath(), pcm); });
    }
    void rebuildMedia(const QList<MediaSlot>& devices) {
        while (auto* item = mediaLayout_->takeAt(0)) {
            if (auto* widget = item->widget()) { widget->hide(); widget->deleteLater(); }
            delete item;
        }
        rows_.clear();
        for (const auto& slot : devices) {
            auto* group = new QWidget(mediaStrip_);
            auto* layout = new QVBoxLayout(group);
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
                row.eject->setIcon(style()->standardIcon(QStyle::SP_ArrowUp));
                connect(row.eject, &QToolButton::clicked, this, [this, slot] { ejectMedia(slot); });
                heading->addWidget(row.eject);
            }
            layout->addLayout(heading);
            if (slot.kind == MediaKind::Tape) {
                row.canRecord = !host_->tapeRecordingExtensions(slot).isEmpty();
                auto* transport = new QHBoxLayout;
                for (size_t i = 0; i < TapeControls.size(); ++i) {
                    const auto& definition = TapeControls[i];
                    auto* button = new QToolButton(group);
                    button->setIcon(style()->standardIcon(definition.icon));
                    button->setCheckable(true);
                    connect(button, &QToolButton::clicked, this, [this, slot, command = definition.command] {
                        if (command == TapeCommand::Record) { recordTape(slot); return; }
                        run([&] { return session_->machine().tape(command, {}); });
                    });
                    row.tape[i] = button;
                    transport->addWidget(button);
                }
                layout->addLayout(transport);
            }
            rows_.push_back(row);
            mediaLayout_->addWidget(group);
        }
        mediaLayout_->addStretch();
        mediaStrip_->setVisible(!devices.isEmpty());
        devices_ = devices;
    }
    void refreshMedia() {
        try {
            auto& machine = session_->machine();
            if (osd_) osd_->setGameName(osdGameName());
            const auto devices = machine.mediaSlots();
            const auto& configuration = session_->configuration();
            if (devices != devices_ || configuration.values != mediaConfigurationValues_) {
                mediaStorage_ = host_->storageSettings(session_->moduleId(), configuration);
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
                        const bool play = command == TapeCommand::Play;
                        const bool record = command == TapeCommand::Record;
                        const bool hold = tape.state == TapeState::Paused;
                        const bool currentOperation = tape.activeOperation == command ||
                            (!tape.activeOperation && ((play && tape.state == TapeState::Playing) ||
                                (record && tape.state == TapeState::Recording)));
                        const bool playingOrRecording = currentOperation &&
                            ((play && (hold || tape.state == TapeState::Playing)) ||
                                (record && (hold || tape.state == TapeState::Recording)));
                        const bool active = playingOrRecording || (hold && command == TapeCommand::Pause) ||
                            (currentOperation && (command == TapeCommand::Rewind || command == TapeCommand::FastForward));
                        const bool blinks = hold && playingOrRecording;
                        blinking |= blinks;
                        auto* button = row.tape[buttonIndex];
                        button->setEnabled(powered && tape.availableCommands.contains(command) &&
                            (tape.state != TapeState::Empty || record) && (!record || row.canRecord) && !playingOrRecording);
                        button->setChecked(active && (!blinks || tapeBlinkVisible_));
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
    void screenshot() {
        const auto name = saveEmulationScreenshot(surface_->currentImage(), resolveEmulationFolders(workspace_.emulationFolders()).captures);
        surface_->setIndicator(name.isEmpty() ? text(translations_, ErrorKey) : name);
    }
    void toggleFullscreenStatistics() {
        run([this] {
            workspace_.setShowFullscreenEmulationStatistics(!workspace_.showFullscreenEmulationStatistics());
            return true;
        });
    }
    bool saveOsdVideo(Profile draft, bool previewed) {
        if (!profiles_.save(session_->moduleId(), session_->configuration().id, draft)) {
            surface_->setIndicator(text(translations_, ErrorKey));
            return false;
        }
        profile_ = std::move(draft);
        if (!previewed) surface_->setProfile(profile_);
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
                [this, withValue, preview](int value) { return saveOsdVideo(withValue(value), preview); },
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
            entry.enabled = [] { return false; };
            entry.visible = std::move(visible);
            rows.push_back(std::move(entry));
        };
        const auto signedValue = [](int value) {
            const QLocale locale;
            return (value > 0 ? locale.positiveSign() : QString{}) + locale.toString(value);
        };
        const auto toggleSetting = [&](const char* key, auto read, auto write, std::function<bool()> visible = {}) {
            add(key, [read](const Profile& p) { return int(std::invoke(read, p)); },
                [write](Profile& p, int value) { std::invoke(write, p, value != 0); },
                [this](int value) { return text(translations_, value ? "Emulation.Value.Enabled" : "Emulation.Value.Disabled"); },
                0, 1, false, std::move(visible), false);
        };
        const auto hostDisplay = [this] { return profile_.usesHostDisplay(); };
        section("Emulation.Video.Settings.RetroMediaLabProcessing");
        choices("Emulation.Video.Technology", osdScreenTypes, &Profile::screenType, &Profile::selectScreen);
        constexpr std::array renderers{
#ifdef Q_OS_WIN
            std::pair{Renderer::Direct3D11, "Emulation.Video.Renderer.Direct3D11"},
#endif
            std::pair{Renderer::Software, "Qt"}, std::pair{Renderer::Vulkan, "Emulation.Video.Renderer.Vulkan"},
            std::pair{Renderer::OpenGL, "Emulation.Video.Renderer.OpenGL"}
        };
        choices("Emulation.Video.Settings.Rendering", renderers, &Profile::renderer,
            [](Profile& p, Renderer value) { p.renderer = value; });
        choices(SamplingKey, videoSamplers(profile_.sampling), &Profile::sampling,
            [](Profile& p, Sampling value) { p.sampling = value; }, hostDisplay);
        toggleSetting(BlackFrameKey, &Profile::blackFrameInsertion,
            [](Profile& p, bool value) { p.blackFrameInsertion = value; });
        using Adjustment = ImageAdjustmentControl;
        const auto showsAdjustments = [this] {
            constexpr std::array controls{Adjustment::Brightness, Adjustment::Contrast, Adjustment::Gamma,
                Adjustment::Saturation, Adjustment::Sharpness};
            return std::ranges::any_of(controls, [this](Adjustment control) { return profile_.showsImageAdjustment(control); });
        };
        for (const auto& [screen, label] : osdScreenTypes)
            section(label, [current = activeScreen(screen), showsAdjustments] { return current() && showsAdjustments(); });
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
        constexpr size_t AdjustmentsBeforeGamma = 2;
        const auto adjustmentList = std::span(adjustments);
        for (const auto& item : adjustmentList.first<AdjustmentsBeforeGamma>()) addAdjustment(item);
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
        for (const auto& item : adjustmentList.subspan<AdjustmentsBeforeGamma>()) addAdjustment(item);
        const auto addFields = [&](const auto& fields, auto group, int maximum, auto visible, auto duration, int durationMaximum) {
            for (const auto& [key, field] : fields)
                number(key, group, field, 0, duration(field) ? durationMaximum : maximum, visible);
        };
        const auto noDuration = [](auto) { return false; };
        const auto crt = activeScreen(ScreenType::Crt);
        section("Emulation.Video.Screen.Crt", [crt, showsAdjustments] { return crt() && !showsAdjustments(); });
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
            section(label, [current, showsAdjustments, screen] {
                return current() && (screen == ScreenType::LedBacklitLcd || !showsAdjustments());
            });
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
        section("Emulation.Video.FixedPixel.Technology.Oled", [oled, showsAdjustments] { return oled() && !showsAdjustments(); });
        panelSource(panelDefinitionChoices, oled);
        labelChoices(LcdPanelTypeResource, oledPanels, &Profile::oled, &OledProfile::panelType, oled);
        labelChoices(LcdPixelStructureResource, oledStructures, &Profile::oled, &OledProfile::pixelStructure, oled);
        panelInput(ScreenType::Oled, oled);

        const auto plasma = activeScreen(ScreenType::Plasma);
        section("Emulation.Video.Technology.Plasma", [plasma, showsAdjustments] { return plasma() && !showsAdjustments(); });
        panelSource(orderedChoices(panelDefinitionChoices, plasmaDefinitionChoices()), plasma);
        enumField(PlasmaCellStructureResource, plasmaCellChoices, &Profile::plasma, &PlasmaProfile::cellStructure, plasma);
        panelInput(ScreenType::Plasma, plasma);
        const auto vector = enabled(DisplayTechnology::Vector);
        const auto vectorParameters = std::span(vectorFields);
        section(VectorDrawingResource, vector);
        addFields(vectorParameters.first<VectorDrawingFields>(), &Profile::vector, VectorProfile::MaximumIntensity, vector, noDuration, 0);
        labelChoices(VectorPhosphorResource, vectorColors, &Profile::vector, &VectorProfile::phosphorColor, vector);
        section(VectorGlowResource, vector);
        addFields(vectorParameters.subspan<VectorDrawingFields>(), &Profile::vector, VectorProfile::MaximumIntensity, vector, noDuration, 0);
        const auto vfd = enabled(DisplayTechnology::Vfd);
        const auto vfdParameters = std::span(vfdFields);
        section(VfdPhosphorResource, vfd);
        labelChoices(VfdColorResource, vfdColors, &Profile::vfd, &VfdProfile::color, vfd);
        addFields(vfdParameters.first<VfdPhosphorFields>(), &Profile::vfd, VfdProfile::MaximumIntensity, vfd, noDuration, 0);
        section(VfdAnodesResource, vfd);
        labelChoices(VfdStructureResource, vfdStructures, &Profile::vfd, &VfdProfile::structure, vfd);
        for (const auto& [key, field] : vfdParameters.subspan<VfdPhosphorFields, VfdAnodeFields>()) {
            number(key, &Profile::vfd, field, 0, VfdProfile::MaximumIntensity,
                [this, vfd] { return vfd() && profile_.vfd.structure == VfdStructure::DotMatrix; });
        }
        section(VfdResponseResource, vfd);
        addFields(vfdParameters.subspan<VfdPhosphorFields + VfdAnodeFields>(), &Profile::vfd, VfdProfile::MaximumIntensity, vfd,
            [](auto field) { return field == &VfdProfile::persistenceMilliseconds; }, VfdProfile::MaximumPersistenceMilliseconds);

        const auto led = enabled(DisplayTechnology::LedMatrix);
        const auto ledParameters = std::span(ledFields);
        section(LedMatrixStructureResource, led);
        labelChoices("Emulation.Video.Parameter.LedMatrixShape", ledShapes, &Profile::ledMatrix, &LedMatrixProfile::shape, led);
        addFields(ledParameters.first<LedStructureFields>(), &Profile::ledMatrix, LedMatrixProfile::MaximumIntensity, led, noDuration, 0);
        section(LedMatrixEmissionResource, led);
        labelChoices("Emulation.Video.Parameter.LedMatrixColor", ledColors, &Profile::ledMatrix, &LedMatrixProfile::color, led);
        addFields(ledParameters.subspan<LedStructureFields>(), &Profile::ledMatrix, LedMatrixProfile::MaximumIntensity, led, noDuration, 0);

        const auto dot = enabled(DisplayTechnology::DotMatrix);
        const auto dotParameters = std::span(dotFields);
        section(DotCellsResource, dot);
        labelChoices("Emulation.Video.Parameter.DotMatrixShape", dotShapes, &Profile::dotMatrix, &DotMatrixProfile::shape, dot);
        addFields(dotParameters.first<DotCellFields>(), &Profile::dotMatrix, DotMatrixProfile::MaximumIntensity, dot, noDuration, 0);
        section(DotLightResource, dot);
        labelChoices("Emulation.Video.Parameter.DotMatrixPalette", dotPalettes, &Profile::dotMatrix, &DotMatrixProfile::palette, dot);
        addFields(dotParameters.subspan<DotCellFields, DotLightFields>(), &Profile::dotMatrix, DotMatrixProfile::MaximumIntensity, dot, noDuration, 0);
        section(DotTimingResource, dot);
        addFields(dotParameters.subspan<DotCellFields + DotLightFields>(), &Profile::dotMatrix, DotMatrixProfile::MaximumIntensity, dot,
            [](auto) { return true; }, DotMatrixProfile::MaximumDurationMilliseconds);
        const auto segments = enabled(DisplayTechnology::SegmentDisplay);
        const auto segmentParameters = std::span(segmentFields);
        constexpr size_t SegmentCellFields = 3, SegmentGeometryFields = 2, SegmentEmissionFields = 5;
        constexpr size_t SegmentEmissionOffset = SegmentCellFields + SegmentGeometryFields;
        constexpr size_t SegmentTimingOffset = SegmentEmissionOffset + SegmentEmissionFields;
        section("Emulation.Video.SegmentDisplay.Group.Cells", segments);
        labelChoices("Emulation.Video.Parameter.SegmentDisplayLayout", segmentLayoutLabels, &Profile::segmentDisplay, &SegmentDisplayProfile::layout, segments);
        addFields(segmentParameters.first<SegmentCellFields>(), &Profile::segmentDisplay, SegmentDisplayProfile::MaximumIntensity, segments, noDuration, 0);
        section("Emulation.Video.SegmentDisplay.Group.Geometry", segments);
        addFields(segmentParameters.subspan<SegmentCellFields, SegmentGeometryFields>(), &Profile::segmentDisplay,
            SegmentDisplayProfile::MaximumIntensity, segments, noDuration, 0);
        labelChoices("Emulation.Video.Parameter.SegmentDisplayEndShape", segmentEndShapeLabels, &Profile::segmentDisplay, &SegmentDisplayProfile::endShape, segments);
        toggleSetting("Emulation.Video.Parameter.SegmentDisplayDecimalPoint", [](const Profile& p) { return p.segmentDisplay.decimalPoint; },
            [](Profile& p, bool value) { p.segmentDisplay.decimalPoint = value; }, segments);
        toggleSetting("Emulation.Video.Parameter.SegmentDisplayColon", [](const Profile& p) { return p.segmentDisplay.colon; },
            [](Profile& p, bool value) { p.segmentDisplay.colon = value; }, segments);
        section("Emulation.Video.SegmentDisplay.Group.Emission", segments);
        labelChoices("Emulation.Video.Parameter.SegmentDisplayColor", segmentColorLabels, &Profile::segmentDisplay, &SegmentDisplayProfile::color, segments);
        addFields(segmentParameters.subspan<SegmentEmissionOffset, SegmentEmissionFields>(), &Profile::segmentDisplay,
            SegmentDisplayProfile::MaximumIntensity, segments, noDuration, 0);
        section("Emulation.Video.SegmentDisplay.Group.LightAndResponse", segments);
        addFields(segmentParameters.subspan<SegmentTimingOffset>(), &Profile::segmentDisplay, SegmentDisplayProfile::MaximumIntensity, segments,
            [](auto field) { return field == &SegmentDisplayProfile::persistenceMilliseconds || field == &SegmentDisplayProfile::responseTimeMilliseconds; },
            SegmentDisplayProfile::MaximumDurationMilliseconds);
        const auto paper = enabled(DisplayTechnology::EPaper);
        section("Emulation.Video.EPaper.Group.InkAndColor", paper);
        labelChoices("Emulation.Video.Parameter.EPaperColorMode", inkModes, &Profile::ePaper, &EPaperProfile::colorMode, paper);
        number("Emulation.Video.Parameter.EPaperInkDensity", &Profile::ePaper, &EPaperProfile::inkDensity, 0, EPaperProfile::MaximumIntensity, paper);
        number("Emulation.Video.Parameter.EPaperContrast", &Profile::ePaper, &EPaperProfile::contrast, 0, EPaperProfile::MaximumIntensity, paper);
        number("Emulation.Video.Parameter.EPaperDithering", &Profile::ePaper, &EPaperProfile::dithering, 0, EPaperProfile::MaximumIntensity, paper);
        number("Emulation.Video.Parameter.EPaperColorSaturation", &Profile::ePaper, &EPaperProfile::colorSaturation, 0, EPaperProfile::MaximumIntensity, [this, paper] { return paper() && profile_.ePaper.colorMode == EPaperColorMode::Color4096; });
        section("Emulation.Video.EPaper.Group.PaperSurface", paper);
        number("Emulation.Video.Parameter.EPaperPaperBrightness", &Profile::ePaper, &EPaperProfile::paperBrightness, 0, EPaperProfile::MaximumIntensity, paper);
        number("Emulation.Video.Parameter.EPaperPaperWarmth", &Profile::ePaper, &EPaperProfile::paperWarmth, 0, EPaperProfile::MaximumIntensity, paper);
        number("Emulation.Video.Parameter.EPaperSurfaceTexture", &Profile::ePaper, &EPaperProfile::surfaceTexture, 0, EPaperProfile::MaximumIntensity, paper);
        number("Emulation.Video.Parameter.EPaperEdgeSoftness", &Profile::ePaper, &EPaperProfile::edgeSoftness, 0, EPaperProfile::MaximumIntensity, paper);
        section("Emulation.Video.EPaper.Group.Refresh", paper);
        number("Emulation.Video.Parameter.EPaperRefreshTime", &Profile::ePaper, &EPaperProfile::refreshTimeMilliseconds, 0, EPaperProfile::MaximumDurationMilliseconds, paper);
        number("Emulation.Video.Parameter.EPaperGhosting", &Profile::ePaper, &EPaperProfile::ghosting, 0, EPaperProfile::MaximumIntensity, paper);

        const auto projection = activeScreen(ScreenType::Projection);
        section("Emulation.Video.Technology.Projection", [projection, showsAdjustments] { return projection() && !showsAdjustments(); });
        panelSource(panelDefinitionChoices, projection);
        labelChoices(ProjectionTechnologyResource, projectionTechnologies, &Profile::projection, &ProjectionProfile::technology, projection);
        labelChoices(ProjectionSurfaceResource, projectionSurfaces, &Profile::projection, &ProjectionProfile::surface, projection);
        number("Emulation.Video.Parameter.ProjectionAmbientLight", &Profile::projection, &ProjectionProfile::ambientLight,
            0, ProjectionProfile::MaximumAmbientLight, projection);
        panelInput(ScreenType::Projection, projection);
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
        section("Emulation.Video.Settings.Stylistic");
        constexpr std::array effects{
            std::pair{"Emulation.Video.Parameter.Grain", &StylisticEffects::grain},
            std::pair{"Emulation.Video.Parameter.Vhs", &StylisticEffects::vhs},
            std::pair{"Emulation.Video.Parameter.ChromaticAberration", &StylisticEffects::chromaticAberration},
            std::pair{"Emulation.Video.Parameter.Bloom", &StylisticEffects::bloom}
        };
        addFields(effects, &Profile::stylistic, StylisticEffects::MaximumIntensity, std::function<bool()>{}, noDuration, 0);
        toggleSetting("Emulation.Video.Parameter.Sepia", [](const Profile& p) { return p.stylistic.sepia; },
            [](Profile& p, bool value) { p.stylistic.sepia = value; });
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
        const auto& config = session_->configuration();
        auto blocks = host_->describe(session_->moduleId(), config);
        auto ranges = host_->numericSettings(session_->moduleId(), config);
        auto videoSections = host_->videoSettingsSections(session_->moduleId(), config);
        auto dependencies = host_->settingsDependencies(session_->moduleId(), config);
        osdValues_ = SettingsRules::values(blocks);
        osdDependencies_ = std::move(dependencies);
        osdBlocks_ = std::move(blocks);
        osdRanges_ = std::move(ranges);
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

    FullscreenOsd::Entry osdModuleField(const SettingsBlock& block, const SettingsField& field) {
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
        const auto unitKey = range == osdRanges_.cend() ? QString{} : range->unitResourceKey;
        if (field.editor == SettingsEditor::Information || (!toggle && !numeric && field.choices.isEmpty())) {
            entry.enabled = [] { return false; };
            entry.status = [this, current] {
                const auto* value = current();
                return value ? moduleFieldDisplay(*value, osdRanges_.value(value->id).unitResourceKey,
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
                    return editor.display(index, text(translations_, AutomaticSettingKey),
                        unitKey.isEmpty() ? QString{} : moduleSettingText(unitKey));
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
                    return moduleChoiceDisplay(choices[index], unitKey,
                        [this](const QString& key) { return moduleSettingText(key); });
                },
                [choices, save](int index) { return index < choices.size() && save(choices[index].id); },
                0, unknown - 1, false};
        }
        return entry;
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
        namespace Icons = EmulationCommandIcons;
        controls.icon = Icons::icon(*this, Icons::Controller, palette().color(QPalette::ButtonText).rgb());
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
            const auto action = [&](const char* key, std::function<void()> command, QStyle::StandardPixmap icon) {
                return Entry{text(translations_, key), [this, command = std::move(command)] {
                    try { command(); }
                    catch (...) { surface_->setIndicator(text(translations_, ErrorKey)); }
                }, {}, {}, style()->standardIcon(icon)};
            };
            refreshQuickStates();
            refreshOsdBlocks();
            auto save = action(QuickSaveKey, [this] { quickState(true); }, QStyle::SP_DialogSaveButton);
            save.enabled = [this] { return quickSave_->isEnabled(); };
            auto load = action(QuickLoadKey, [this] { quickState(false); }, QStyle::SP_DialogOpenButton);
            load.enabled = [this] { return quickLoad_->isEnabled(); };
            std::vector<Entry> entries;
            entries.push_back(action(OsdResumeKey, [this] { if (osd_) osd_->close(); }, QStyle::SP_MediaPlay));
            entries.push_back({text(translations_, OsdSavedStatesKey), {}, {std::move(save), std::move(load)}, {},
                style()->standardIcon(QStyle::SP_DialogSaveButton)});
            entries.push_back({text(translations_, OsdVideoSettingsKey), {}, videoOsdSettings(), {},
                style()->standardIcon(QStyle::SP_ComputerIcon), true});
            entries.back().fullHeight = true;
            auto mute = action(OsdMuteKey, [this] { mute_->toggle(); }, QStyle::SP_MediaVolumeMuted);
            mute.enabled = [this] { return mute_->isEnabled(); };
            mute.status = [this] { return QString::fromUtf16(mute_->isChecked() ? OsdMutedStatus : OsdUnmutedStatus); };
            std::vector<Entry> audioRows{std::move(mute)};
            appendOsdModuleSettings(audioRows, QLatin1String(AudioTabKey));
            entries.push_back({text(translations_, OsdAudioKey), {}, std::move(audioRows), {},
                style()->standardIcon(QStyle::SP_MediaVolume)});
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
                style()->standardIcon(QStyle::SP_FileDialogDetailedView)};
            machineSettings.fullHeight = true;
            std::vector<std::function<bool()>> machineTabVisibility;
            for (const auto& page : machineSettings.children) machineTabVisibility.push_back(page.visible);
            machineSettings.visible = [pages = std::move(machineTabVisibility)] {
                return std::ranges::any_of(pages, [](const auto& visible) { return visible(); });
            };
            entries.push_back(std::move(machineSettings));
            auto exit = action(OsdExitKey, [this] { toggleFullScreen(); }, QStyle::SP_DialogCloseButton);
            exit.separatorBefore = true;
            entries.push_back(std::move(exit));
            const auto footer = osdFooterHints();
            const QString media = osdGameName();
            if (pause && machine.state() == MachineState::Running) {
                machine.execute(LifecycleCommand::Pause, {});
                pausedForOsd = machine.state() == MachineState::Paused;
            }
            osd_ = new FullscreenOsd(*fullScreen_, session_->configuration().name, media, {},
                std::move(entries), [this, pausedForOsd, resumeAfterOsd, restorePointer] {
                    osd_ = nullptr;
                    osdRepeat_.clear();
                    updateShaderBadge();
                    for (const auto& shortcut : configuredShortcuts_) shortcut.keyboard->setParent(surface_);
                    for (auto* shortcut : osdNavigationShortcuts_) shortcut->setEnabled(false);
                    heldGamepads_ = lastGamepads_;
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
            releaseInput();
            osd_->setFooterHints(footer);
            osd_->setExternalKeyboardNavigation(true);
            for (const auto& shortcut : configuredShortcuts_) shortcut.keyboard->setParent(osd_);
            for (auto* shortcut : osdNavigationShortcuts_) shortcut->setEnabled(true);
            osd_->activate();
            updateShaderBadge();
        } catch (...) {
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
            const auto inputState = session_->machine().state();
            if (inputState == MachineState::Running || inputState == MachineState::Paused || fullScreen_) pollControllers();
            auto& machine = session_->machine();
            const auto state = machine.state();
            const bool running = state == MachineState::Running;
            const bool paused = state == MachineState::Paused;
            if (gamepadSource_) {
                const bool active = running && physicalInputActive() && !osd_;
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
            const auto revision = session_->runtimeRevision();
            if (presentedRuntime_ != revision) {
                presentedRuntime_ = revision;
                surface_->clear();
                executionRate_.reset();
            }
            surface_->setEmulationRunning(running);
            const auto frame = running || paused ? machine.latestFrame() : nullptr;
            if (!frame && !paused) surface_->submit(nullptr);
            const bool visible = surface_->isVisible();
            if (!running || !visible) executionRate_.reset();
            if (frame && visible) {
                surface_->submit(frame);
                if (running) {
                    std::optional<std::uint64_t> clockHz;
                    if (const auto* status = dynamic_cast<const IMachineCoreRuntimeStatus*>(&machine))
                        clockHz = status->coreRuntimeStatus().processorClockHz;
                    if (!clockHz)
                        if (const auto* clock = dynamic_cast<const IMachineProcessorClock*>(&machine))
                            clockHz = clock->processorClockHz();
                    executionRate_.observe(frame->sequence, frame->timestamp, ExecutionRateWindow::Clock::now(), clockHz);
                }
            }
            const bool audible = running && surface_->isVisible() && !mute_->isChecked();
            session_->pumpAudio(audible);
            QString statistics;
            const bool showStatistics = fullScreen_ ? workspace_.showFullscreenEmulationStatistics() : statistics_->isChecked();
            if (showStatistics && frame && (running || paused)) {
                const auto expected = machine.framesPerSecond();
                const auto measured = executionRate_.measuredFramesPerSecond();
                const auto frequency = std::isfinite(expected) && expected > 0 ? expected : measured;
                const QLocale locale;
                const auto fps = text(translations_, FpsKey).arg(locale.toString(measured, 'f', 1));
                const auto imageHeight = frame->scanMode == ScanMode::Progressive ? qint64(frame->height) : 2LL * frame->height;
                statistics = QString::fromUtf8(FrameStatisticsFormat).arg(locale.toString(frame->width),
                    locale.toString(imageHeight), locale.toString(frequency, 'f', 1), fps);
                if (const auto& rate = executionRate_.rate(); rate && rate->effectiveMegahertz) {
                    statistics += text(translations_, ExecutionRateKey).arg(
                        locale.toString(*rate->effectiveMegahertz, 'f', 2),
                        locale.toString(rate->speedRatio * PercentScale, 'f', 0));
                }
            }
            surface_->setStatistics(std::move(statistics));
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
    VideoPresentation::ProfileStore profiles_;
    VideoPresentation::Profile profile_;
    OpenMachineSettings openMachineSettings_;
    Translation::Subscription subscription_;
    VideoSurface* surface_ = nullptr;
    QVBoxLayout* layout_ = nullptr;
    QDialog* fullScreen_ = nullptr;
    QPointer<FullscreenOsd> osd_;
    QPointer<VideoPresentation::ShaderLoadingBadge> shaderBadge_;
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
    SettingsRules::Values osdValues_;
    QHash<QString, SettingsNumericRange> osdRanges_;
    QHash<QString, VideoSettingsSection> osdVideoSections_;
    QToolButton *power_, *pause_, *reset_, *hardReset_;
    std::optional<MachineState> lifecycleState_;
    quint64 presentedRuntime_ = 0;
    QList<std::pair<QToolButton*, const char*>> commandButtons_;
    QHash<QToolButton*, const EmulationKeyboardShortcuts::Binding*> commandShortcuts_;
    QPushButton *save_, *load_;
    QToolButton *quickSave_, *quickLoad_, *screenshotButton_;
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
        std::array<QToolButton*, TapeControls.size()> tape{};
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
    EmulationPhysicalShortcuts::OsdDirectionRepeat osdRepeat_;
    quint32 heldPointerButtons_ = 0;
    bool changingVideoHost_ = false;
    std::unique_ptr<Platform::GamepadSource> gamepadSource_;
    EmulationControllerFeedback controllerFeedback_;
    QHash<quint64, Platform::GamepadState> lastGamepads_;
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
            if (auto session = state.emulation->open(state.emulationModuleId, config)) {
                addSession(session, true);
                updateSelection();
                return;
            }
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
        try {
            auto config = session ? session->configuration() : choice();
            if (session) {
                const auto savedConfigurations = state.emulation->configurations(module);
                const auto latest = std::ranges::find(savedConfigurations, config.id, &Configuration::id);
                if (latest != savedConfigurations.end()) config = *latest;
            }
            if (config.id.isNull()) return;
            bool returningToMotor = false;
            while (editConfigurationPage(parent, session, module, config, returningToMotor))
                returningToMotor = true;
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
        bool returningToMotor = false;
        while (editConfigurationPage(parent, session, module, config, returningToMotor))
            returningToMotor = true;
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
                bool returningToMotor = false;
                while (editConfigurationPage(parent, session, id, config, returningToMotor, &*module, &next, creating ? &draft : nullptr))
                    returningToMotor = true;
                machine = std::move(next);
            }
            refreshConfigurations();
        } catch (...) {
            showError(text(translations, ErrorKey));
        }
    }

    bool editConfigurationPage(QWidget* parent, const std::shared_ptr<Session>& session, const QString& module,
        Configuration& config, bool returningToMotor, const ModuleInfo* optionsModule = nullptr,
        QString* nextMachine = nullptr, MachineConfigurationDraft* storedDraft = nullptr) {
        const auto blocks = state.emulation->describe(module, config);
        const auto ranges = state.emulation->numericSettings(module, config);
        const auto editableSettings = state.emulation->editableNumericSettings(module, config);
        const auto dependencies = state.emulation->settingsDependencies(module, config);
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
        const auto inputValid = [&] {
            return (!controllerPorts || controllerPorts->valid()) &&
                std::ranges::all_of(inputEditors, &MachineInputBindings::valid);
        };
        auto currentValues = SettingsRules::values(blocks);
        QDialog dialog(parent);
        QTimer autosave;
        autosave.setSingleShot(true);
        bool interactive = false, inputDirty = false, storageDirty = false;
        const bool saveWhileEditing = optionsModule && !storedDraft;
        const auto scheduleSave = [&] {
            if (interactive && saveWhileEditing) autosave.start(0);
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
            QGroupBox* widget;
            QWidget* page;
        };
        QList<SettingControl> controls;
        QList<SettingsGroup> groups;
        QJsonObject changes;
        QString requestedEmulator;
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
        if (auto storage = state.emulation->storageSettings(module, config)) {
            constexpr auto StorageScope = "EmulationStorage/%1/%2";
            storagePage = new EmulationStoragePanel(std::move(*storage), state, translations,
                QString::fromLatin1(StorageScope).arg(module, config.machineId),
                [&](const QString& key) { return translated(module, key); }, sections);
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
            for (const auto& group : groups) {
                const bool visible = SettingsRules::blockVisible(dependencies, currentValues, *group.block);
                group.widget->setVisible(visible);
                if (visible) visiblePages.insert(group.page);
            }
            for (const auto& field : controls) {
                field.form->setRowVisible(field.control, SettingsRules::fieldVisible(dependencies, currentValues, field.id));
            }
            for (int index = 0; index < sections->count(); ++index)
                sections->setTabVisible(index, visiblePages.contains(sections->widget(index)));
        };
        const auto applyChange = [&](const QString& id, const QString& value) {
            const auto patch = SettingsRules::changesFor(dependencies, currentValues, id, value);
            for (auto field = patch.constBegin(); field != patch.constEnd(); ++field) {
                const auto next = field.value().toString();
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
        };
        const int sessionIndex = session ? activeTab(module, config.machineId, config.id) : -1;
        auto* sessionView = sessionIndex > 0 ? static_cast<SessionView*>(tabs->widget(sessionIndex)) : nullptr;
        for (const auto& block : blocks) {
            if (!block.visible) continue;
            auto* section = sectionLayouts.value(block.tabResourceKey);
            if (!section) {
                auto* scroll = new QScrollArea(sections);
                scroll->setWidgetResizable(true);
                auto* content = new QWidget(scroll);
                section = new QVBoxLayout(content);
                if (sessionView && block.tabResourceKey == QLatin1String(VideoKey))
                    section->addWidget(sessionView->createShaderStatusRow(content));
                scroll->setWidget(content);
                sections->addTab(scroll, translated(module, block.tabResourceKey));
                sectionLayouts.insert(block.tabResourceKey, section);
                sectionPages.insert(block.tabResourceKey, scroll);
            }
            auto* group = new QGroupBox(translated(module, block.titleResourceKey), section->parentWidget());
            auto* form = new QFormLayout(group);
            section->addWidget(group);
            groups.append({&block, group, sectionPages.value(block.tabResourceKey)});
            for (const auto& field : block.fields) {
                if (!field.visible) continue;
                QWidget* control = nullptr;
                std::function<QString()> value;
                std::function<void(const QString&)> write;
                QWidget* input = nullptr;
                ModuleEditableSlider* editable = nullptr;
                const auto range = ranges.constFind(field.id);
                const auto numeric = field.editor == SettingsEditor::Slider && range != ranges.cend() ?
                    ModuleNumericSetting::from(*range) : std::nullopt;
                if (field.editor == SettingsEditor::Information) {
                    auto* status = new QLabel(moduleFieldDisplay(field, ranges.value(field.id).unitResourceKey,
                        [&](const QString& key) { return translated(module, key); }), group);
                    status->setTextFormat(Qt::PlainText);
                    status->setWordWrap(true);
                    form->addRow(translated(module, field.labelResourceKey), status);
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
                    const auto unit = range->unitResourceKey.isEmpty() ? QString{} : translated(module, range->unitResourceKey);
                    const auto automatic = text(translations, AutomaticSettingKey);
                    const auto update = [caption, editor = *numeric, automatic, unit](int index) {
                        caption->setText(editor.display(index, automatic, unit));
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
                    auto* combo = new QComboBox(group);
                    for (const auto& item : field.choices)
                        combo->addItem(item.invariantDisplayValue.isEmpty() ? translated(module, item.displayResourceKey) : item.invariantDisplayValue, item.id);
                    combo->setCurrentIndex(combo->findData(field.value));
                    control = input = combo;
                    write = [combo](const QString& value) {
                        const QSignalBlocker block(combo);
                        combo->setCurrentIndex(combo->findData(value));
                    };
                    value = [combo, field] { return combo->currentIndex() < 0 ? field.value : combo->currentData().toString(); };
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
                            const auto path = field.editor == SettingsEditor::Folder
                                ? QFileDialog::getExistingDirectory(row, label, line->text())
                                : QFileDialog::getOpenFileName(row, label, line->text());
                            if (!path.isEmpty()) {
                                line->setText(path);
                                scheduleSave();
                            }
                        });
                        control = row;
                    }
                }
                control->setEnabled(field.enabled);
                if (!field.explanationResourceKey.isEmpty())
                    control->setToolTip(translated(module, field.explanationResourceKey));
                form->addRow(translated(module, field.labelResourceKey), control);
                if (field.enabled) {
                    const auto onChanged = [&, id = field.id, value] { applyChange(id, value()); };
                    const auto commit = [onChanged, &scheduleSave] {
                        onChanged();
                        scheduleSave();
                    };
                    if (editable) editable->onValueChanged(commit);
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
                            if (interactive && saveWhileEditing) autosave.start(TextIdleMilliseconds);
                        });
                }
                controls.append({field.id, form, control, field.enabled ? std::move(value) : nullptr, std::move(write)});
            }
        }
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
        if (returningToMotor && emulatorPage) sections->setCurrentWidget(emulatorPage);
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
        auto* saveError = new QLabel(&dialog);
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
                outputs, firmware, storage, input, emulator, mode);
            if (outcome == MachineSettingsResult::Rejected) throw std::runtime_error(ErrorKey);
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
        autosave.stop();
        if (emulatorPage) emulatorPage->finish();
        const bool accepted = result == QDialog::Accepted;
        if ((!accepted && !optionsModule) || (requestedEmulator.isEmpty() && !inputValid())) return false;
        if (!accepted && !storedDraft && !pending()) return false;
        const auto save = storedDraft && !accepted ? MachineSettingsSave::KeepDraft : MachineSettingsSave::Persist;
        const auto outcome = commitSettings(save, !saveWhileEditing, requestedEmulator);
        if (outcome == MachineSettingsResult::EmulatorChanged) return true;
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
        showError(issues.isEmpty() ? QString{} : text(translations, ModuleIssueKey).arg(issues.front().directory));
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
        const int index = tabs->addTab(view, themeIcon(QLatin1String(AppConstants::Emulation), tabs->palette().color(QPalette::WindowText)), configuration.name.isEmpty() ? configuration.machineId : configuration.name);
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
        try {
            view.state.emulation->close(sessionView->session());
            view.tabs->removeTab(index);
            sessionView->deleteLater();
            view.updateSelection();
        } catch (...) {
            view.showError(text(view.translations, ErrorKey));
        }
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
