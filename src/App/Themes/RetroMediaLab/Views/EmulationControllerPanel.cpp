#include "EmulationControllerPanel.h"
#include "Shared/Controls/ControllerAnalogProfiles.h"
#include "Shared/Controls/ControllerArtwork.h"
#include "Shared/Controls/ControllerVisualProfiles.h"
#include <ErrorKeys.h>
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QComboBox>
#include <QFocusEvent>
#include <QFontDatabase>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHideEvent>
#include <QLabel>
#include <QLocale>
#include <QMouseEvent>
#include <QPushButton>
#include <QShowEvent>
#include <QSignalBlocker>
#include <QSlider>
#include <QTimer>
#include <QToolButton>

#include <QGridLayout>
#include <QHeaderView>
#include <QScrollArea>
#include <QTableWidget>
#include <QVBoxLayout>
#include <algorithm>
#include <array>
#include <memory>
#include <optional>

namespace RetroMediaLab {
namespace {
namespace Analog = ControllerAnalogProfiles;
namespace VisualProfiles = ControllerVisualProfiles;
using HostModel = ControllerArtwork::HostModel;
constexpr auto VisualModelKey = "Controllers.VisualModel";
constexpr auto ModelAutoKey = "Emulation.Controller.Automatic";
constexpr int AutomaticModel = -1;
constexpr const char* ModelKeys[]{
    "Controllers.Model.GenericGamepad",
    "Controllers.Model.XboxSeries",
    "Controllers.Model.XboxOne",
    "Controllers.Model.Xbox360",
    "Controllers.Model.Xbox360White",
    "Controllers.Model.XboxRematchCore",
    "Controllers.Model.PlayStation4",
    "Controllers.Model.PlayStation5",
    "Controllers.Model.MasterSystem",
    "Controllers.Model.NintendoEntertainmentSystem",
    "Controllers.Model.Nintendo64",
    "Controllers.Model.SuperNintendo",
    "Controllers.Model.MegaDrive3",
    "Controllers.Model.MegaDrive6",
    "Controllers.Model.PlayStation1",
    "Controllers.Model.PlayStation2",
    "Controllers.Model.Saturn",
    "Controllers.Model.Dreamcast",
    "Controllers.Model.RacingWheel",
    "Controllers.Model.FlightStick",
    "Controllers.Model.ArcadeStick",
};
static_assert(std::size(ModelKeys) == static_cast<unsigned>(HostModel::ArcadeStick) + 1);
constexpr int LiveIntervalMs = 50;
constexpr int AxisPrecision = 3;
constexpr auto PercentFormat = "%1 %";
constexpr auto CountArgument = "%1";
constexpr auto DetectKey = "Controllers.Detect";
constexpr auto DeviceKey = "Controllers.Device";
constexpr auto NoneDetectedKey = "Controllers.NoneDetected";
constexpr auto DetectedCountKey = "Controllers.DetectedCount";
constexpr auto DeadZonesKey = "Controllers.AnalogDeadZones";
constexpr auto LiveInputsKey = "Controllers.LiveInputs";
constexpr auto AnalogValuesKey = "Controllers.AnalogValues";
constexpr auto RumbleKey = "Controllers.Rumble";
constexpr auto RumbleStrengthKey = "Controllers.RumbleStrength";
constexpr auto TestRunningKey = "Controllers.TestRunning";
constexpr auto TestFailedKey = "Controllers.TestFailed";
constexpr auto TestCompletedKey = "Controllers.TestCompleted";
constexpr int RumblePulseMs = 250;
constexpr int RumbleMinPercent = 5;
constexpr int RumbleMaxPercent = 100;
constexpr int InitialRumblePercent = 20;
constexpr int RumbleColumns = 2;
constexpr int ControlColumn = 0, ValueColumn = 1, ActiveColumn = 2;
constexpr const char* RawColumnKeys[] = {"Controllers.Control", "Controllers.Value", "Controllers.Active"};

struct Motor {
    Platform::GamepadRumbleMotor capability;
    const char* key;
    float Platform::GamepadRumble::* member;
};
constexpr Motor Motors[] = {
    {Platform::RumbleLowFrequency, "Controllers.Enum.LowFrequencyMotor", &Platform::GamepadRumble::lowFrequency},
    {Platform::RumbleHighFrequency, "Controllers.Enum.HighFrequencyMotor", &Platform::GamepadRumble::highFrequency},
    {Platform::RumbleLeftTrigger, "Controllers.LeftTrigger", &Platform::GamepadRumble::leftTrigger},
    {Platform::RumbleRightTrigger, "Controllers.RightTrigger", &Platform::GamepadRumble::rightTrigger},
};

struct ProfileField {
    const char* key;
    int Analog::Profile::* member;
    int maximum;
};

constexpr ProfileField Fields[]{
    {"Controllers.StickDeadZone", &Analog::Profile::stickPercent, Analog::MaximumInnerPercent},
    {"Controllers.TriggerDeadZone", &Analog::Profile::triggerPercent, Analog::MaximumInnerPercent},
    {"Controllers.OuterDeadZone", &Analog::Profile::outerPercent, Analog::MaximumOuterPercent},
};
constexpr const char* AxisKeys[]{
    "Controllers.LeftStickX", "Controllers.LeftStickY", "Controllers.RightStickX", "Controllers.RightStickY",
    "Controllers.LeftTrigger", "Controllers.RightTrigger"
};
constexpr auto SteeringKey = "Controllers.Steering";
constexpr auto ThrottleKey = "Controllers.Throttle";
constexpr auto BrakeKey = "Controllers.Brake";
constexpr auto ClutchKey = "Controllers.Clutch";
constexpr auto HandbrakeKey = "Controllers.Handbrake";
constexpr auto GearKey = "Controllers.Gear";
constexpr auto CapabilitiesKey = "Controllers.Capabilities";
constexpr auto IdentityKey = "Controllers.Identity";
constexpr auto UnknownKey = "Common.Unknown";
constexpr auto IdentityLine = "%1: %2";
constexpr auto HexPair = "%1:%2";
constexpr int IdentifierWidth = 4;
constexpr int HexBase = 16;
constexpr auto GameInputIdPrefix = QLatin1String("gameinput:");
constexpr auto ControlCountsKey = "Controllers.ControlCounts";
constexpr auto ReportCountsKey = "Controllers.ReportCounts";
constexpr auto WheelDetailsKey = "Controllers.WheelDetails";
constexpr auto NoneKey = "Controllers.None";
constexpr auto Separator = " · ";
constexpr auto YesKey = "Common.Yes";
constexpr auto NoKey = "Common.No";
enum CapabilityRow { StatusRow, InputKindsRow, ControlsRow, RumbleRow, ForceFeedbackRow, HapticsRow, SystemButtonsRow,
    RawReportsRow, GamepadLayoutRow, WheelRow, ExtraControlsRow, HapticEndpointRow, HapticLocationsRow, CapabilityRowCount };
constexpr const char* CapabilityKeys[]{
    DeviceKey, "Controllers.InputKinds", "Controllers.Controls", RumbleKey, "Controllers.ForceFeedback",
    "Controllers.Haptics", "Controllers.SystemButtons", "Controllers.RawReports", "Controllers.GamepadLayout",
    "Controllers.WheelCapabilities", "Controllers.ExtraControls", "Controllers.HapticEndpoint", "Controllers.HapticLocations"
};
constexpr const char* FeedbackEffects[]{
    "Controllers.Enum.Effect.Constant", "Controllers.Enum.Effect.Ramp", "Controllers.Enum.Effect.SineWave",
    "Controllers.Enum.Effect.SquareWave", "Controllers.Enum.Effect.TriangleWave", "Controllers.Enum.Effect.SawtoothUp",
    "Controllers.Enum.Effect.SawtoothDown", "Controllers.Enum.Effect.Spring", "Controllers.Enum.Effect.Friction",
    "Controllers.Enum.Effect.Damper", "Controllers.Enum.Effect.Inertia"
};
constexpr const char* GamepadLayoutGlyphs[]{
    "☰", "◫", "A", "B", "X", "Y", "D-pad ↑", "D-pad ↓", "D-pad ←", "D-pad →", "LB", "RB", "L3", "R3", "C", "Z",
    "LT", "RT", "LS ↑", "LS ↓", "LS ←", "LS →", "RS ↑", "RS ↓", "RS ←", "RS →", "L P1", "L P2", "R P1", "R P2"
};
constexpr const char* AxisDirections[]{"X", "Y", "Z"};
constexpr auto LinearKey = "Controllers.Enum.Linear";
constexpr auto AngularKey = "Controllers.Enum.Angular";
constexpr auto NormalKey = "Controllers.Enum.Normal";
constexpr auto ExtraAxesKey = "Controllers.Enum.Axes";
constexpr auto ExtraButtonsKey = "Controllers.Enum.Buttons";
constexpr auto FeedbackFormat = "#%1: ⏻ %2 · %3 · %4";
constexpr auto ExtraControlsFormat = "%1 %2: %3";
constexpr auto AxisDirectionFormat = "%1 %2";
constexpr auto ListSeparator = ", ";
static_assert(std::size(CapabilityKeys) == CapabilityRowCount);
using Capabilities = Platform::GamepadCapabilities;
constexpr std::pair<Capabilities::InputKind, const char*> InputKinds[]{
    {Capabilities::RawReports, "Controllers.RawReports"}, {Capabilities::Axes, "Controllers.ControlType.Axis"},
    {Capabilities::Buttons, "Controllers.ControlType.Button"}, {Capabilities::Switches, "Controllers.ControlType.Switch"},
    {Capabilities::Keyboard, "Emulation.Tab.Keyboard"}, {Capabilities::Mouse, "Emulation.Tab.Mouse"},
    {Capabilities::Sensors, "Controllers.Enum.Sensors"}, {Capabilities::Arcade, "Controllers.Model.ArcadeStick"},
    {Capabilities::Flight, "Controllers.Model.FlightStick"}, {Capabilities::Gamepad, "Controllers.Enum.Gamepad"},
    {Capabilities::Racing, "Controllers.Model.RacingWheel"}, {Capabilities::UiNavigation, "Controllers.Enum.UiNavigation"}
};
constexpr auto ShareKey = "Controllers.Enum.Share";
constexpr auto InputReportKey = "Controllers.Enum.InputReport";
constexpr auto OutputReportKey = "Controllers.Enum.OutputReport";
constexpr auto ReportFormat = "%1 #%2: ID=%3, %4 B";
constexpr auto UnknownFlagFormat = "0x%1";
constexpr auto ConnectedFormat = "✓ %1";
constexpr auto GuideGlyph = "◆";
constexpr const char* SwitchPositions[]{"●", "↑", "↗", "→", "↘", "↓", "↙", "←", "↖"};
constexpr const char* SwitchKinds[]{"↕ · 2", "✣ · 4", "✣ · 8"};
constexpr auto NumberedControlFormat = "%1 %2";
constexpr auto UnknownControlFormat = "%1 (%2)";
constexpr auto RawInputDevicePrefix = QLatin1String("rawgamecontroller:");
using ControlDescription = Platform::GamepadControlDescription;
constexpr std::pair<QLatin1StringView, const char*> ControlTypes[]{
    {QLatin1String(Platform::RawGamepad::AxisPrefix), "Controllers.ControlType.Axis"},
    {QLatin1String(Platform::RawGamepad::ButtonPrefix), "Controllers.ControlType.Button"},
    {QLatin1String(Platform::RawGamepad::SwitchPrefix), "Controllers.ControlType.Switch"}
};
constexpr std::pair<int, const char*> ControlLabelGlyphs[]{
    {1, "Xbox ◆"},
    {2, "Xbox ◫"},
    {3, "Xbox ☰"},
    {4, "Xbox ☰"},
    {5, "Xbox ◫"},
    {7, "Xbox A"},
    {8, "Xbox B"},
    {9, "Xbox X"},
    {10, "Xbox Y"},
    {11, "Xbox ↑"},
    {12, "Xbox ↓"},
    {13, "Xbox ←"},
    {14, "Xbox →"},
    {15, "Xbox LB"},
    {16, "Xbox LT"},
    {17, "Xbox L3"},
    {18, "Xbox RB"},
    {19, "Xbox RT"},
    {20, "Xbox R3"},
    {21, "Xbox P1"},
    {22, "Xbox P2"},
    {23, "Xbox P3"},
    {24, "Xbox P4"},
    {61, "↑"},
    {62, "↗"},
    {63, "→"},
    {64, "↘"},
    {65, "↓"},
    {66, "↙"},
    {67, "←"},
    {68, "↖"},
    {69, "↕"},
    {70, "↔"},
    {71, "✣"},
    {72, "↻"},
    {73, "↺"},
    {74, "↵"},
    {75, "◆"},
    {76, "⌂"},
    {77, "☰"},
    {78, "✕"},
    {79, "○"},
    {80, "□"},
    {81, "△"},
    {82, "★"},
    {83, "↑"},
    {84, "↓"},
    {85, "←"},
    {86, "→"},
    {87, "↻"},
    {88, "↺"},
    {89, "↔"},
    {90, "↕"},
    {91, "↕"},
    {92, "+"},
    {93, "−"},
    {94, "⏸"},
    {95, "⌂"},
    {96, "◆"},
    {97, "MODE"},
    {98, "SELECT"},
    {99, "☰"},
    {100, "◫"},
    {101, "↩"},
    {102, "START"},
    {103, "⋯"},
    {104, "↥"},
    {105, "↑"},
    {106, "↓"},
    {107, "←"},
    {108, "→"},
    {109, "LB"},
    {110, "LT"},
    {111, "LSB"},
    {112, "L1"},
    {113, "L2"},
    {114, "L3"},
    {115, "RB"},
    {116, "RT"},
    {117, "RSB"},
    {118, "R1"},
    {119, "R2"},
    {120, "R3"},
    {121, "L P1"},
    {122, "L P2"},
    {123, "R P1"},
    {124, "R P2"},
};
constexpr int LetterALabel = 25;
constexpr int LetterZLabel = 50;
constexpr int Number0Label = 51;
constexpr int Number9Label = 60;
constexpr auto RollKey = "Controllers.Roll";
constexpr auto PitchKey = "Controllers.Pitch";
constexpr auto YawKey = "Controllers.Yaw";

class ProfileSlider final : public QSlider {
public:
    explicit ProfileSlider(QWidget* parent, std::function<void()> save)
        : QSlider(Qt::Horizontal, parent), save_(std::move(save)) {}
protected:
    void mouseReleaseEvent(QMouseEvent* event) override {
        QSlider::mouseReleaseEvent(event);
        if (event->button() == Qt::LeftButton) save_();
    }
    void focusOutEvent(QFocusEvent* event) override {
        QSlider::focusOutEvent(event);
        save_();
    }
private:
    std::function<void()> save_;
};

class ControllerPanel final : public QWidget {
public:
    ControllerPanel(QWidget* parent, Translations::TranslationService& translations)
        : QWidget(parent), translations_(translations) {
        auto* layout = new QVBoxLayout(this);
        auto* header = new QHBoxLayout;
        devices_ = new QComboBox(this);
        deviceLabel_ = new QLabel(this);
        deviceLabel_->setBuddy(devices_);
        count_ = new QLabel(this);
        detect_ = new QPushButton(this);
        header->addWidget(deviceLabel_);
        header->addWidget(devices_, 1);
        header->addWidget(count_);
        header->addWidget(detect_);
        layout->addLayout(header);
        modelRow_ = new QWidget(this);
        auto* modelLayout = new QHBoxLayout(modelRow_);
        modelLayout->setContentsMargins({});
        modelLabel_ = new QLabel(modelRow_);
        models_ = new QComboBox(modelRow_);
        modelLabel_->setBuddy(models_);
        modelLayout->addWidget(modelLabel_);
        modelLayout->addWidget(models_, 1);
        layout->addWidget(modelRow_);
        modelRow_->hide();
        connect(models_, &QComboBox::activated, this, [this](int index) { chooseModel(index); });

        auto* scroll = new QScrollArea(this);
        scroll->setWidgetResizable(true);
        layout->addWidget(scroll, 1);
        auto* content = new QWidget(scroll);
        scroll->setWidget(content);
        auto* body = new QHBoxLayout(content);
        artworkPanel_ = new QWidget(content);
        artworkLayout_ = new QVBoxLayout(artworkPanel_);
        artworkLayout_->setContentsMargins({});
        body->addWidget(artworkPanel_, 1);
        artworkPanel_->hide();
        auto* details = new QWidget(content);
        body->addWidget(details, 2);
        auto* panels = new QVBoxLayout(details);
        identitySummary_ = new QLabel(details);
        identitySummary_->setTextFormat(Qt::PlainText);
        identitySummary_->hide();
        panels->addWidget(identitySummary_);
        panels->setContentsMargins({});
        zones_ = new QGroupBox(details);
        auto* zonesLayout = new QFormLayout(zones_);
        for (std::size_t i = 0; i < std::size(Fields); ++i) {
            auto* row = new QWidget(zones_);
            auto* rowLayout = new QHBoxLayout(row);
            rowLayout->setContentsMargins({});
            auto* slider = new ProfileSlider(row, [this] { save(); });
            slider->setRange(0, Fields[i].maximum);
            auto* percent = new QLabel(row);
            auto* label = new QLabel(zones_);
            label->setBuddy(slider);
            rowLayout->addWidget(slider, 1);
            rowLayout->addWidget(percent);
            zonesLayout->addRow(label, row);
            fields_[i] = {label, slider, percent};
            connect(slider, &QSlider::valueChanged, this, [this] {
                if (selected_.isEmpty()) return;
                draft_ = readSliders();
                dirty_ = true;
                Analog::preview(selected_, draft_);
                updatePercentages();
                sample();
            });
        }
        panels->addWidget(zones_);
        live_ = new QGroupBox(details);
        analogValues_ = new QFormLayout(live_);
        panels->addWidget(live_);
        capabilitiesToggle_ = new QToolButton(details);
        capabilitiesToggle_->setCheckable(true);
        capabilitiesToggle_->setArrowType(Qt::RightArrow);
        capabilitiesToggle_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        panels->addWidget(capabilitiesToggle_, 0, Qt::AlignLeading);
        capabilitiesContent_ = new QWidget(details);
        auto* capabilitiesLayout = new QHBoxLayout(capabilitiesContent_);
        capabilitiesLayout->setContentsMargins({});
        raw_ = new QGroupBox(capabilitiesContent_);
        auto* rawLayout = new QVBoxLayout(raw_);
        controls_ = new QTableWidget(raw_);
        controls_->setColumnCount(static_cast<int>(std::size(RawColumnKeys)));
        controls_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        controls_->setSelectionMode(QAbstractItemView::NoSelection);
        controls_->verticalHeader()->hide();
        controls_->horizontalHeader()->setSectionResizeMode(ControlColumn, QHeaderView::Stretch);
        controls_->horizontalHeader()->setSectionResizeMode(ValueColumn, QHeaderView::ResizeToContents);
        controls_->horizontalHeader()->setSectionResizeMode(ActiveColumn, QHeaderView::ResizeToContents);
        rawLayout->addWidget(controls_);
        capabilitiesLayout->addWidget(raw_, 3);
        capabilities_ = new QGroupBox(capabilitiesContent_);
        capabilityLayout_ = new QFormLayout(capabilities_);
        capabilityLayout_->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        for (auto& row : capabilityRows_) {
            row = {new QLabel(capabilities_), new QLabel(capabilities_)};
            row.value->setTextFormat(Qt::PlainText);
            row.value->setWordWrap(true);
            row.value->setTextInteractionFlags(Qt::TextSelectableByMouse);
            capabilityLayout_->addRow(row.label, row.value);
        }
        auto* diagnostic = new QVBoxLayout;
        diagnostic->addWidget(capabilities_);
        identity_ = new QGroupBox(capabilitiesContent_);
        auto* identityLayout = new QVBoxLayout(identity_);
        identityDetails_ = new QLabel(identity_);
        identityDetails_->setTextFormat(Qt::PlainText);
        identityDetails_->setTextInteractionFlags(Qt::TextSelectableByMouse);
        identityDetails_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        identityDetails_->setWordWrap(true);
        identityLayout->addWidget(identityDetails_);
        diagnostic->addWidget(identity_);
        diagnostic->addStretch();
        capabilitiesLayout->addLayout(diagnostic, 2);
        panels->addWidget(capabilitiesContent_, 1);
        capabilitiesContent_->hide();
        capabilitiesToggle_->hide();
        connect(capabilitiesToggle_, &QToolButton::toggled, this, [this](bool expanded) {
            capabilitiesToggle_->setArrowType(expanded ? Qt::DownArrow : Qt::RightArrow);
            capabilitiesContent_->setVisible(expanded);
            if (expanded) applySelection();
        });
        createRumble(details);
        panels->addWidget(rumble_);
        panels->addStretch();
        raw_->hide();
        rumble_->hide();
        error_ = new QLabel(this);
        error_->setWordWrap(true);
        error_->hide();
        layout->addWidget(error_);

        zones_->hide();
        live_->hide();
        connect(devices_, &QComboBox::currentIndexChanged, this, [this] { applySelection(); });
        connect(detect_, &QPushButton::clicked, this, [this] { sample(); });
        connect(&timer_, &QTimer::timeout, this, [this] { sample(); });
        refreshLabels();
        translation_ = translations_.subscribe([this] { refreshLabels(); });
    }

protected:
    void showEvent(QShowEvent* event) override {
        QWidget::showEvent(event);
        sample();
        timer_.start(LiveIntervalMs);
    }
    void hideEvent(QHideEvent* event) override {
        timer_.stop();
        finishRumble();
        save();
        source_.reset();
        QWidget::hideEvent(event);
    }

private:
    void createRumble(QWidget* parent) {
        rumble_ = new QGroupBox(parent);
        auto* layout = new QVBoxLayout(rumble_);
        auto* row = new QHBoxLayout;
        rumbleLabel_ = new QLabel(rumble_);
        rumbleStrength_ = new QSlider(Qt::Horizontal, rumble_);
        rumbleStrength_->setRange(RumbleMinPercent, RumbleMaxPercent);
        rumbleStrength_->setSingleStep(RumbleMinPercent);
        rumbleStrength_->setPageStep(RumbleMinPercent);
        rumbleStrength_->setValue(InitialRumblePercent);
        rumbleLabel_->setBuddy(rumbleStrength_);
        rumblePercent_ = new QLabel(rumble_);
        row->addWidget(rumbleLabel_);
        row->addWidget(rumbleStrength_, 1);
        row->addWidget(rumblePercent_);
        layout->addLayout(row);
        auto* grid = new QGridLayout;
        for (std::size_t i = 0; i < motors_.size(); ++i) {
            motors_[i] = new QPushButton(rumble_);
            grid->addWidget(motors_[i], int(i) / RumbleColumns, int(i) % RumbleColumns);
            connect(motors_[i], &QPushButton::clicked, this, [this, i] { pulse(Motors[i]); });
        }
        layout->addLayout(grid);
        rumbleStatus_ = new QLabel(rumble_);
        layout->addWidget(rumbleStatus_);
        rumbleTimer_.setSingleShot(true);
        connect(&rumbleTimer_, &QTimer::timeout, this, [this] { finishRumble(TestCompletedKey); });
        connect(rumbleStrength_, &QSlider::valueChanged, this, [this] {
            const int value = rumbleStrength_->value();
            const int stepped = ((value + RumbleMinPercent / 2) / RumbleMinPercent) * RumbleMinPercent;
            const QSignalBlocker block(rumbleStrength_);
            rumbleStrength_->setValue(stepped);
            updatePercentages();
        });
    }

