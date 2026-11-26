#include <RetroMediaLab/Platform/Gamepads.h>
#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QSet>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <GameInput.h>
#include "GameInputReportControls.h"
#include <roapi.h>
#include <wrl/client.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Gaming.Input.h>
#include <algorithm>
#include <limits>
#include <type_traits>
#include <map>
#include <memory>
#include <mutex>
#include <optional>

namespace RetroMediaLab::Platform {
namespace {
using Microsoft::WRL::ComPtr;
using winrt::Windows::Gaming::Input::GameControllerSwitchPosition;
using winrt::Windows::Gaming::Input::RawGameController;
constexpr auto LibraryName = L"GameInput.dll";
constexpr auto GameInputDevicePrefix = "gameinput:";
constexpr auto RawDevicePrefix = "rawgamecontroller:";
constexpr qint64 RawEnumerationIntervalMs = 500;
constexpr quint64 RawDeviceIdBase = quint64{1} << 63;
constexpr auto ControllerKinds = static_cast<GameInputKind>(GameInputKindGamepad | GameInputKindController |
    GameInputKindArcadeStick | GameInputKindFlightStick | GameInputKindRacingWheel);
constexpr auto InputKinds = static_cast<GameInputKind>(ControllerKinds | GameInputKindRawDeviceReport);
constexpr auto CreateExport = "GameInputCreate";
constexpr auto SystemButtonFilter = static_cast<GameInputSystemButtons>(GameInputSystemButtonGuide | GameInputSystemButtonShare);
constexpr auto CallbackDrainTimeout = (std::numeric_limits<uint64_t>::max)();
static_assert(Confirm == static_cast<quint64>(GameInputGamepadA));
static_assert(Cancel == static_cast<quint64>(GameInputGamepadB));
static_assert(Up == static_cast<quint64>(GameInputGamepadDPadUp));
static_assert(Down == static_cast<quint64>(GameInputGamepadDPadDown));
static_assert(Left == static_cast<quint64>(GameInputGamepadDPadLeft));
static_assert(Right == static_cast<quint64>(GameInputGamepadDPadRight));
static_assert(RumbleLowFrequency == static_cast<quint32>(GameInputRumbleLowFrequency));
static_assert(RumbleHighFrequency == static_cast<quint32>(GameInputRumbleHighFrequency));
static_assert(RumbleLeftTrigger == static_cast<quint32>(GameInputRumbleLeftTrigger));
static_assert(RumbleRightTrigger == static_cast<quint32>(GameInputRumbleRightTrigger));

struct ControllerIdentity {
    quint16 vendor;
    quint16 product;
    QString name;
};

using HardwareId = std::pair<quint16, quint16>;

bool consumeMatch(auto& counts, const auto& key) {
    const auto match = counts.find(key);
    if (match == counts.end() || match->second == 0) return false;
    --match->second;
    return true;
}

QString normalizedName(const QString& name) {
    return name.simplified().toCaseFolded();
}

quint64 directionButtons(GameControllerSwitchPosition position) {
    using Position = GameControllerSwitchPosition;
    switch (position) {
    case Position::Up: return Up;
    case Position::UpRight: return Up | Right;
    case Position::Right: return Right;
    case Position::DownRight: return Down | Right;
    case Position::Down: return Down;
    case Position::DownLeft: return Down | Left;
    case Position::Left: return Left;
    case Position::UpLeft: return Up | Left;
    default: return 0;
    }
}

struct ControllerMapping {
    QString name;
    QHash<QByteArray, QByteArray> controls;
};

quint32 controllerKey(quint16 vendor, quint16 product) {
    constexpr int VendorShift = 16;
    return (quint32{vendor} << VendorShift) | product;
}

const ControllerMapping* findControllerMapping(const ControllerIdentity& identity) {
    static const auto mappings = [] {
        constexpr auto DatabasePath = "Assets/Input/gamecontrollerdb.txt";
        constexpr auto WindowsPlatform = "platform:Windows";
        constexpr qsizetype GuidLength = 32;
        constexpr qsizetype VendorOffset = 8, ProductOffset = 16;
        constexpr int Hexadecimal = 16;
        QFile database{QDir(QCoreApplication::applicationDirPath()).filePath(QLatin1String(DatabasePath))};
        QHash<quint32, ControllerMapping> entries;
        if (!database.open(QIODevice::ReadOnly)) return entries;

        const auto readId = [](QByteArrayView guid, qsizetype offset) -> std::optional<quint16> {
            bool lowOk{}, highOk{};
            const auto low = guid.sliced(offset, 2).toUShort(&lowOk, Hexadecimal);
            const auto high = guid.sliced(offset + 2, 2).toUShort(&highOk, Hexadecimal);
            if (!lowOk || !highOk) return {};
            return static_cast<quint16>((high << 8) | low);
        };
        while (!database.atEnd()) {
            const auto line = database.readLine().trimmed();
            if (line.size() <= GuidLength + 1 || line[GuidLength] != ',' || !line.contains(WindowsPlatform)) continue;
            const auto guid = QByteArrayView(line).first(GuidLength);
            if (!std::ranges::all_of(guid, [](char ch) {
                    return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
                })) continue;
            const auto vendor = readId(guid, VendorOffset);
            const auto product = readId(guid, ProductOffset);
            if (!vendor || !product) continue;
            const auto key = controllerKey(*vendor, *product);
            if (entries.contains(key)) continue;
            const auto fields = line.sliced(GuidLength + 1).split(',');
            ControllerMapping mapping{QString::fromUtf8(fields.first().trimmed()), {}};
            if (mapping.name.isEmpty()) continue;
            for (const auto& field : fields.sliced(1)) {
                const auto separator = field.indexOf(':');
                if (separator <= 0 || separator == field.size() - 1) continue;
                const auto name = field.first(separator).trimmed().toLower();
                if (!mapping.controls.contains(name))
                    mapping.controls.insert(name, field.sliced(separator + 1).trimmed());
            }
            entries.insert(key, std::move(mapping));
        }
        return entries;
    }();
    const auto found = mappings.constFind(controllerKey(identity.vendor, identity.product));
    return found == mappings.constEnd() ? nullptr : &found.value();
}

struct RawControllerReading {
    const winrt::com_array<bool>& buttons;
    const winrt::com_array<GameControllerSwitchPosition>& switches;
    const winrt::com_array<double>& axes;

