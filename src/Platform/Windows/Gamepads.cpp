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
#ifdef RML_GAMEINPUT_V3
#include "GameInputGamepadMapping.h"
#include "HidReportControls.h"
#include <shellapi.h>
#include <cfgmgr32.h>
#include <QFileInfo>
#include <QSettings>
#include <QVersionNumber>
#include <QUuid>
#else
#include "GameInputReportControls.h"
#endif
#include <roapi.h>
#include <wrl/client.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Gaming.Input.h>
#include <algorithm>
#include <array>
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
#ifdef RML_GAMEINPUT_V3
using namespace GameInput::v3;
using ReportControls = HidReportControls;
static_assert(LeftTriggerButton == quint64{GameInputGamepadLeftTriggerButton});
static_assert(RightTriggerButton == quint64{GameInputGamepadRightTriggerButton});
static_assert(PaddleLeft1 == quint64{GameInputGamepadPaddleLeft1});
static_assert(PaddleLeft2 == quint64{GameInputGamepadPaddleLeft2});
static_assert(PaddleRight1 == quint64{GameInputGamepadPaddleRight1});
static_assert(PaddleRight2 == quint64{GameInputGamepadPaddleRight2});
constexpr auto LibraryName = L"GameInputRedist.dll";
constexpr auto InitializeExport = "GameInputInitialize";
constexpr auto RuntimeRegistryKey = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\GameInput";
constexpr auto RuntimeRegistryValue = "RedistDir";
constexpr auto RuntimeLibraryDirectory = "lib/GameInput";
constexpr auto MinimumRuntimeVersion = RML_GAMEINPUT_MIN_VERSION;
constexpr auto RuntimeInstaller = "Runtime/GameInputRedist.msi";
constexpr auto WindowsInstaller = "msiexec.exe";
constexpr auto InstallParameters = "/i \"%1\"";
constexpr auto RunAs = L"runas";

QString systemDirectory() {
    wchar_t system[MAX_PATH]{};
    const auto length = GetSystemDirectoryW(system, MAX_PATH);
    return length && length < MAX_PATH ? QString::fromWCharArray(system, length) : QString{};
}

QString bundledInstallerPath() {
    const auto path = QDir(QCoreApplication::applicationDirPath()).filePath(QLatin1String(RuntimeInstaller));
    return QFileInfo(path).isFile() ? QDir::toNativeSeparators(path) : QString{};
}

QVersionNumber runtimeVersion(const QString& path) {
    const auto* name = reinterpret_cast<LPCWSTR>(path.utf16());
    DWORD ignored{};
    const auto size = GetFileVersionInfoSizeW(name, &ignored);
    if (!size) return {};
    QByteArray buffer(size, Qt::Uninitialized);
    if (!GetFileVersionInfoW(name, 0, size, buffer.data())) return {};
    void* value{};
    UINT length{};
    if (!VerQueryValueW(buffer.data(), L"\\", &value, &length) || length < sizeof(VS_FIXEDFILEINFO)) return {};
    const auto& info = *static_cast<const VS_FIXEDFILEINFO*>(value);
    return QVersionNumber({HIWORD(info.dwFileVersionMS), LOWORD(info.dwFileVersionMS),
        HIWORD(info.dwFileVersionLS), LOWORD(info.dwFileVersionLS)});
}

QString gameInputRuntimePath() {
    const QDir application(QCoreApplication::applicationDirPath());
    const QSettings registry32(QLatin1String(RuntimeRegistryKey), QSettings::Registry32Format);
    const QSettings registry64(QLatin1String(RuntimeRegistryKey), QSettings::Registry64Format);
    const auto name = QString::fromWCharArray(LibraryName);
    const QStringList directories{
        systemDirectory(),
        application.filePath(QLatin1String(RuntimeLibraryDirectory)), application.absolutePath(),
        registry64.value(QLatin1String(RuntimeRegistryValue)).toString(),
        registry32.value(QLatin1String(RuntimeRegistryValue)).toString()
    };
    QString selected;
    const auto requiredVersion = QVersionNumber::fromString(QLatin1String(MinimumRuntimeVersion));
    auto selectedVersion = requiredVersion;
    for (const auto& directory : directories) {
        if (directory.isEmpty() || QDir::isRelativePath(directory)) continue;
        const auto path = QDir::toNativeSeparators(QDir(directory).filePath(name));
        if (!QFile::exists(path)) continue;
        const auto version = runtimeVersion(path);
        if (version > selectedVersion || (selected.isEmpty() && version == requiredVersion)) {
            selected = path;
            selectedVersion = version;
        }
    }
    return selected;
}