    void updateRumbleControls() {
        rumble_->setVisible(motorCapabilities_ != 0);
        for (std::size_t i = 0; i < motors_.size(); ++i)
            motors_[i]->setEnabled(!rumbleTimer_.isActive() && (motorCapabilities_ & Motors[i].capability) != 0);
    }

    void setRumbleStatus(const char* key) {
        rumbleStatusKey_ = key;
        rumbleStatus_->setText(key ? text(key) : QString{});
    }

    void pulse(const Motor& motor) {
        if (!source_ || selected_.isEmpty() || !(motorCapabilities_ & motor.capability) || rumbleTimer_.isActive())
            return;
        Platform::GamepadRumble output;
        output.*motor.member = float(rumbleStrength_->value()) / RumbleMaxPercent;
        if (!source_->updateRumble({{selected_, output}})) {
            setRumbleStatus(TestFailedKey);
            return;
        }
        rumbleTimer_.start(RumblePulseMs);
        setRumbleStatus(TestRunningKey);
        updateRumbleControls();
    }

    void finishRumble(const char* status = nullptr) {
        if (source_) source_->stopRumble();
        rumbleTimer_.stop();
        setRumbleStatus(status);
        updateRumbleControls();
    }

    static ControlDescription controlDescription(const Platform::GamepadState& device, const QString& name) {
        if (device.controlDescriptions) {
            const auto control = device.controlDescriptions->constFind(name);
            if (control != device.controlDescriptions->cend()) return *control;
        }
        for (std::size_t kind = 0; kind < std::size(ControlTypes); ++kind) {
            const auto prefix = ControlTypes[kind].first;
            if (!name.startsWith(prefix, Qt::CaseInsensitive)) continue;
            bool valid = false;
            const auto index = name.sliced(prefix.size()).toInt(&valid);
            if (valid && index >= 0) {
                const bool oneBased = kind == ControlDescription::Button && index > 0 &&
                    device.deviceId.startsWith(RawInputDevicePrefix, Qt::CaseInsensitive);
                return {static_cast<ControlDescription::Kind>(kind), index - int(oneBased)};
            }
        }
        return {ControlDescription::Other, 0};
    }