    float value(QByteArrayView source) const {
        if (source.isEmpty()) return 0.0f;
        const char sign = source.front() == '+' || source.front() == '-' ? source.front() : '\0';
        if (sign) source = source.sliced(1);
        if (source.size() < 2) return 0.0f;
        const auto indexText = source.sliced(1);
        bool indexOk{};
        if (source.front() == 'h') {
            const auto separator = indexText.indexOf('.');
            if (separator < 0) return 0.0f;
            const auto index = indexText.first(separator).toUInt(&indexOk);
            bool maskOk{};
            const auto mask = indexText.sliced(separator + 1).toUInt(&maskOk);
            if (!indexOk || !maskOk || index >= switches.size()) return 0.0f;
            constexpr unsigned HatUp = 1, HatRight = 2, HatDown = 4, HatLeft = 8;
            const auto pressed = directionButtons(switches[index]);
            return ((mask & HatUp) && (pressed & Up)) || ((mask & HatRight) && (pressed & Right)) ||
                ((mask & HatDown) && (pressed & Down)) || ((mask & HatLeft) && (pressed & Left)) ? 1.0f : 0.0f;
        }
        const auto index = indexText.toUInt(&indexOk);
        if (!indexOk) return 0.0f;
        if (source.front() == 'b' && index < buttons.size()) return buttons[index] ? 1.0f : 0.0f;
        if (source.front() != 'a' || index >= axes.size()) return 0.0f;
        const auto axis = std::clamp(static_cast<float>(axes[index]), 0.0f, 1.0f);
        if (sign == '-') return axis < RawGamepad::NeutralAxisMin ? 1.0f : 0.0f;
        if (sign == '+') return axis > RawGamepad::NeutralAxisMax ? 1.0f : 0.0f;
        return axis;
    }