HMODULE loadGameInputRuntime() {
    const auto path = gameInputRuntimePath();
    return path.isEmpty() ? nullptr : LoadLibraryExW(reinterpret_cast<LPCWSTR>(path.utf16()), nullptr,
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);
}
#else
using ReportControls = GameInputReportControls;
constexpr auto LibraryName = L"GameInput.dll";
constexpr auto CreateExport = "GameInputCreate";
constexpr auto CallbackDrainTimeout = (std::numeric_limits<uint64_t>::max)();
#endif
constexpr auto GameInputDevicePrefix = "gameinput:";
constexpr auto RawDevicePrefix = "rawgamecontroller:";
constexpr qint64 RawEnumerationIntervalMs = 500;
constexpr quint64 RawDeviceIdBase = quint64{1} << 63;
constexpr auto ControllerKinds = static_cast<GameInputKind>(GameInputKindGamepad | GameInputKindController |
    GameInputKindArcadeStick | GameInputKindFlightStick | GameInputKindRacingWheel);
constexpr auto InputKinds = static_cast<GameInputKind>(ControllerKinds | GameInputKindRawDeviceReport);
constexpr auto SystemButtonFilter = static_cast<GameInputSystemButtons>(GameInputSystemButtonGuide | GameInputSystemButtonShare);
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
        state.displayName = device.identity.name;
        state.vendorId = device.identity.vendor;
        state.productId = device.identity.product;
        state.standardGamepad = !state.axes.isEmpty();
        state.controlCounts = {static_cast<quint32>(axes.size()),
            static_cast<quint32>(buttons.size()), static_cast<quint32>(switches.size())};
        return state;
    }

    const HRESULT apartment_ = RoInitialize(RO_INIT_SINGLETHREADED);
    QElapsedTimer enumerated_;
    QList<Device> devices_;
    std::map<QString, quint64> ids_;
    quint64 nextId_ = RawDeviceIdBase;
};

}

bool canOfferGameInputInstaller() {
#ifdef RML_GAMEINPUT_V3
    return !bundledInstallerPath().isEmpty() && gameInputRuntimePath().isEmpty();
#else
    return false;
#endif
}

GameInputInstallResult launchGameInputInstaller() {
#ifdef RML_GAMEINPUT_V3
    const auto installer = bundledInstallerPath();
    const auto system = systemDirectory();
    if (!installer.isEmpty() && !system.isEmpty()) {
        const auto program = QDir::toNativeSeparators(QDir(system).filePath(QLatin1String(WindowsInstaller)));
        const auto parameters = QString::fromLatin1(InstallParameters).arg(installer);
        SHELLEXECUTEINFOW launch{};
        launch.cbSize = sizeof(launch);
        launch.fMask = SEE_MASK_NOASYNC;
        launch.lpVerb = RunAs;
        launch.lpFile = reinterpret_cast<LPCWSTR>(program.utf16());
        launch.lpParameters = reinterpret_cast<LPCWSTR>(parameters.utf16());
        launch.nShow = SW_SHOWNORMAL;
        if (ShellExecuteExW(&launch)) return GameInputInstallResult::Started;
        if (GetLastError() == ERROR_CANCELLED) return GameInputInstallResult::Cancelled;
    }
#endif
    return GameInputInstallResult::Failed;
}

struct GamepadSource::State {
    State() {
#ifdef RML_GAMEINPUT_V3
        library_.reset(loadGameInputRuntime());
        if (!library_) return;
        const auto initialize = reinterpret_cast<decltype(&GameInputInitialize)>(
            GetProcAddress(library_.get(), InitializeExport));
        if (!initialize || FAILED(initialize(IID_IGameInput, reinterpret_cast<void**>(input_.GetAddressOf())))) return;
#else
        library_.reset(LoadLibraryExW(LibraryName, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32));
        if (!library_) return;
        const auto create = reinterpret_cast<decltype(&GameInputCreate)>(GetProcAddress(library_.get(), CreateExport));
        if (!create || FAILED(create(input_.GetAddressOf()))) return;
#endif
        registered_ = SUCCEEDED(input_->RegisterDeviceCallback(nullptr, InputKinds,
            GameInputDeviceConnected, GameInputBlockingEnumeration, this, &deviceChanged, &token_));
        if (registered_) {
            systemButtonsRegistered_ = SUCCEEDED(input_->RegisterSystemButtonCallback(nullptr, SystemButtonFilter,
                this, &systemButtonsChanged, &systemButtonsToken_));
        }
    }