    QString unknownControl(int id) const {
        return QLatin1String(UnknownControlFormat).arg(text(UnknownKey), QLocale().toString(id));
    }

    QString controlLabel(int label) const {
        constexpr int NoLabel = 0, UnknownLabel = -1;
        if (label == NoLabel || label == UnknownLabel) return {};
        const auto found = std::ranges::lower_bound(ControlLabelGlyphs, label, {}, &std::pair<int, const char*>::first);
        if (found != std::end(ControlLabelGlyphs) && found->first == label) return QString::fromUtf8(found->second);
        if (label >= LetterALabel && label <= LetterZLabel) return QChar(static_cast<char16_t>(u'A' + label - LetterALabel));
        if (label >= Number0Label && label <= Number9Label) return QChar(static_cast<char16_t>(u'0' + label - Number0Label));
        return unknownControl(label);
    }

    QString controlName(const QString& technicalName, const ControlDescription& control) const {
        if (control.kind == ControlDescription::Other) return technicalName;
        const auto index = QLocale().toString(control.index + 1);
        const auto name = QLatin1String(NumberedControlFormat).arg(text(ControlTypes[control.kind].second), index);
        const auto label = controlLabel(control.label);
        return label.isEmpty() ? name : label + QString::fromUtf8(Separator) + name;
    }