    float stick(QByteArrayView source) const {
        if (!source.startsWith('a')) return 0.0f;
        bool ok{};
        const auto index = source.sliced(1).toUInt(&ok);
        return ok && index < axes.size() ? std::clamp(static_cast<float>(axes[index]) * 2.0f - 1.0f, -1.0f, 1.0f) : 0.0f;
    }
};

void applyControllerMapping(const ControllerMapping& mapping, const RawControllerReading& reading, GamepadState& state) {
    constexpr float PressedThreshold = 0.5f;
    constexpr std::pair<const char*, GamepadButton> buttons[]{
        {"a", Confirm}, {"b", Cancel}, {"x", X}, {"y", Y}, {"back", View}, {"start", Menu},
        {"leftshoulder", LeftShoulder}, {"rightshoulder", RightShoulder}, {"leftstick", LeftStick}, {"rightstick", RightStick},
        {"dpup", Up}, {"dpdown", Down}, {"dpleft", Left}, {"dpright", Right},
    };
    for (const auto& [name, flag] : buttons) {
        if (reading.value(mapping.controls.value(name)) > PressedThreshold) state.buttons |= flag;
    }
    state.axes = {reading.stick(mapping.controls.value("leftx")), reading.stick(mapping.controls.value("lefty")),
        reading.stick(mapping.controls.value("rightx")), reading.stick(mapping.controls.value("righty")),
        reading.value(mapping.controls.value("lefttrigger")), reading.value(mapping.controls.value("righttrigger"))};
    if (const auto guide = mapping.controls.constFind("guide"); guide != mapping.controls.constEnd())
        state.controls.insert(QLatin1String(RawGamepad::GuideButton), reading.value(guide.value()));
}

class RawControllerFallback {
public:
    RawControllerFallback() = default;
    RawControllerFallback(const RawControllerFallback&) = delete;
    RawControllerFallback& operator=(const RawControllerFallback&) = delete;

    ~RawControllerFallback() {
        devices_.clear();
        if (SUCCEEDED(apartment_)) RoUninitialize();
    }

    void appendStates(const QList<ControllerIdentity>& gameInputDevices, QList<GamepadState>& result) {
        if (FAILED(apartment_) && apartment_ != RPC_E_CHANGED_MODE) return;
        if (!enumerated_.isValid() || enumerated_.hasExpired(RawEnumerationIntervalMs)) refresh();
        if (devices_.isEmpty()) return;

        std::map<HardwareId, qsizetype> hardwareCounts;
        std::map<QString, qsizetype> nameCounts;
        for (const auto& identity : gameInputDevices) {
            const HardwareId hardware{identity.vendor, identity.product};
            if (hardware != HardwareId{}) ++hardwareCounts[hardware];
            if (!identity.name.isEmpty()) ++nameCounts[normalizedName(identity.name)];
        }
        // Match hardware first, so a name-only match cannot consume a more precise match.
        QSet<quint64> matchedDevices;
        for (const auto& device : devices_) {
            const HardwareId hardware{device.identity.vendor, device.identity.product};
            if (consumeMatch(hardwareCounts, hardware)) {
                matchedDevices.insert(device.id);
                consumeMatch(nameCounts, normalizedName(device.identity.name));
            }
        }
        for (const auto& device : devices_) {
            if (matchedDevices.contains(device.id) || consumeMatch(nameCounts, normalizedName(device.identity.name))) continue;
            try {
                result.append(read(device));
            } catch (const winrt::hresult_error&) {
                // The controller may have disconnected since enumeration.
            }
        }
    }

private:
    struct Device {
        RawGameController controller;
        quint64 id;
        QString deviceId;
        ControllerIdentity identity;
        const ControllerMapping* mapping;
    };