    ~State() {
        stopRumble();
        if (systemButtonsRegistered_) unregisterCallback(systemButtonsToken_);
        if (registered_) unregisterCallback(token_);
#ifdef RML_GAMEINPUT_V3
        for (const auto& [id, token] : rawCallbacks_)
            if (token) unregisterCallback(token);
#endif
    }

    struct ConnectedDevice {
        ComPtr<IGameInputDevice> device;
        QString id;
        GameInputKind kinds;
        ControllerIdentity identity;
#ifdef RML_GAMEINPUT_V3
        std::shared_ptr<const ReportControls> reportControls;
        bool hasReportControls() const { return bool(reportControls); }
#else
        ReportControls reportControls;
        bool hasReportControls() const { return !reportControls.isEmpty(); }
#endif
        GameInputRumbleMotors motors;
        GameInputSystemButtons supportedSystemButtons;
        GameInputSystemButtons systemButtons{};
        GamepadControlCounts controlCounts{};
        GamepadCapabilities capabilities{};
        std::shared_ptr<const GamepadIdentity> gameInputIdentity{};
        std::shared_ptr<const QHash<QString, GamepadControlDescription>> controlDescriptions{};
#ifdef RML_GAMEINPUT_V3
        QByteArray latestRawReport{};
        ComPtr<IGameInputMapper> mapper{};
#endif
    };

#ifdef RML_GAMEINPUT_V3
    static std::shared_ptr<const GamepadIdentity> describeIdentity(const GameInputDeviceInfo& info) {
        constexpr auto version = [](const GameInputVersion& value) {
            return QVersionNumber(value.major, value.minor, value.build).toString() + QLatin1Char('.') +
                QString::number(value.revision);
        };
        auto identity = std::make_shared<GamepadIdentity>();
        identity->family = static_cast<GamepadIdentity::Family>(info.deviceFamily);
        identity->name = QString::fromUtf8(info.displayName);
        identity->revision = info.revisionNumber;
        identity->hardwareVersion = version(info.hardwareVersion);
        identity->firmwareVersion = version(info.firmwareVersion);
        identity->usagePage = info.usage.page;
        identity->usage = info.usage.id;
        identity->rootId = QString::fromLatin1(QByteArray(
            reinterpret_cast<const char*>(info.deviceRootId.value), sizeof(info.deviceRootId.value)).toHex());
        identity->containerId = QUuid(info.containerId).toString(QUuid::WithoutBraces);
        identity->pnpPath = QString::fromUtf8(info.pnpPath);

        constexpr auto InterfacePrefix = QLatin1String(R"(\\?\)");
        constexpr auto InterfaceSuffix = QLatin1String("#{");
        const auto end = identity->pnpPath.lastIndexOf(InterfaceSuffix);
        if (!identity->pnpPath.startsWith(InterfacePrefix) || end <= InterfacePrefix.size()) return identity;
        auto instanceId = identity->pnpPath.sliced(InterfacePrefix.size(), end - InterfacePrefix.size());
        instanceId.replace(QLatin1Char('#'), QLatin1Char('\\'));
        auto nativeId = instanceId.toStdWString();
        DEVINST node{};
        if (CM_Locate_DevNodeW(&node, nativeId.data(), CM_LOCATE_DEVNODE_NORMAL) != CR_SUCCESS) return identity;

        constexpr GUID DeviceProperties{0xa45c254e, 0xdf1c, 0x4efd, {0x80,0x20,0x67,0xd1,0x46,0xa8,0x50,0xe0}};
        constexpr GUID BusProperties{0x540b947e, 0x8b40, 0x45bc, {0xa8,0xa2,0x6a,0x0b,0x89,0x4c,0xbd,0xa2}};
        constexpr DEVPROPKEY Description{DeviceProperties, 2}, Manufacturer{DeviceProperties, 13},
            FriendlyName{DeviceProperties, 14}, BusDescription{BusProperties, 4};
        constexpr int MaximumDepth = 8;
        constexpr ULONG MaximumPropertyBytes = 64 * 1024;
        constexpr auto EntryFormat = QLatin1String("%1:%2=%3");
        for (int depth = 0; depth < MaximumDepth; ++depth) {
            const auto append = [&](QLatin1StringView kind, const QString& value) {
                if (!value.trimmed().isEmpty()) identity->windowsChain.append(EntryFormat.arg(QString::number(depth), kind, value));
            };
            const auto read = [&](const DEVPROPKEY& key) {
                DEVPROPTYPE type = 0;
                ULONG size = 0;
                const auto result = CM_Get_DevNode_PropertyW(node, &key, &type, nullptr, &size, 0);
                if ((result != CR_SUCCESS && result != CR_BUFFER_SMALL) || !size ||
                    size > MaximumPropertyBytes || size % sizeof(wchar_t)) return QString{};
                std::wstring value(size / sizeof(wchar_t), L'\0');
                if (CM_Get_DevNode_PropertyW(node, &key, &type, reinterpret_cast<PBYTE>(value.data()), &size, 0) != CR_SUCCESS ||
                    type != DEVPROP_TYPE_STRING) return QString{};
                const auto terminator = value.find(L'\0');
                return QString::fromWCharArray(value.data(), terminator == std::wstring::npos ? value.size() : terminator);
            };
            append(QLatin1String("Bus"), read(BusDescription));
            append(QLatin1String("Friendly"), read(FriendlyName));
            append(QLatin1String("Description"), read(Description));
            const auto manufacturer = read(Manufacturer);
            append(QLatin1String("Manufacturer"), manufacturer);
            if (identity->manufacturer.isEmpty()) identity->manufacturer = manufacturer.trimmed();
            constexpr std::size_t DeviceIdBufferLength = 1024;
            std::array<wchar_t, DeviceIdBufferLength> id{};
            if (CM_Get_Device_IDW(node, id.data(), static_cast<ULONG>(id.size()), 0) == CR_SUCCESS)
                append(QLatin1String("Id"), QString::fromWCharArray(id.data()));
            DEVINST parent{};
            if (CM_Get_Parent(&parent, node, 0) != CR_SUCCESS) break;
            node = parent;
        }
        return identity;
    }