    QString controlDefinition(const QString& name, const ControlDescription& control) const {
        auto result = controlName(name, control);
        if (control.kind != ControlDescription::Switch) return result;
        if (control.switchKind >= 0 && static_cast<std::size_t>(control.switchKind) < std::size(SwitchKinds))
            result += QString::fromUtf8(Separator) + QString::fromUtf8(SwitchKinds[control.switchKind]);
        else if (!control.switchLabels.isEmpty())
            result += QString::fromUtf8(Separator) + text(UnknownKey);
        QStringList labels;
        for (const auto rawLabel : control.switchLabels) {
            const auto label = controlLabel(rawLabel);
            if (!label.isEmpty() && !labels.contains(label)) labels.append(label);
        }
        if (!labels.isEmpty()) result += QString::fromUtf8(Separator) + labels.join(QLatin1Char(' '));
        return result;
    }

    QString controlValue(const ControlDescription& control, float value) const {
        switch (control.kind) {
        case ControlDescription::Button: return QString::number(int(value > 0.5f));
        case ControlDescription::Switch: {
            const auto position = static_cast<int>(value);
            return position >= 0 && static_cast<std::size_t>(position) < std::size(SwitchPositions)
                ? QString::fromUtf8(SwitchPositions[position]) : unknownControl(position);
        }
        default: return QLocale().toString(value, 'f', AxisPrecision);
        }
    }

