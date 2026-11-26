#pragma once

#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>
#include <memory>
#include <optional>

namespace RetroMediaLab::Platform {
enum GamepadButton : quint64 {
    NoButton = 0,
    Menu = 0x01,
    View = 0x02,
    Confirm = 0x04,
    Cancel = 0x08,
    X = 0x10,
    Y = 0x20,
    Up = 0x40,
    Down = 0x80,
    Left = 0x100,
    Right = 0x200,
    LeftShoulder = 0x400,
    RightShoulder = 0x800,
    LeftStick = 0x1000,
    RightStick = 0x2000,
    LeftTriggerButton = 0x00010000,
    RightTriggerButton = 0x00020000,
    PaddleLeft1 = 0x04000000,
    PaddleLeft2 = 0x08000000,
    PaddleRight1 = 0x10000000,
    PaddleRight2 = 0x20000000,
};

namespace RawGamepad {
inline constexpr auto AxisPrefix = "Axis";
inline constexpr auto ButtonPrefix = "Button";
inline constexpr auto SwitchPrefix = "Switch";
inline constexpr auto GuideButton = "XboxButton";
inline constexpr auto ShareButton = "Share";
inline constexpr float AxisCenter = 0.5f;
inline constexpr float NeutralAxisMin = 0.25f;
inline constexpr float NeutralAxisMax = 0.75f;

inline QString controlName(QLatin1StringView prefix, qsizetype index) {
    return prefix + QString::number(index);
}

inline bool isAxis(QStringView name) {
    constexpr auto prefix = QLatin1String(AxisPrefix);
    if (!name.startsWith(prefix, Qt::CaseInsensitive)) return false;
    bool valid = false;
    const auto index = name.sliced(prefix.size()).toInt(&valid);
    return valid && index >= 0;
}

inline int direction(QStringView name, float value) {
    if (!isAxis(name)) return value > 0.5f ? 1 : 0;
    return value > NeutralAxisMax ? 1 : value < NeutralAxisMin ? -1 : 0;
}
}

enum GamepadKind : quint8 {
    GamepadRacingWheel = 0x01,
    GamepadFlightStick = 0x02,
    GamepadArcadeStick = 0x04,
};

// Button masks and hat positions use the values defined by GameInput.
struct RacingWheelInput {
    enum Button : quint16 {
        Menu = 0x01, View = 0x02, PreviousGear = 0x04, NextGear = 0x08,
        Up = 0x10, Down = 0x20, Left = 0x40, Right = 0x80,
        A = 0x100, B = 0x200, X = 0x400, Y = 0x800,
        LeftThumbstick = 0x1000, RightThumbstick = 0x2000
    };
    quint16 buttons = 0;
    int gear = 0;
    float wheel = 0, throttle = 0, brake = 0, clutch = 0, handbrake = 0;
    bool operator==(const RacingWheelInput&) const = default;
};

struct FlightStickInput {
    enum Button : quint16 {
        Menu = 0x01, View = 0x02, PrimaryFire = 0x04, SecondaryFire = 0x08,
        A = 0x100, B = 0x200, X = 0x400, Y = 0x800,
        LeftShoulder = 0x1000, RightShoulder = 0x2000
    };
    quint16 buttons = 0;
    quint8 hat = 0;
    float roll = 0, pitch = 0, yaw = 0, throttle = 0;
    bool operator==(const FlightStickInput&) const = default;
};

struct ArcadeStickInput {
    enum Button : quint16 {
        Menu = 0x01, View = 0x02, Up = 0x04, Down = 0x08, Left = 0x10, Right = 0x20,
        Action1 = 0x40, Action2 = 0x80, Action3 = 0x100, Action4 = 0x200,
        Action5 = 0x400, Action6 = 0x800, Special1 = 0x1000, Special2 = 0x2000
    };
    quint16 buttons = 0;
    bool operator==(const ArcadeStickInput&) const = default;
};

struct GamepadControlCounts {
    quint32 axes = 0, buttons = 0, switches = 0;
    bool operator==(const GamepadControlCounts&) const = default;
};

struct GamepadCapabilities {
    // Values of GameInputKind, retained so unknown driver flags can be displayed.
    enum InputKind : quint32 {
        RawReports = 0x01, Axes = 0x02, Buttons = 0x04, Switches = 0x08, Keyboard = 0x10,
        Mouse = 0x20, Sensors = 0x40, Arcade = 0x10000, Flight = 0x20000,
        Gamepad = 0x40000, Racing = 0x80000, UiNavigation = 0x01000000
    };
    enum SystemButton : quint8 { Guide = 0x01, Share = 0x02 };
    struct Report {
        quint32 id, size;
        bool operator==(const Report&) const = default;
    };
    struct Wheel {
        float maxAngle;
        bool clutch, handbrake, shifter;
        bool operator==(const Wheel&) const = default;
    };
    quint32 inputKinds = 0, systemButtons = 0, forceFeedbackMotors = 0;
    quint32 inputReportCount = 0, outputReportCount = 0;
    QList<Report> inputReports{}, outputReports{};
    struct FeedbackMotor {
        quint32 axes = 0;
        quint16 effects = 0;
        bool poweredOn = false;
        bool operator==(const FeedbackMotor&) const = default;
    };
    struct ExtraControls {
        InputKind kind;
        QList<quint8> axes, buttons;
        bool operator==(const ExtraControls&) const = default;
    };
    struct Haptics {
        QString endpoint;
        QStringList locations;
        bool operator==(const Haptics&) const = default;
    };
    std::optional<Wheel> wheel{};
    QList<FeedbackMotor> feedbackDetails{};
    QList<ExtraControls> extraControls{};
    std::optional<quint32> gamepadLayout{};
    std::optional<Haptics> haptics{};
    bool hapticsQueryable = false;
    bool operator==(const GamepadCapabilities&) const = default;
};

struct GamepadIdentity {
    enum Family { Virtual = -1, Unknown, XboxOne, Xbox360, Hid, I8042, Aggregate };
    Family family = Unknown;
    quint16 revision = 0, usagePage = 0, usage = 0;
    QString name, hardwareVersion, firmwareVersion, rootId, containerId, pnpPath, manufacturer;
    QStringList windowsChain;
};

struct GamepadControlDescription {
    enum Kind { Axis, Button, Switch, Other };
    Kind kind;
    int index;
    int label = 0;
    int switchKind = -1;
    QList<int> switchLabels{};
};

struct GamepadState {
    quint64 id;
    quint64 buttons;
    QList<float> axes;
    QString deviceId;
    QHash<QString, float> controls;
    QString displayName{};
    bool standardGamepad = false;
    quint16 vendorId = 0;
    quint16 productId = 0;
    quint8 kinds = 0;
    std::optional<RacingWheelInput> racingWheel{};
    std::optional<FlightStickInput> flightStick{};
    std::optional<ArcadeStickInput> arcadeStick{};
    GamepadControlCounts controlCounts{};
    std::optional<GamepadCapabilities> capabilities{};
    std::shared_ptr<const GamepadIdentity> identity{};
    std::shared_ptr<const QHash<QString, GamepadControlDescription>> controlDescriptions{};
};

struct GamepadRumble {
    float lowFrequency = 0;
    float highFrequency = 0;
    float leftTrigger = 0;
    float rightTrigger = 0;
    bool operator==(const GamepadRumble&) const = default;
};

enum GamepadRumbleMotor : quint32 {
    RumbleLowFrequency = 0x01,
    RumbleHighFrequency = 0x02,
    RumbleLeftTrigger = 0x04,
    RumbleRightTrigger = 0x08,
};

struct GamepadRumbleDevice {
    QString deviceId;
    QString displayName;
    quint32 motors;
};

enum class GameInputInstallResult { Started, Cancelled, Failed };
bool canOfferGameInputInstaller();
GameInputInstallResult launchGameInputInstaller();

class GamepadSource {
public:
    GamepadSource();
    ~GamepadSource();
    QList<GamepadState> read();
    QList<GamepadRumbleDevice> rumbleDevices() const;
    bool updateRumble(const QHash<QString, GamepadRumble>& outputs);
    void stopRumble() noexcept;

private:
    struct State;
    std::unique_ptr<State> state_;
};
}
