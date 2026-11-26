#pragma once

#include "ControllerFeedback.h"

namespace RetroMediaLab::Emulation {

enum class InputSource { None = 0, Keyboard = 1, Mouse = 2, Controller = 4 };
Q_DECLARE_FLAGS(InputSources, InputSource)
Q_DECLARE_OPERATORS_FOR_FLAGS(InputSources)

enum class ControllerCategory {
    Unspecified, None, Gamepad, Joystick, PointingDevice, FlightSimulation,
    RacingSimulation, DanceSimulation, SingingSimulation, OtherSimulation
};

enum class ControllerVisualControl
{
    DirectionUp,
    DirectionDown,
    DirectionLeft,
    DirectionRight,
    StickUp,
    StickDown,
    StickLeft,
    StickRight,
    CUp,
    CDown,
    CLeft,
    CRight,
    LeftStick,
    RightStick,
    Steering,
    Buzzer,
    Guide,
    FaceA,
    FaceB,
    FaceC,
    FaceX,
    FaceY,
    FaceZ,
    PrimaryAction,
    SecondaryAction,
    Turbo,
    TertiaryAction,
    QuaternaryAction,
    LeftShoulder,
    RightShoulder,
    LeftTrigger,
    RightTrigger,
    TouchPad,
    MouseLeftButton,
    MouseRightButton,
    Start,
    Pause,
    PlayPause,
    Stop,
    Rewind,
    FastForward,
    Keyboard,
    Reset,
    Key1,
    Key2,
    Key3,
    Key4,
    Key5,
    Key6,
    Key7,
    Key8,
    Key9,
    KeyStar,
    Key0,
    KeyHash,
    Option,
    AnalogMode,
    RightStickUp,
    RightStickDown,
    RightStickLeft,
    RightStickRight,
    Buzz1Red,
    Buzz1Blue,
    Buzz1Orange,
    Buzz1Green,
    Buzz1Yellow,
    Buzz2Red,
    Buzz2Blue,
    Buzz2Orange,
    Buzz2Green,
    Buzz2Yellow,
    Buzz3Red,
    Buzz3Blue,
    Buzz3Orange,
    Buzz3Green,
    Buzz3Yellow,
    Buzz4Red,
    Buzz4Blue,
    Buzz4Orange,
    Buzz4Green,
    Buzz4Yellow,
    FretGreen,
    FretRed,
    FretYellow,
    FretBlue,
    FretOrange,
    StrumUp,
    StrumDown,
    Whammy,
    RotaryLeft,
    RotaryRight,
    Slow,
    RepeatRate,
    StickMode,
    VibrationMode,
    Accelerator,
    BrakePedal,
    GearUp,
    GearDown,
    DialLeft,
    DialRight,
    DialPress,
    Audio,
    Shuffle,
    Angle,
    Program,
    Subtitle,
    Repeat,
    SlowBackward,
    SlowForward,
    Clear,
    Time,
    Previous,
    Next,
    RepeatAB,
    Display,
    Title,
    Menu,
    Return,
    Enter,
    PowerIncrease,
    PowerDecrease,
    BrakeIncrease,
    BrakeDecrease,
    CombinedLeverForward,
    CombinedLeverBackward,
    FaceD,
    Horn,
    HornPedal,
    Announcement,
    Camera,
    LeftDoor,
    RightDoor,
    ReverserForward,
    ReverserNeutral,
    ReverserBackward,
    Ats,
    CloseDoors,
    Key10,
    Key11,
    Key12,
    Key13,
    Key14,
    Key15,
    Key16,
    Key17,
    Key18,
    Key19,
    Key20,
    Key21,
    Key22,
    Key23,
    Key24,
    OpticalSensor1,
    OpticalSensor2,
    OpticalSensor3,
    OpticalSensor4,
    OpticalSensor5,
    SoloFretGreen,
    SoloFretRed,
    SoloFretYellow,
    SoloFretBlue,
    SoloFretOrange,
    EffectPosition1,
    EffectPosition2,
    EffectPosition3,
    EffectPosition4,
    EffectPosition5,
    DrumHiHat,
    DrumSnare,
    DrumHighTom,
    DrumLowTom,
    DrumCymbal,
    DrumKick,
    DrumRed,
    DrumYellow,
    DrumBlue,
    DrumGreen,
    DrumOrange,
    LeftTetherLeft,
    LeftTetherRight,
    LeftTetherForward,
    LeftTetherBackward,
    LeftTetherExtend,
    LeftTetherRetract,
    RightTetherLeft,
    RightTetherRight,
    RightTetherForward,
    RightTetherBackward,
    RightTetherExtend,
    RightTetherRetract,
    CrossfaderLeft,
    CrossfaderRight,
};

struct InputBindingDefinition {
    QString id, displayResourceKey, defaultBinding, invariantDisplayValue, oppositeDirectionId;
};

struct InputBindingSet {
    QList<InputBindingDefinition> definitions;
    QHash<QString, QString> values;
    InputSources sources;
    bool prefixKeyboardSource = false;
    std::optional<QStringList> compatibleVisualIds;
    QString defaultVisualId;
    QHash<ControllerVisualControl, QString> visualCommandIds;
};

struct ControllerOutputDefinition {
    QString id, displayResourceKey;
};

struct ControllerChoice {
    QString id, displayResourceKey, invariantDisplayValue;
    std::optional<QList<InputBindingDefinition>> bindingDefinitions;
    std::optional<QStringList> compatibleVisualIds;
    QString defaultVisualId;
    QHash<ControllerVisualControl, QString> visualCommandIds;
    QHash<QString, QList<InputBindingDefinition>> visualBindingDefinitions;
    ControllerCategory category = ControllerCategory::Unspecified;
    bool isSupportedByEmulator = true;
    QHash<QString, QList<ControllerOutputDefinition>> visualOutputDefinitions;
};

struct ControllerPortSettings {
    int number = 0;
    QList<ControllerChoice> controllerChoices;
    QString selectedControllerId, physicalDeviceId;
    InputBindingSet bindings;
    int deadZonePercent = 0;
    QString visualId, displayName;
    std::optional<int> displayNumber;
    bool isUsb = false;
    QHash<QString, ControllerOutputBinding> outputBindings;
};

// A value means model-specific output definitions govern this port, even if the active model has no outputs.
RETROMEDIALAB_EMULATION_API std::optional<ControllerPortFeedback> controllerModelFeedback(const ControllerPortSettings&);

struct InputSettings {
    std::optional<InputBindingSet> keyboard, mouse;
    QList<ControllerPortSettings> controllerPorts;
};

class IMachineInputBindings {
public:
    virtual ~IMachineInputBindings() = default;
    // Refresh the running machine from the configuration already saved by its module.
    virtual void updateInputBindings(const Configuration&) = 0;
};

class IMachineKeyboardBindings {
public:
    virtual ~IMachineKeyboardBindings() = default;
    // Keys are EmulationKey names; values are the configured host bindings.
    virtual const QHash<QString, QString>& keyboardBindings() const = 0;
};

class IModuleInputSettingsManager {
public:
    virtual ~IModuleInputSettingsManager() = default;
    virtual InputSettings describeInputSettings(const Configuration&) const = 0;
    virtual Configuration applyInputSettings(Configuration, const InputSettings&) = 0;
    virtual void saveInputSettings(const Configuration&, std::stop_token = {}) = 0;
};

}