    void refresh() {
        enumerated_.start();
        QList<Device> devices;
        try {
            qsizetype index = 0;
            for (const auto& controller : RawGameController::RawGameControllers()) {
                const auto controllerIndex = index++;
                try {
                    ControllerIdentity identity{controller.HardwareVendorId(), controller.HardwareProductId(),
                        QString::fromStdWString(std::wstring(controller.DisplayName()))};
                    const auto* mapping = findControllerMapping(identity);
                    if (identity.name.trimmed().isEmpty() && mapping) identity.name = mapping->name;
                    auto id = QString::fromStdWString(std::wstring(controller.NonRoamableId()));
                    while (id.endsWith(QChar::Null)) id.chop(1);
                    if (id.trimmed().isEmpty()) {
                        const auto hex = [](quint16 value) { return QString::number(value, 16).rightJustified(4, QLatin1Char('0')).toUpper(); };
                        id = hex(identity.vendor) + QLatin1Char(':') + hex(identity.product) + QLatin1Char(':') + QString::number(controllerIndex);
                    }
                    id.prepend(QLatin1String(RawDevicePrefix));
                    const auto [entry, inserted] = ids_.try_emplace(id, nextId_);
                    if (inserted) ++nextId_;
                    devices.append(Device{controller, entry->second, id, std::move(identity), mapping});
                } catch (const winrt::hresult_error&) {
                    // A device can be removed while its properties are being read.
                }
            }
        } catch (const winrt::hresult_error&) {
            devices.clear();
        }
        devices_ = std::move(devices);
    }

    static GamepadState read(const Device& device) {
        const auto& controller = device.controller;
        winrt::com_array<bool> buttons(static_cast<uint32_t>(controller.ButtonCount()));
        winrt::com_array<GameControllerSwitchPosition> switches(static_cast<uint32_t>(controller.SwitchCount()));
        winrt::com_array<double> axes(static_cast<uint32_t>(controller.AxisCount()));
        controller.GetCurrentReading(buttons, switches, axes);
        GamepadState state{device.id, 0, {}, device.deviceId, {}};
        for (uint32_t i = 0; i < buttons.size(); ++i)
            state.controls.insert(RawGamepad::controlName(QLatin1String(RawGamepad::ButtonPrefix), i + 1), buttons[i] ? 1.0f : 0.0f);
        for (uint32_t i = 0; i < axes.size(); ++i)
            state.controls.insert(RawGamepad::controlName(QLatin1String(RawGamepad::AxisPrefix), i), static_cast<float>(axes[i]));
        constexpr std::pair<QLatin1StringView, GamepadButton> directions[]{
            {QLatin1String("Up"), Up}, {QLatin1String("Down"), Down},
            {QLatin1String("Left"), Left}, {QLatin1String("Right"), Right},
        };
        for (uint32_t i = 0; i < switches.size(); ++i) {
            const auto directionsDown = directionButtons(switches[i]);
            if (i == 0) state.buttons |= directionsDown;
            const auto name = RawGamepad::controlName(QLatin1String(RawGamepad::SwitchPrefix), i + 1);
            for (const auto& [suffix, flag] : directions)
                state.controls.insert(name + suffix, (directionsDown & flag) != 0 ? 1.0f : 0.0f);
        }
        if (device.mapping) applyControllerMapping(*device.mapping, {buttons, switches, axes}, state);
        return state;
    }

    const HRESULT apartment_ = RoInitialize(RO_INIT_SINGLETHREADED);
    QElapsedTimer enumerated_;
    QList<Device> devices_;
    std::map<QString, quint64> ids_;
    quint64 nextId_ = RawDeviceIdBase;
};

}

struct GamepadSource::State {
    State() {
        library_.reset(LoadLibraryExW(LibraryName, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32));
        if (!library_) return;
        const auto create = reinterpret_cast<decltype(&GameInputCreate)>(GetProcAddress(library_.get(), CreateExport));
        if (!create || FAILED(create(input_.GetAddressOf()))) return;
        registered_ = SUCCEEDED(input_->RegisterDeviceCallback(nullptr, InputKinds,
            GameInputDeviceConnected, GameInputBlockingEnumeration, this, &deviceChanged, &token_));
        if (registered_) {
            systemButtonsRegistered_ = SUCCEEDED(input_->RegisterSystemButtonCallback(nullptr, SystemButtonFilter,
                this, &systemButtonsChanged, &systemButtonsToken_));
        }
    }