    bool updateRawControls(const Platform::GamepadState& device) {
        const auto& values = device.controls;
        raw_->setVisible(!values.isEmpty());
        const bool keysChanged = device.controlDescriptions != shownControlDescriptions_ ||
            device.deviceId != shownControlsDevice_ || values.size() != rawKeys_.size() ||
            !std::ranges::all_of(rawKeys_, [&](const auto& name) { return values.contains(name); });
        if (keysChanged) {
            shownControlsDevice_ = device.deviceId;
            shownControlDescriptions_ = device.controlDescriptions;
            rawKeys_ = values.keys();
            QHash<QString, ControlDescription> descriptions;
            descriptions.reserve(rawKeys_.size());
            for (const auto& key : rawKeys_) descriptions.insert(key, controlDescription(device, key));
            std::ranges::sort(rawKeys_, [&](const QString& left, const QString& right) {
                const auto& a = *descriptions.constFind(left);
                const auto& b = *descriptions.constFind(right);
                if (a.kind != b.kind) return a.kind < b.kind;
                if (a.index != b.index) return a.index < b.index;
                return left.compare(right, Qt::CaseInsensitive) < 0;
            });
            rawDescriptions_.clear();
            controls_->setRowCount(int(rawKeys_.size()));
            for (qsizetype row = 0; row < rawKeys_.size(); ++row) {
                const auto& key = rawKeys_[row];
                const auto& description = *descriptions.constFind(key);
                rawDescriptions_.append(description);
                controls_->setItem(int(row), ControlColumn, new QTableWidgetItem(controlName(key, description)));
                controls_->setItem(int(row), ValueColumn, new QTableWidgetItem);
                auto* active = new QTableWidgetItem;
                active->setFlags(Qt::ItemIsEnabled);
                controls_->setItem(int(row), ActiveColumn, active);
            }
        }
        for (qsizetype row = 0; row < rawKeys_.size(); ++row) {
            const auto& name = rawKeys_[row];
            const auto value = values.value(name);
            const auto formatted = controlValue(rawDescriptions_[row], value);
            auto* item = controls_->item(int(row), ValueColumn);
            if (item->text() != formatted) item->setText(formatted);
            controls_->item(int(row), ActiveColumn)->setCheckState(
                Platform::RawGamepad::direction(name, value) == 0 ? Qt::Unchecked : Qt::Checked);
        }
        return keysChanged;
    }

    QString text(const char* key) const { return QString::fromStdString(translations_.text(key)); }
    void refreshLabels() {
        deviceLabel_->setText(text(DeviceKey));
        modelLabel_->setText(text(VisualModelKey));
        const auto selection = models_->currentData();
        models_->clear();
        models_->addItem(text(ModelAutoKey), AutomaticModel);
        for (std::size_t i = 0; i < std::size(ModelKeys); ++i)
            models_->addItem(text(ModelKeys[i]), static_cast<int>(i));
        if (selection.isValid()) models_->setCurrentIndex(models_->findData(selection));
        detect_->setText(text(DetectKey));
        zones_->setTitle(text(DeadZonesKey));
        live_->setTitle(text(AnalogValuesKey));
        raw_->setTitle(text(LiveInputsKey));
        capabilitiesToggle_->setText(text(CapabilitiesKey));
        capabilities_->setTitle(text(CapabilitiesKey));
        identity_->setTitle(text(IdentityKey));
        for (std::size_t i = 0; i < capabilityRows_.size(); ++i)
            capabilityRows_[i].label->setText(text(CapabilityKeys[i]));
        QStringList columns;
        for (const auto* key : RawColumnKeys) columns.append(text(key));
        controls_->setHorizontalHeaderLabels(columns);
        rumble_->setTitle(text(RumbleKey));
        rumbleLabel_->setText(text(RumbleStrengthKey));
        for (std::size_t i = 0; i < motors_.size(); ++i) motors_[i]->setText(text(Motors[i].key));
        setRumbleStatus(rumbleStatusKey_);
        error_->setText(text(ErrorKeys::CannotSavePreferences));
        for (std::size_t i = 0; i < fields_.size(); ++i) fields_[i].label->setText(text(Fields[i].key));
        const auto sample = std::ranges::find(samples_, selected_, &Platform::GamepadState::deviceId);
        updateAnalogValues(sample == samples_.end() ? nullptr : &*sample);
        if (sample != samples_.end()) {
            updateIdentitySummary(&*sample);
            if (capabilitiesToggle_->isChecked()) {
                for (qsizetype row = 0; row < rawKeys_.size(); ++row)
                    controls_->item(int(row), ControlColumn)->setText(controlName(rawKeys_[row], rawDescriptions_[row]));
                updateCapabilities(*sample);
            }
        }
        updateCount();
        updatePercentages();
    }
    void updateCount() {
        count_->setText(devices_->count() == 0 ? text(NoneDetectedKey)
            : text(DetectedCountKey).replace(QLatin1String(CountArgument), QLocale().toString(devices_->count())));
    }
    void updatePercentages() {
        rumblePercent_->setText(QString::fromLatin1(PercentFormat).arg(QLocale().toString(rumbleStrength_->value())));
        for (const auto& field : fields_)
            field.percent->setText(QString::fromLatin1(PercentFormat).arg(QLocale().toString(field.slider->value())));
    }
    Analog::Profile readSliders() const {
        Analog::Profile profile;
        for (std::size_t i = 0; i < fields_.size(); ++i) profile.*Fields[i].member = fields_[i].slider->value();
        return profile;
    }
    void save() {
        if (!dirty_ || selected_.isEmpty()) return;
        if (Analog::save(selected_, draft_)) {
            dirty_ = false;
            error_->hide();
        } else error_->show();
    }
    void chooseModel(int index) {
        const auto id = models_->itemData(index).toInt();
        const auto profile = id == AutomaticModel ? std::nullopt
            : std::optional(VisualProfiles::Profile{static_cast<HostModel>(id), models_->itemText(index)});
        error_->setVisible(!VisualProfiles::save(selected_, profile));
        sample();
    }