    static void describeAdvancedCapabilities(ConnectedDevice& controller, const GameInputDeviceInfo& info) {
        auto& caps = controller.capabilities;
        caps.hapticsQueryable = true;
        if (info.gamepadInfo) caps.gamepadLayout = static_cast<quint32>(info.gamepadInfo->supportedLayout);
        constexpr uint32_t MaximumFeedbackMotors = 64;
        if (info.forceFeedbackMotorInfo && info.forceFeedbackMotorCount <= MaximumFeedbackMotors) {
            for (uint32_t index = 0; index < info.forceFeedbackMotorCount; ++index) {
                const auto& motor = info.forceFeedbackMotorInfo[index];
                const std::array effects{motor.isConstantEffectSupported, motor.isRampEffectSupported,
                    motor.isSineWaveEffectSupported, motor.isSquareWaveEffectSupported, motor.isTriangleWaveEffectSupported,
                    motor.isSawtoothUpWaveEffectSupported, motor.isSawtoothDownWaveEffectSupported,
                    motor.isSpringEffectSupported, motor.isFrictionEffectSupported,
                    motor.isDamperEffectSupported, motor.isInertiaEffectSupported};
                quint16 mask = 0;
                for (std::size_t bit = 0; bit < effects.size(); ++bit)
                    if (effects[bit]) mask |= static_cast<quint16>(1u << bit);
                caps.feedbackDetails.append(GamepadCapabilities::FeedbackMotor{static_cast<quint32>(motor.supportedAxes), mask});
            }
        }
        const auto indexes = [&](GameInputKind kind, auto getCount, auto getIndexes) {
            constexpr uint32_t MaximumControls = 1024;
            uint32_t count = 0;
            auto& device = *controller.device.Get();
            if (FAILED((device.*getCount)(kind, &count)) || !count || count > MaximumControls) return QList<quint8>{};
            QList<quint8> result(count);
            if (FAILED((device.*getIndexes)(kind, count, result.data()))) result.clear();
            return result;
        };
        constexpr std::array kinds{GameInputKindArcadeStick, GameInputKindFlightStick, GameInputKindGamepad, GameInputKindRacingWheel};
        for (const auto kind : kinds) {
            if (!(controller.kinds & kind)) continue;
            auto axes = indexes(kind, &IGameInputDevice::GetExtraAxisCount, &IGameInputDevice::GetExtraAxisIndexes);
            auto buttons = indexes(kind, &IGameInputDevice::GetExtraButtonCount, &IGameInputDevice::GetExtraButtonIndexes);
            if (!axes.isEmpty() || !buttons.isEmpty())
                caps.extraControls.append(GamepadCapabilities::ExtraControls{
                    static_cast<GamepadCapabilities::InputKind>(kind), std::move(axes), std::move(buttons)});
        }
    }