    ~State() {
        stopRumble();
        if (systemButtonsRegistered_) {
            input_->StopCallback(systemButtonsToken_);
            input_->UnregisterCallback(systemButtonsToken_, CallbackDrainTimeout);
        }
        if (registered_) {
            input_->StopCallback(token_);
            input_->UnregisterCallback(token_, CallbackDrainTimeout);
        }
    }

    struct ConnectedDevice {
        ComPtr<IGameInputDevice> device;
        QString id;
        GameInputKind kinds;
        ControllerIdentity identity;
        GameInputReportControls reportControls;
        GameInputRumbleMotors motors;
        GameInputSystemButtons supportedSystemButtons;
        GameInputSystemButtons systemButtons{};
    };

    QList<GamepadRumbleDevice> rumbleDevices() {
        QList<GamepadRumbleDevice> result;
        {
            const std::lock_guard lock(mutex_);
            for (const auto& [id, controller] : devices_) {
                if (controller.motors == GameInputRumbleNone) continue;
                const auto& name = controller.identity.name;
                result.append({controller.id, name.isEmpty() ? controller.id : name, static_cast<quint32>(controller.motors)});
            }
        }
        std::ranges::sort(result, [](const auto& a, const auto& b) {
            const auto order = a.displayName.compare(b.displayName, Qt::CaseInsensitive);
            return order == 0 ? a.deviceId < b.deviceId : order < 0;
        });
        return result;
    }

    void updateRumble(const QHash<QString, GamepadRumble>& outputs) {
        if (outputs.isEmpty()) {
            stopRumble();
            return;
        }

        QHash<QString, ActiveRumble> next;
        {
            const std::lock_guard lock(mutex_);
            for (const auto& [id, controller] : devices_) {
                const auto requested = outputs.constFind(controller.id);
                if (requested == outputs.constEnd()) continue;
                const GamepadRumble supported{
                    (controller.motors & GameInputRumbleLowFrequency) != 0 ? requested->lowFrequency : 0.0f,
                    (controller.motors & GameInputRumbleHighFrequency) != 0 ? requested->highFrequency : 0.0f,
                    (controller.motors & GameInputRumbleLeftTrigger) != 0 ? requested->leftTrigger : 0.0f,
                    (controller.motors & GameInputRumbleRightTrigger) != 0 ? requested->rightTrigger : 0.0f,
                };
                if (supported != GamepadRumble{}) next.insert(controller.id, {controller.device, supported});
            }
        }
        for (auto previous = activeRumble_.cbegin(); previous != activeRumble_.cend(); ++previous) {
            const auto replacement = next.constFind(previous.key());
            if (replacement == next.constEnd() || replacement->device.Get() != previous->device.Get())
                sendRumble(*previous->device.Get(), {});
        }
        for (auto output = next.cbegin(); output != next.cend(); ++output) {
            const auto previous = activeRumble_.constFind(output.key());
            if (previous == activeRumble_.constEnd() || previous->device.Get() != output->device.Get() ||
                previous->value != output->value)
                sendRumble(*output->device.Get(), output->value);
        }
        activeRumble_.swap(next);
    }

    void stopRumble() noexcept {
        for (const auto& output : std::as_const(activeRumble_))
            sendRumble(*output.device.Get(), {});
        activeRumble_.clear();
    }