    void updateArtwork(const Platform::GamepadState* device) {
        const auto profile = device ? VisualProfiles::profile(device->deviceId) : std::nullopt;
        const auto matched = device ? ControllerArtwork::hostModel(*device) : ControllerArtwork::HostMatch{};
        modelRow_->setVisible(device && (profile || !matched.exact));
        if (device) {
            const int row = models_->findData(profile ? static_cast<int>(profile->model) : AutomaticModel);
            if (models_->currentIndex() != row) models_->setCurrentIndex(row);
        }
        const auto model = device ? std::optional(profile ? profile->model : matched.model) : std::nullopt;
        if (model == artworkModel_) {
            if (artwork_) {
                artwork_->setAccessibleName(displayName(*device));
                artwork_->setHostInput(*device);
            }
            return;
        }
        artworkModel_ = model;
        delete artwork_;
        artwork_ = model ? ControllerArtwork::createHost(*model, artworkPanel_) : nullptr;
        if (artwork_) {
            artwork_->setAccessibleName(displayName(*device));
            artwork_->setHostInput(*device);
            artworkLayout_->addWidget(artwork_, 0, Qt::AlignVCenter);
        }
        artworkPanel_->setVisible(artwork_ != nullptr);
    }
    void applySelection() {
        const auto selected = devices_->currentData().toString();
        if (selected_ != selected) {
            capabilitiesCurrent_ = false;
            finishRumble();
            save();
            selected_ = selected;
            dirty_ = false;
            draft_ = Analog::profile(selected_);
            for (std::size_t i = 0; i < fields_.size(); ++i) {
                auto* slider = fields_[i].slider;
                const QSignalBlocker blocker(slider);
                slider->setValue(draft_.*Fields[i].member);
            }
            updatePercentages();
        }
        const auto device = std::ranges::find(samples_, selected_, &Platform::GamepadState::deviceId);
        const bool connected = device != samples_.end();
        updateArtwork(connected ? &*device : nullptr);
        updateIdentitySummary(connected ? &*device : nullptr);
        const auto rumble = std::ranges::find(rumbleDevices_, selected_, &Platform::GamepadRumbleDevice::deviceId);
        const quint32 capabilities = connected && rumble != rumbleDevices_.end() ? rumble->motors : 0;
        if (capabilities != motorCapabilities_) {
            capabilitiesCurrent_ = false;
            motorCapabilities_ = capabilities;
            if (rumbleTimer_.isActive()) finishRumble(TestFailedKey);
            else updateRumbleControls();
        }
        capabilitiesToggle_->setVisible(connected);
        capabilitiesContent_->setVisible(connected && capabilitiesToggle_->isChecked());
        if (connected && capabilitiesToggle_->isChecked()) {
            const bool controlsChanged = updateRawControls(*device);
            if (!capabilitiesCurrent_ || controlsChanged ||
                device->controlCounts != shownControlCounts_ || device->capabilities != shownCapabilities_ ||
                device->identity != shownIdentity_) {
                updateCapabilities(*device);
                shownControlCounts_ = device->controlCounts;
                shownCapabilities_ = device->capabilities;
                shownIdentity_ = device->identity;
                capabilitiesCurrent_ = true;
            }
        } else {
            capabilitiesCurrent_ = false;
        }
        zones_->setVisible(connected && device->standardGamepad);
        updateAnalogValues(connected ? &*device : nullptr);
    }

    static QString hexPair(quint16 first, quint16 second) {
        return QString::fromLatin1(HexPair).arg(first, IdentifierWidth, HexBase, QLatin1Char('0'))
            .arg(second, IdentifierWidth, HexBase, QLatin1Char('0')).toUpper();
    }

    QString deviceFamily(Platform::GamepadIdentity::Family family) const {
        using Identity = Platform::GamepadIdentity;
        switch (family) {
        case Identity::XboxOne: return QStringLiteral("Xbox One / GIP");
        case Identity::Xbox360: return QStringLiteral("Xbox 360 / XInput");
        case Identity::Hid: return QStringLiteral("HID");
        case Identity::I8042: return QStringLiteral("i8042");
        case Identity::Aggregate: return text("Controllers.Enum.Aggregate");
        case Identity::Virtual: return text("Controllers.Enum.Virtual");
        default: return text(UnknownKey);
        }
    }

    QString deviceUsage(const Platform::GamepadIdentity& id) const {
        constexpr quint16 GenericDesktop = 0x01;
        constexpr std::pair<quint16, const char*> usages[]{{0x02, "Emulation.Tab.Mouse"}, {0x04, "Emulation.Controller.Category.Joystick"},
            {0x05, "Controllers.Enum.Gamepad"}, {0x06, "Emulation.Tab.Keyboard"}, {0x08, "Controllers.Enum.MultiAxisController"}};
        const auto usage = std::ranges::find(usages, id.usage, &std::pair<quint16, const char*>::first);
        const auto name = text(id.usagePage == GenericDesktop && usage != std::end(usages)
            ? usage->second : "Controllers.Enum.HidUsage");
        return name + QString::fromUtf8(Separator) + hexPair(id.usagePage, id.usage);
    }

    void updateIdentitySummary(const Platform::GamepadState* device) {
        const auto summary = device && device->identity
            ? hexPair(device->vendorId, device->productId) + QString::fromUtf8(Separator) + deviceFamily(device->identity->family)
            : QString{};
        identitySummary_->setVisible(!summary.isEmpty());
        if (identitySummary_->text() != summary) identitySummary_->setText(summary);
    }

    void updateIdentityDetails(const Platform::GamepadState& device) {
        const auto& id = device.identity;
        identity_->setVisible(bool(id));
        if (!id) {
            identityDetails_->clear();
            return;
        }
        QStringList lines;
        const auto add = [&](const QString& label, const QString& value) {
            lines.append(QLatin1String(IdentityLine).arg(label, value));
        };
        add(text("Controllers.GameInputName"), id->name);
        add(QStringLiteral("VID:PID"), hexPair(device.vendorId, device.productId));
        add(text("Controllers.Revision"), QLocale().toString(id->revision));
        add(text("Controllers.HardwareVersion"), id->hardwareVersion);
        add(text("Controllers.FirmwareVersion"), id->firmwareVersion);
        add(text("Controllers.Family"), deviceFamily(id->family));
        add(text("Controllers.Usage"), deviceUsage(*id));
        add(QStringLiteral("DeviceId"), device.deviceId.startsWith(GameInputIdPrefix, Qt::CaseInsensitive)
            ? device.deviceId.sliced(GameInputIdPrefix.size()) : device.deviceId);
        add(QStringLiteral("DeviceRootId"), id->rootId);
        add(QStringLiteral("ContainerId"), id->containerId);
        add(QStringLiteral("PnP"), id->pnpPath);
        add(text("Controllers.Manufacturer"), id->manufacturer);
        if (!id->windowsChain.isEmpty()) {
            lines.append(QString{});
            lines.append(text("Controllers.WindowsIdentity"));
            lines.append(id->windowsChain);
        }
        identityDetails_->setText(lines.join(QChar::LineFeed));
    }