    static void readAdvancedCapabilities(IGameInputDevice& device, GamepadCapabilities& caps) {
        if (!caps.feedbackDetails.isEmpty()) {
            for (qsizetype index = 0; index < caps.feedbackDetails.size(); ++index)
                caps.feedbackDetails[index].poweredOn = device.IsForceFeedbackMotorPoweredOn(static_cast<uint32_t>(index));
        }
        if ((device.GetDeviceStatus() & GameInputDeviceHapticInfoReady) == 0) return;
        GameInputHapticInfo haptics{};
        if (FAILED(device.GetHapticInfo(&haptics))) return;
        const auto begin = std::begin(haptics.audioEndpointId);
        const auto end = std::find(begin, std::end(haptics.audioEndpointId), L'\0');
        GamepadCapabilities::Haptics result{QString::fromWCharArray(begin, end - begin), {}};
        const auto count = std::min(haptics.locationCount, GAMEINPUT_HAPTIC_MAX_LOCATIONS);
        for (uint32_t index = 0; index < count; ++index)
            result.locations.append(QUuid(haptics.locations[index]).toString(QUuid::WithoutBraces));
        caps.haptics = std::move(result);
    }
#endif

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

    bool updateRumble(const QHash<QString, GamepadRumble>& outputs) {
        if (outputs.isEmpty()) {
            stopRumble();
            return false;
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
        return !activeRumble_.isEmpty();
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
#ifdef RML_GAMEINPUT_V3
        syncRawCallbacks(devices);
#endif
        QList<GamepadState> result;
        QList<ControllerIdentity> usableDevices;
        usableDevices.reserve(static_cast<qsizetype>(devices.size()));
        for (const auto& [id, controller] : devices) {
            GameInputGamepadState state{};
            ComPtr<IGameInputReading> gamepad;
            const bool hasGamepad = (controller.kinds & GameInputKindGamepad) != 0 &&
                SUCCEEDED(input_->GetCurrentReading(GameInputKindGamepad, controller.device.Get(), gamepad.GetAddressOf())) &&
                gamepad && gamepad->GetGamepadState(&state);
            if (!hasGamepad) state = {};
            QHash<QString, float> controls;
            ComPtr<IGameInputReading> raw;
            if (
#ifdef RML_GAMEINPUT_V3
                !controller.hasReportControls() &&
#endif
                (controller.kinds & GameInputKindController) != 0 &&
                SUCCEEDED(input_->GetCurrentReading(GameInputKindController, controller.device.Get(), raw.GetAddressOf())) &&
                raw) {
                const auto sample = readControls(*raw.Get());
                controls = sample.namedControls();
#ifdef RML_GAMEINPUT_V3
                if (!hasGamepad && controller.mapper) {
                    state = GameInputGamepadMapping::map(*controller.mapper.Get(),
                        {{sample.axes.constData(), size_t(sample.axes.size())},
                            {sample.buttons.constData(), size_t(sample.buttons.size())},
                            {sample.switches.constData(), size_t(sample.switches.size())}});
                }
#endif
            }
            if (controls.isEmpty() && controller.hasReportControls())
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
            bool standardGamepad = (controller.kinds & GameInputKindGamepad) != 0;
#ifdef RML_GAMEINPUT_V3
            standardGamepad = standardGamepad || bool(controller.mapper);
#endif
            const auto kinds = static_cast<quint8>(
                ((controller.kinds & GameInputKindRacingWheel) ? GamepadRacingWheel : 0) |
                ((controller.kinds & GameInputKindFlightStick) ? GamepadFlightStick : 0) |
                ((controller.kinds & GameInputKindArcadeStick) ? GamepadArcadeStick : 0));
            GamepadState snapshot{id, static_cast<quint64>(state.buttons), {state.leftThumbstickX, state.leftThumbstickY,
                state.rightThumbstickX, state.rightThumbstickY, state.leftTrigger, state.rightTrigger}, controller.id,
                std::move(controls), controller.identity.name, standardGamepad,
                controller.identity.vendor, controller.identity.product, kinds};
            const auto readSpecialized = [&](GameInputKind kind, auto getter, auto& value) {
                ComPtr<IGameInputReading> reading;
                return (controller.kinds & kind) != 0 &&
                    SUCCEEDED(input_->GetCurrentReading(kind, controller.device.Get(), reading.GetAddressOf())) &&
                    reading && (reading.Get()->*getter)(&value);
            };
            GameInputRacingWheelState wheel{};
            if (readSpecialized(GameInputKindRacingWheel, &IGameInputReading::GetRacingWheelState, wheel)) {
                snapshot.racingWheel = RacingWheelInput{static_cast<quint16>(wheel.buttons), wheel.patternShifterGear,
                    wheel.wheel, wheel.throttle, wheel.brake, wheel.clutch, wheel.handbrake};
            }
            GameInputFlightStickState flight{};
            if (readSpecialized(GameInputKindFlightStick, &IGameInputReading::GetFlightStickState, flight)) {
                snapshot.flightStick = FlightStickInput{static_cast<quint16>(flight.buttons),
                    static_cast<quint8>(flight.hatSwitch), flight.roll, flight.pitch, flight.yaw, flight.throttle};
            }
            GameInputArcadeStickState arcade{};
            if (readSpecialized(GameInputKindArcadeStick, &IGameInputReading::GetArcadeStickState, arcade))
                snapshot.arcadeStick = ArcadeStickInput{static_cast<quint16>(arcade.buttons)};
            snapshot.controlCounts = controller.controlCounts;
            snapshot.capabilities = controller.capabilities;
            snapshot.identity = controller.gameInputIdentity;
            snapshot.controlDescriptions = controller.controlDescriptions;
#ifdef RML_GAMEINPUT_V3
            readAdvancedCapabilities(*controller.device.Get(), *snapshot.capabilities);
#endif
            if (!snapshot.controlCounts.axes && !snapshot.controlCounts.buttons && !snapshot.controlCounts.switches) {
                for (auto it = snapshot.controls.cbegin(); it != snapshot.controls.cend(); ++it) {
                    if (RawGamepad::isAxis(it.key())) ++snapshot.controlCounts.axes;
                    else if (it.key().startsWith(QLatin1String(RawGamepad::ButtonPrefix))) ++snapshot.controlCounts.buttons;
                    else if (it.key().startsWith(QLatin1String(RawGamepad::SwitchPrefix))) ++snapshot.controlCounts.switches;
                }
            }
            result.append(std::move(snapshot));
        }
        rawFallback_.appendStates(usableDevices, result);
        return result;
    }

private:
    void unregisterCallback(GameInputCallbackToken token) noexcept {
        input_->StopCallback(token);
#ifdef RML_GAMEINPUT_V3
        input_->UnregisterCallback(token);
#else
        input_->UnregisterCallback(token, CallbackDrainTimeout);
#endif
    }

#ifdef RML_GAMEINPUT_V3
    void syncRawCallbacks(const std::map<quint64, ConnectedDevice>& devices) {
        // GameInput requires registration and removal outside its callback thread.
        std::erase_if(rawCallbacks_, [&](const auto& entry) {
            if (devices.contains(entry.first)) return false;
            if (entry.second) unregisterCallback(entry.second);
            return true;
        });
        for (const auto& [id, controller] : devices) {
            if (!controller.hasReportControls()) continue;
            const auto [entry, added] = rawCallbacks_.try_emplace(id);
            if (added)
                input_->RegisterReadingCallback(controller.device.Get(), GameInputKindRawDeviceReport,
                    this, &rawReading, &entry->second);
        }
    }

    static QByteArray rawReportBytes(IGameInputReading& reading) {
        ComPtr<IGameInputRawDeviceReport> report;
        if (!reading.GetRawReport(report.GetAddressOf()) || !report) return {};
        GameInputRawDeviceReportInfo info{};
        report->GetReportInfo(&info);
        constexpr auto MaximumHidReportSize = (std::numeric_limits<USHORT>::max)();
        const auto size = report->GetRawDataSize();
        if (info.kind != GameInputRawInputReport || !size || size > MaximumHidReportSize) return {};
        QByteArray bytes(qsizetype(size), Qt::Uninitialized);
        bytes.truncate(qsizetype(std::min(size, report->GetRawData(size, bytes.data()))));
        return bytes;
    }

    static void CALLBACK rawReading(GameInputCallbackToken, void* context, IGameInputReading* reading) noexcept {
        try {
            if (!reading) return;
            auto bytes = rawReportBytes(*reading);
            if (bytes.isEmpty()) return;
            ComPtr<IGameInputDevice> device;
            reading->GetDevice(device.GetAddressOf());
            auto& self = *static_cast<State*>(context);
            const std::lock_guard lock(self.mutex_);
            const auto found = std::ranges::find_if(self.devices_, [&](const auto& entry) {
                return entry.second.device.Get() == device.Get();
            });
            if (found != self.devices_.end()) found->second.latestRawReport = std::move(bytes);
        } catch (...) {
            // Exceptions cannot cross the GameInput callback boundary.
        }
    }
#endif

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
        if (FAILED(input_->GetCurrentReading(GameInputKindRawDeviceReport, controller.device.Get(), reading.GetAddressOf())))
            reading.Reset();
#ifdef RML_GAMEINPUT_V3
        const auto bytes = reading ? rawReportBytes(*reading.Get()) : QByteArray{};
        controller.reportControls->decode(bytes.isEmpty() ? controller.latestRawReport : bytes, controls);
#else
        ComPtr<IGameInputRawDeviceReport> report;
        if (!reading || !reading->GetRawReport(report.GetAddressOf()) || !report) return;
        const auto* info = report->GetReportInfo();
        if (!info || info->kind != GameInputRawInputReport) return;
        controller.reportControls.decode(info->id, [&](uint32_t index, int64_t& value) {
            return index < info->itemCount && report->GetItemValue(index, &value);
        }, controls);
#endif
    }