    QList<GamepadState> read() {
        std::map<quint64, ConnectedDevice> devices;
        {
            const std::lock_guard lock(mutex_);
            devices = devices_;
        }
        QList<GamepadState> result;
        QList<ControllerIdentity> usableDevices;
        usableDevices.reserve(static_cast<qsizetype>(devices.size()));
        for (const auto& [id, controller] : devices) {
            GameInputGamepadState state{};
            ComPtr<IGameInputReading> gamepad;
            if ((controller.kinds & GameInputKindGamepad) != 0 &&
                SUCCEEDED(input_->GetCurrentReading(GameInputKindGamepad, controller.device.Get(), gamepad.GetAddressOf())) &&
                gamepad) {
                gamepad->GetGamepadState(&state);
            }
            QHash<QString, float> controls;
            ComPtr<IGameInputReading> raw;
            if ((controller.kinds & GameInputKindController) != 0 &&
                SUCCEEDED(input_->GetCurrentReading(GameInputKindController, controller.device.Get(), raw.GetAddressOf())) &&
                raw) {
                controls = readControls(*raw.Get());
            }
            if (controls.isEmpty() && !controller.reportControls.isEmpty())
                readReportControls(controller, controls);
            if ((controller.kinds & ControllerKinds) == 0 && controls.isEmpty()) continue;
            if (systemButtonsRegistered_) {
                if ((controller.supportedSystemButtons & GameInputSystemButtonGuide) != 0)
                    controls.insert(QLatin1String(RawGamepad::GuideButton),
                        (controller.systemButtons & GameInputSystemButtonGuide) != 0 ? 1.0f : 0.0f);
                if ((controller.supportedSystemButtons & GameInputSystemButtonShare) != 0)
                    controls.insert(QLatin1String(RawGamepad::ShareButton),
                        (controller.systemButtons & GameInputSystemButtonShare) != 0 ? 1.0f : 0.0f);
            }
            usableDevices.append(controller.identity);
            result.append(GamepadState{id, static_cast<quint64>(state.buttons), {state.leftThumbstickX, state.leftThumbstickY,
                state.rightThumbstickX, state.rightThumbstickY, state.leftTrigger, state.rightTrigger}, controller.id, std::move(controls)});
        }
        rawFallback_.appendStates(usableDevices, result);
        return result;
    }

private:
    struct ActiveRumble {
        ComPtr<IGameInputDevice> device;
        GamepadRumble value;
    };

    static void sendRumble(IGameInputDevice& device, const GamepadRumble& value) noexcept {
        const GameInputRumbleParams parameters{value.lowFrequency, value.highFrequency, value.leftTrigger, value.rightTrigger};
        device.SetRumbleState(&parameters);
    }

    void readReportControls(const ConnectedDevice& controller, QHash<QString, float>& controls) {
        ComPtr<IGameInputReading> reading;
        if (FAILED(input_->GetCurrentReading(GameInputKindRawDeviceReport, controller.device.Get(), reading.GetAddressOf())) ||
            !reading) return;
        ComPtr<IGameInputRawDeviceReport> report;
        if (!reading->GetRawReport(report.GetAddressOf()) || !report) return;
        const auto* info = report->GetReportInfo();
        if (!info || info->kind != GameInputRawInputReport) return;
        controller.reportControls.decode(info->id, [&](uint32_t index, int64_t& value) {
            return index < info->itemCount && report->GetItemValue(index, &value);
        }, controls);
    }

    static QHash<QString, float> readControls(IGameInputReading& reading) {
        const auto axisCount = reading.GetControllerAxisCount();
        const auto buttonCount = reading.GetControllerButtonCount();
        const auto switchCount = reading.GetControllerSwitchCount();
        QList<float> axes(axisCount);
        QList<bool> buttons(buttonCount);
        QList<GameInputSwitchPosition> switches(switchCount);
        const auto axesRead = (std::min)(axisCount, reading.GetControllerAxisState(axisCount, axes.data()));
        const auto buttonsRead = (std::min)(buttonCount, reading.GetControllerButtonState(buttonCount, buttons.data()));
        const auto switchesRead = (std::min)(switchCount, reading.GetControllerSwitchState(switchCount, switches.data()));
        QHash<QString, float> result;
        result.reserve(qsizetype(axesRead) + qsizetype(buttonsRead) + qsizetype(switchesRead));
        for (uint32_t i = 0; i < axesRead; ++i)
            result.insert(RawGamepad::controlName(QLatin1String(RawGamepad::AxisPrefix), i), axes[i]);
        for (uint32_t i = 0; i < buttonsRead; ++i)
            result.insert(RawGamepad::controlName(QLatin1String(RawGamepad::ButtonPrefix), i), buttons[i] ? 1.0f : 0.0f);
        for (uint32_t i = 0; i < switchesRead; ++i)
            result.insert(RawGamepad::controlName(QLatin1String(RawGamepad::SwitchPrefix), i), static_cast<float>(switches[i]));
        return result;
    }