    void updateCapabilities(const Platform::GamepadState& device) {
        updateIdentityDetails(device);
        const auto set = [&](CapabilityRow row, const QString& value) {
            const auto& field = capabilityRows_[row];
            capabilityLayout_->setRowVisible(field.label, !value.isEmpty());
            if (field.value->text() != value) field.value->setText(value);
        };
        const auto join = [this](const QStringList& values) {
            return values.isEmpty() ? text(NoneKey) : values.join(QString::fromUtf8(Separator));
        };
        const auto number = [](quint32 value) { return QLocale().toString(value); };
        set(StatusRow, QString::fromUtf8(ConnectedFormat).arg(text(DeviceKey)));
        const auto& counts = device.controlCounts;
        const auto countText = text(ControlCountsKey).arg(number(counts.axes), number(counts.buttons), number(counts.switches));
        QStringList definitions{countText};
        for (qsizetype row = 0; row < rawKeys_.size(); ++row)
            definitions.append(controlDefinition(rawKeys_[row], rawDescriptions_[row]));
        set(ControlsRow, definitions.join(QChar::LineFeed));
        QStringList motors;
        for (const auto& motor : Motors)
            if (motorCapabilities_ & motor.capability) motors.append(text(motor.key));
        if (!motors.isEmpty()) motors.prepend(number(static_cast<quint32>(motors.size())));
        set(RumbleRow, join(motors));

        const auto& caps = device.capabilities;
        if (!caps) {
            for (const auto row : {InputKindsRow, ForceFeedbackRow, HapticsRow, SystemButtonsRow, RawReportsRow,
                    GamepadLayoutRow, WheelRow, ExtraControlsRow, HapticEndpointRow, HapticLocationsRow}) set(row, {});
            return;
        }
        QStringList inputKinds;
        auto remaining = caps->inputKinds;
        for (const auto& [kind, key] : InputKinds) {
            if (remaining & kind) inputKinds.append(text(key));
            remaining &= ~static_cast<quint32>(kind);
        }
        if (remaining != 0) inputKinds.append(QLatin1String(UnknownFlagFormat).arg(QString::number(remaining, 16).toUpper()));
        set(InputKindsRow, join(inputKinds));
        const auto yesNo = [&](bool value) { return text(value ? YesKey : NoKey); };
        QStringList feedback;
        for (qsizetype index = 0; index < caps->feedbackDetails.size(); ++index) {
            const auto& motor = caps->feedbackDetails[index];
            QStringList axes, effects;
            auto remainingAxes = motor.axes;
            constexpr auto directionCount = std::size(AxisDirections);
            for (std::size_t bit = 0; bit < directionCount * 2; ++bit) {
                const auto flag = 1u << bit;
                if (remainingAxes & flag) {
                    axes.append(QLatin1String(AxisDirectionFormat).arg(text(bit < directionCount ? LinearKey : AngularKey),
                        QLatin1String(AxisDirections[bit % directionCount])));
                    remainingAxes &= ~flag;
                }
            }
            constexpr auto normal = 1u << (directionCount * 2);
            if (remainingAxes & normal) axes.append(text(NormalKey));
            remainingAxes &= ~normal;
            if (remainingAxes) axes.append(QLatin1String(UnknownFlagFormat).arg(QString::number(remainingAxes, 16).toUpper()));
            for (std::size_t bit = 0; bit < std::size(FeedbackEffects); ++bit)
                if (motor.effects & (1u << bit)) effects.append(text(FeedbackEffects[bit]));
            feedback.append(QString::fromUtf8(FeedbackFormat).arg(QLocale().toString(index + 1), yesNo(motor.poweredOn),
                join(axes), effects.isEmpty() ? text(NoneKey) : effects.join(QLatin1String(ListSeparator))));
        }
        const auto motorCount = number(caps->forceFeedbackMotors);
        set(ForceFeedbackRow, !caps->forceFeedbackMotors ? text(NoneKey) :
            feedback.isEmpty() ? motorCount : motorCount + QString::fromUtf8(Separator) + feedback.join(QChar::LineFeed));
        set(HapticsRow, caps->hapticsQueryable ? yesNo(caps->haptics.has_value()) : QString{});
        set(HapticEndpointRow, caps->haptics ? caps->haptics->endpoint : QString{});
        set(HapticLocationsRow, caps->haptics && !caps->haptics->locations.isEmpty()
            ? join(caps->haptics->locations) : QString{});
        QStringList systemButtons;
        if (caps->systemButtons & Capabilities::Guide) systemButtons.append(QString::fromUtf8(GuideGlyph));
        if (caps->systemButtons & Capabilities::Share) systemButtons.append(text(ShareKey));
        set(SystemButtonsRow, join(systemButtons));
        QStringList reports{text(ReportCountsKey).arg(number(caps->inputReportCount), number(caps->outputReportCount))};
        const auto addReports = [&](const QList<Capabilities::Report>& source, const char* key) {
            for (qsizetype index = 0; index < source.size(); ++index)
                reports.append(QLatin1String(ReportFormat).arg(text(key), QLocale().toString(index + 1),
                    number(source[index].id), number(source[index].size)));
        };
        addReports(caps->inputReports, InputReportKey);
        addReports(caps->outputReports, OutputReportKey);
        set(RawReportsRow, join(reports));
        QStringList layout;
        if (caps->gamepadLayout) {
            auto remainingLayout = *caps->gamepadLayout;
            for (std::size_t bit = 0; bit < std::size(GamepadLayoutGlyphs); ++bit) {
                const auto flag = 1u << bit;
                if (remainingLayout & flag) layout.append(QString::fromUtf8(GamepadLayoutGlyphs[bit]));
                remainingLayout &= ~flag;
            }
            if (remainingLayout)
                layout.append(QLatin1String(UnknownFlagFormat).arg(QString::number(remainingLayout, 16).toUpper()));
        }
        set(GamepadLayoutRow, caps->gamepadLayout ? join(layout) : QString{});
        QStringList extra;
        for (const auto& controls : caps->extraControls) {
            const auto kind = std::ranges::find(InputKinds, controls.kind, &std::pair<Capabilities::InputKind, const char*>::first);
            if (kind == std::end(InputKinds)) continue;
            const auto append = [&](const QList<quint8>& indexes, const char* key) {
                if (indexes.isEmpty()) return;
                QStringList positions;
                for (const auto index : indexes) positions.append(number(quint32{index} + 1));
                extra.append(QLatin1String(ExtraControlsFormat).arg(text(kind->second), text(key),
                    positions.join(QLatin1String(ListSeparator))));
            };
            append(controls.axes, ExtraAxesKey);
            append(controls.buttons, ExtraButtonsKey);
        }
        set(ExtraControlsRow, extra.isEmpty() ? QString{} : join(extra));
        set(WheelRow, caps->wheel ? text(WheelDetailsKey).arg(QLocale().toString(caps->wheel->maxAngle),
            yesNo(caps->wheel->clutch), yesNo(caps->wheel->handbrake), yesNo(caps->wheel->shifter)) : QString{});
    }