    struct ControllerSample {
        QList<float> axes;
        QList<bool> buttons;
        QList<GameInputSwitchPosition> switches;

        QHash<QString, float> namedControls() const {
            QHash<QString, float> result;
            result.reserve(axes.size() + buttons.size() + switches.size());
            for (qsizetype i = 0; i < axes.size(); ++i)
                result.insert(RawGamepad::controlName(QLatin1String(RawGamepad::AxisPrefix), i), axes[i]);
            for (qsizetype i = 0; i < buttons.size(); ++i)
                result.insert(RawGamepad::controlName(QLatin1String(RawGamepad::ButtonPrefix), i), buttons[i] ? 1.0f : 0.0f);
            for (qsizetype i = 0; i < switches.size(); ++i)
                result.insert(RawGamepad::controlName(QLatin1String(RawGamepad::SwitchPrefix), i), static_cast<float>(switches[i]));
            return result;
        }
    };

    static ControllerSample readControls(IGameInputReading& reading) {
        const auto axisCount = reading.GetControllerAxisCount();
        const auto buttonCount = reading.GetControllerButtonCount();
        const auto switchCount = reading.GetControllerSwitchCount();
        QList<float> axes(axisCount);
        QList<bool> buttons(buttonCount);
        QList<GameInputSwitchPosition> switches(switchCount);
        const auto axesRead = (std::min)(axisCount, reading.GetControllerAxisState(axisCount, axes.data()));
        const auto buttonsRead = (std::min)(buttonCount, reading.GetControllerButtonState(buttonCount, buttons.data()));
        const auto switchesRead = (std::min)(switchCount, reading.GetControllerSwitchState(switchCount, switches.data()));
        axes.resize(axesRead);
        buttons.resize(buttonsRead);
        switches.resize(switchesRead);
        return {std::move(axes), std::move(buttons), std::move(switches)};
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
#ifdef RML_GAMEINPUT_V3
                    const GameInputDeviceInfo* info{};
                    if (FAILED(device->GetDeviceInfo(&info))) return;
#else
                    const auto* info = device->GetDeviceInfo();
#endif
                    if (!info) return;
                    const bool hasControllerKind = (info->supportedInput & ControllerKinds) != 0;
                    const auto usage = info->usage;
                    const bool controllerUsage = usage.page == ReportControls::DesktopPage &&
                        (usage.id == ReportControls::JoystickUsage || usage.id == ReportControls::GamepadUsage ||
                            usage.id == ReportControls::MultiAxisControllerUsage);
                    if (!hasControllerKind && !controllerUsage) return;
#ifdef RML_GAMEINPUT_V3
                    const auto* descriptor = info->controllerInfo;
                    constexpr uint32_t MaximumDescribedControls = 1024;
                    const bool hasDescribedControls = descriptor &&
                        descriptor->controllerAxisCount <= MaximumDescribedControls &&
                        descriptor->controllerButtonCount <= MaximumDescribedControls &&
                        descriptor->controllerSwitchCount <= MaximumDescribedControls &&
                        ((descriptor->controllerAxisCount && descriptor->controllerAxisLabels) ||
                            (descriptor->controllerButtonCount && descriptor->controllerButtonLabels) ||
                            (descriptor->controllerSwitchCount && descriptor->controllerSwitchInfo));
                    auto reportControls = !hasDescribedControls && (info->supportedInput & GameInputKindRawDeviceReport) != 0
                        ? ReportControls::open(QString::fromUtf8(info->pnpPath)) : nullptr;
                    if (!hasControllerKind && !reportControls) return;
                    const auto* name = info->displayName;
#else
                    auto reportControls = ReportControls::fromDeviceInfo(*info);
                    if (!hasControllerKind && reportControls.isEmpty()) return;
                    const auto* name = info->displayName ? info->displayName->data : nullptr;
#endif
                    const auto& bytes = info->deviceId.value;
                    const auto id = QLatin1String(GameInputDevicePrefix) + QString::fromLatin1(
                        QByteArray(reinterpret_cast<const char*>(bytes), sizeof(bytes)).toHex());
                    ConnectedDevice connected{ComPtr<IGameInputDevice>(device), id, info->supportedInput,
                        {info->vendorId, info->productId, QString::fromUtf8(name)},
                        std::move(reportControls), info->supportedRumbleMotors, info->supportedSystemButtons};
                    connected.capabilities.inputKinds = static_cast<quint32>(info->supportedInput);
                    connected.capabilities.systemButtons = static_cast<quint32>(info->supportedSystemButtons);
                    connected.capabilities.forceFeedbackMotors = info->forceFeedbackMotorCount;
                    connected.capabilities.inputReportCount = info->inputReportCount;
                    connected.capabilities.outputReportCount = info->outputReportCount;
#ifdef RML_GAMEINPUT_V3
                    if (hasDescribedControls) {
                        using Control = GamepadControlDescription;
                        connected.controlCounts = {descriptor->controllerAxisCount,
                            descriptor->controllerButtonCount, descriptor->controllerSwitchCount};
                        auto definitions = std::make_shared<QHash<QString, Control>>();
                        const auto describe = [&](QLatin1StringView prefix, Control::Kind kind,
                            const GameInputLabel* labels, uint32_t count) {
                            if (!labels) return;
                            for (uint32_t index = 0; index < count; ++index)
                                definitions->insert(RawGamepad::controlName(prefix, index),
                                    Control{kind, static_cast<int>(index), static_cast<int>(labels[index])});
                        };
                        describe(QLatin1String(RawGamepad::AxisPrefix), Control::Axis,
                            descriptor->controllerAxisLabels, descriptor->controllerAxisCount);
                        describe(QLatin1String(RawGamepad::ButtonPrefix), Control::Button,
                            descriptor->controllerButtonLabels, descriptor->controllerButtonCount);
                        if (descriptor->controllerSwitchInfo) {
                            for (uint32_t index = 0; index < descriptor->controllerSwitchCount; ++index) {
                                const auto& source = descriptor->controllerSwitchInfo[index];
                                Control control{Control::Switch, static_cast<int>(index), 0, static_cast<int>(source.kind)};
                                for (const auto label : source.labels) control.switchLabels.append(static_cast<int>(label));
                                definitions->insert(RawGamepad::controlName(QLatin1String(RawGamepad::SwitchPrefix), index),
                                    std::move(control));
                            }
                        }
                        connected.controlDescriptions = std::move(definitions);
                    }
                    const auto appendReports = [](auto& target, const auto* reports, uint32_t count) {
                        if (!reports || count > MaximumDescribedControls) return;
                        target.reserve(count);
                        for (uint32_t index = 0; index < count; ++index)
                            target.append(GamepadCapabilities::Report{reports[index].id, reports[index].size});
                    };
                    appendReports(connected.capabilities.inputReports, info->inputReportInfo, info->inputReportCount);
                    appendReports(connected.capabilities.outputReports, info->outputReportInfo, info->outputReportCount);
                    if (const auto* wheel = info->racingWheelInfo)
                        connected.capabilities.wheel = GamepadCapabilities::Wheel{
                            wheel->maxWheelAngle, wheel->hasClutch, wheel->hasHandbrake, wheel->hasPatternShifter};
                    connected.gameInputIdentity = describeIdentity(*info);
                    describeAdvancedCapabilities(connected, *info);
                    if ((connected.kinds & GameInputKindController) != 0 &&
                        FAILED(device->CreateInputMapper(connected.mapper.GetAddressOf()))) connected.mapper.Reset();
#endif
                    self.devices_.emplace(++self.lastId_, std::move(connected));
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
#ifdef RML_GAMEINPUT_V3
    std::map<quint64, GameInputCallbackToken> rawCallbacks_;
#endif
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
bool GamepadSource::updateRumble(const QHash<QString, GamepadRumble>& outputs) { return state_->updateRumble(outputs); }
void GamepadSource::stopRumble() noexcept { state_->stopRumble(); }
}
