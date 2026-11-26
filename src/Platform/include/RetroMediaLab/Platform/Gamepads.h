#pragma once

#include <QHash>
#include <QList>
#include <QString>
#include <memory>

namespace RetroMediaLab::Platform {
enum GamepadButton : quint64 {
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
};

namespace RawGamepad {
inline constexpr auto AxisPrefix = "Axis";
inline constexpr auto ButtonPrefix = "Button";
inline constexpr auto SwitchPrefix = "Switch";
inline constexpr float AxisCenter = 0.5f;
inline constexpr float NeutralAxisMin = 0.25f;
inline constexpr float NeutralAxisMax = 0.75f;

inline QString controlName(QLatin1StringView prefix, qsizetype index) {
    return prefix + QString::number(index);
}

inline bool isAxis(QStringView name) {
    return name.startsWith(QLatin1String(AxisPrefix));
}

inline int direction(QStringView name, float value) {
    if (!isAxis(name)) return value > 0.5f ? 1 : 0;
    return value > NeutralAxisMax ? 1 : value < NeutralAxisMin ? -1 : 0;
}
}

struct GamepadState {
    quint64 id;
    quint64 buttons;
    QList<float> axes;
    QString deviceId;
    QHash<QString, float> controls;
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

class GamepadSource {
public:
    GamepadSource();
    ~GamepadSource();
    QList<GamepadState> read();
    QList<GamepadRumbleDevice> rumbleDevices() const;
    void updateRumble(const QHash<QString, GamepadRumble>& outputs);
    void stopRumble() noexcept;

private:
    struct State;
    std::unique_ptr<State> state_;
};
}