    void updateAnalogValues(const Platform::GamepadState* device) {
        qsizetype row = 0;
        if (device) {
            const QLocale locale;
            const auto add = [&](const QString& label, const QString& value) {
                if (row == axes_.size()) {
                    auto* name = new QLabel(live_);
                    name->setTextFormat(Qt::PlainText);
                    auto* reading = new QLabel(live_);
                    reading->setTextFormat(Qt::PlainText);
                    analogValues_->addRow(name, reading);
                    axes_.append(Axis{name, reading});
                }
                const auto& axis = axes_[row++];
                if (axis.label->text() != label) axis.label->setText(label);
                if (axis.value->text() != value) axis.value->setText(value);
            };
            const auto addAxis = [&](const char* key, float value) { add(text(key), locale.toString(value, 'f', AxisPrecision)); };
            if (device->standardGamepad) {
                for (std::size_t i = 0; i < std::size(AxisKeys); ++i)
                    addAxis(AxisKeys[i], device->axes.value(static_cast<qsizetype>(i)));
            }
            if (const auto& wheel = device->racingWheel) {
                addAxis(SteeringKey, wheel->wheel);
                addAxis(ThrottleKey, wheel->throttle);
                addAxis(BrakeKey, wheel->brake);
                addAxis(ClutchKey, wheel->clutch);
                addAxis(HandbrakeKey, wheel->handbrake);
                add(text(GearKey), locale.toString(wheel->gear));
            }
            if (const auto& flight = device->flightStick) {
                addAxis(RollKey, flight->roll);
                addAxis(PitchKey, flight->pitch);
                addAxis(YawKey, flight->yaw);
                addAxis(ThrottleKey, flight->throttle);
            }
            if (row == 0) {
                constexpr auto prefix = QLatin1String(Platform::RawGamepad::AxisPrefix);
                QList<QString> rawAxes;
                for (auto it = device->controls.cbegin(); it != device->controls.cend(); ++it)
                    if (Platform::RawGamepad::isAxis(it.key())) rawAxes.append(it.key());
                std::ranges::sort(rawAxes, [](const QString& left, const QString& right) {
                    return left.sliced(prefix.size()).toInt() < right.sliced(prefix.size()).toInt();
                });
                for (const auto& name : rawAxes)
                    add(controlName(name, controlDescription(*device, name)), locale.toString(device->controls.value(name), 'f', AxisPrecision));
            }
        }
        while (axes_.size() > row) {
            analogValues_->removeRow(static_cast<int>(axes_.size() - 1));
            axes_.removeLast();
        }
        live_->setVisible(row > 0);
    }
    void sample() {
        if (!isVisible()) return;
        if (!source_) source_ = std::make_unique<Platform::GamepadSource>();
        samples_ = Analog::read(*source_);
        rumbleDevices_ = source_->rumbleDevices();
        std::ranges::sort(samples_, [this](const auto& left, const auto& right) {
            const int order = displayName(left).compare(displayName(right), Qt::CaseInsensitive);
            return order == 0 ? left.deviceId < right.deviceId : order < 0;
        });
        const auto matches = [&] {
            if (devices_->count() != samples_.size()) return false;
            for (int i = 0; i < devices_->count(); ++i)
                if (devices_->itemData(i).toString() != samples_[i].deviceId ||
                    devices_->itemText(i) != displayName(samples_[i])) return false;
            return true;
        };
        if (!matches()) {
            const QSignalBlocker blocker(devices_);
            devices_->clear();
            for (const auto& sample : samples_) devices_->addItem(displayName(sample), sample.deviceId);
            const int previous = devices_->findData(selected_);
            if (previous >= 0) devices_->setCurrentIndex(previous);
            updateCount();
        }
        applySelection();
    }
    QString displayName(const Platform::GamepadState& state) const {
        if (const auto profile = VisualProfiles::profile(state.deviceId); profile && !profile->displayName.isEmpty())
            return profile->displayName;
        return state.displayName.isEmpty() ? state.deviceId : state.displayName;
    }

    QFormLayout* analogValues_;
    struct Field { QLabel* label; QSlider* slider; QLabel* percent; };
    struct Axis { QLabel* label; QLabel* value; };
    Translations::TranslationService& translations_;
    QComboBox* devices_;
    QWidget* modelRow_;
    QLabel* modelLabel_;
    QComboBox* models_;
    QWidget* artworkPanel_;
    QVBoxLayout* artworkLayout_;
    ControllerArtwork* artwork_ = nullptr;
    std::optional<ControllerArtwork::HostModel> artworkModel_;
    QLabel* deviceLabel_;
    QLabel* count_;
    QPushButton* detect_;
    QGroupBox* zones_;
    QGroupBox* live_;
    QLabel* error_;
    QToolButton* capabilitiesToggle_;
    QWidget* capabilitiesContent_;
    QGroupBox* capabilities_;
    QGroupBox* identity_;
    QLabel* identitySummary_;
    QLabel* identityDetails_;
    std::shared_ptr<const Platform::GamepadIdentity> shownIdentity_;
    QFormLayout* capabilityLayout_;
    std::array<Axis, CapabilityRowCount> capabilityRows_{};
    Platform::GamepadControlCounts shownControlCounts_{};
    std::optional<Capabilities> shownCapabilities_;
    bool capabilitiesCurrent_ = false;
    QGroupBox* raw_;
    QTableWidget* controls_;
    QStringList rawKeys_;
    QList<ControlDescription> rawDescriptions_;
    QString shownControlsDevice_;
    std::shared_ptr<const QHash<QString, ControlDescription>> shownControlDescriptions_;
    QGroupBox* rumble_;
    QLabel* rumbleLabel_;
    QLabel* rumblePercent_;
    QLabel* rumbleStatus_;
    QSlider* rumbleStrength_;
    std::array<QPushButton*, std::size(Motors)> motors_{};
    const char* rumbleStatusKey_ = nullptr;
    quint32 motorCapabilities_ = 0;
    QTimer rumbleTimer_;
    QList<Platform::GamepadRumbleDevice> rumbleDevices_;
    std::array<Field, std::size(Fields)> fields_{};
    QList<Axis> axes_;
    QTimer timer_;
    std::unique_ptr<Platform::GamepadSource> source_;
    QList<Platform::GamepadState> samples_;
    QString selected_;
    Analog::Profile draft_;
    bool dirty_ = false;
    Translations::TranslationService::Subscription translation_;
};
}

QWidget* createEmulationControllerPanel(QWidget* parent, Translations::TranslationService& translations) {
    return new ControllerPanel(parent, translations);
}
}