    static void CALLBACK deviceChanged(GameInputCallbackToken, void* context, IGameInputDevice* device,
        uint64_t, GameInputDeviceStatus status, GameInputDeviceStatus) noexcept {
        auto& self = *static_cast<State*>(context);
        try {
            const std::lock_guard lock(self.mutex_);
            const auto found = std::find_if(self.devices_.begin(), self.devices_.end(),
                [device](const auto& entry) { return entry.second.device.Get() == device; });
            if ((status & GameInputDeviceConnected) != 0) {
                if (found == self.devices_.end()) {
                    const auto* info = device->GetDeviceInfo();
                    if (!info) return;
                    const bool hasControllerKind = (info->supportedInput & ControllerKinds) != 0;
                    using Reports = GameInputReportControls;
                    const auto usage = info->usage;
                    const bool controllerUsage = usage.page == Reports::DesktopPage &&
                        (usage.id == Reports::JoystickUsage || usage.id == Reports::GamepadUsage ||
                            usage.id == Reports::MultiAxisControllerUsage);
                    if (!hasControllerKind && !controllerUsage) return;
                    auto reportControls = Reports::fromDeviceInfo(*info);
                    if (!hasControllerKind && reportControls.isEmpty()) return;
                    const auto& bytes = info->deviceId.value;
                    const auto id = QLatin1String(GameInputDevicePrefix) + QString::fromLatin1(
                        QByteArray(reinterpret_cast<const char*>(bytes), sizeof(bytes)).toHex());
                    const auto* name = info->displayName;
                    self.devices_.emplace(++self.lastId_, ConnectedDevice{ComPtr<IGameInputDevice>(device), id, info->supportedInput,
                        {info->vendorId, info->productId, name ? QString::fromUtf8(name->data) : QString{}},
                        std::move(reportControls), info->supportedRumbleMotors, info->supportedSystemButtons});
                }
            } else if (found != self.devices_.end()) {
                self.devices_.erase(found);
            }
        } catch (...) {
            // Exceptions cannot cross the GameInput callback boundary.
        }
    }

    static void CALLBACK systemButtonsChanged(GameInputCallbackToken, void* context, IGameInputDevice* device,
        uint64_t, GameInputSystemButtons current, GameInputSystemButtons) noexcept {
        auto& self = *static_cast<State*>(context);
        try {
            const std::lock_guard lock(self.mutex_);
            const auto found = std::ranges::find_if(self.devices_,
                [device](const auto& entry) { return entry.second.device.Get() == device; });
            if (found != self.devices_.end()) found->second.systemButtons = current;
        } catch (...) {
            // Exceptions cannot cross the GameInput callback boundary.
        }
    }

    struct LibraryDeleter {
        void operator()(HMODULE library) const noexcept { FreeLibrary(library); }
    };
    std::unique_ptr<std::remove_pointer_t<HMODULE>, LibraryDeleter> library_;
    ComPtr<IGameInput> input_;
    std::mutex mutex_;
    std::map<quint64, ConnectedDevice> devices_;
    quint64 lastId_ = 0;
    GameInputCallbackToken token_{};
    bool registered_ = false;
    GameInputCallbackToken systemButtonsToken_{};
    bool systemButtonsRegistered_ = false;
    RawControllerFallback rawFallback_;
    QHash<QString, ActiveRumble> activeRumble_;
};

GamepadSource::GamepadSource() : state_(std::make_unique<State>()) {}
GamepadSource::~GamepadSource() = default;
QList<GamepadState> GamepadSource::read() { return state_->read(); }
QList<GamepadRumbleDevice> GamepadSource::rumbleDevices() const { return state_->rumbleDevices(); }
void GamepadSource::updateRumble(const QHash<QString, GamepadRumble>& outputs) { state_->updateRumble(outputs); }
void GamepadSource::stopRumble() noexcept { state_->stopRumble(); }
}
