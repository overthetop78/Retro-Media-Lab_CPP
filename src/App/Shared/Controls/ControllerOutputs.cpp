#include "Shared/Controls/ControllerOutputs.h"
#include <RetroMediaLab/Platform/Gamepads.h>

#include <QComboBox>
#include <QFormLayout>
#include <QSignalBlocker>
#include <algorithm>

namespace RetroMediaLab {
namespace {
using namespace Emulation;
class RumbleSelector final : public QComboBox {
public:
    using QComboBox::QComboBox;
    std::function<void()> refresh;

    void showPopup() override {
        if (refresh) refresh();
        QComboBox::showPopup();
    }
};

constexpr auto RumbleDeviceKey = "Emulation.Controller.Feedback.Device";
constexpr auto NoControllerKey = "Controllers.None";
constexpr auto StrongRumbleKey = "Emulation.Controller.Feedback.Strong";
constexpr auto WeakRumbleKey = "Emulation.Controller.Feedback.Weak";
constexpr auto DeviceMotorFormat = u"%1 · %2";
constexpr int RumbleMotorRole = Qt::UserRole + 1;
constexpr auto RumbleDeviceNameFormat = "%1 #%2";

QList<Platform::GamepadRumbleDevice> namedRumbleDevices(const Platform::GamepadSource& source) {
    auto devices = source.rumbleDevices();
    for (qsizetype index = 0; index < devices.size(); ++index) {
        auto& device = devices[index];
        device.displayName = QString::fromLatin1(RumbleDeviceNameFormat).arg(device.displayName).arg(index + 1);
    }
    return devices;
}

struct RumbleMotorChoice {
    ControllerMotor motor;
    Platform::GamepadRumbleMotor capability;
    const char* resourceKey;
};
constexpr RumbleMotorChoice RumbleMotorChoices[] = {
    {ControllerMotor::LowFrequency, Platform::RumbleLowFrequency, "Controllers.Enum.LowFrequencyMotor"},
    {ControllerMotor::HighFrequency, Platform::RumbleHighFrequency, "Controllers.Enum.HighFrequencyMotor"},
    {ControllerMotor::LeftTrigger, Platform::RumbleLeftTrigger, "Controllers.LeftTrigger"},
    {ControllerMotor::RightTrigger, Platform::RumbleRightTrigger, "Controllers.RightTrigger"},
};
}
void addControllerOutputs(QFormLayout& layout, Emulation::ControllerPortFeedback& port, Platform::GamepadSource& source,
    MachineInputBindings::Translate text, MachineInputBindings::Translate moduleText, std::function<void()> changed) {
    auto* group = layout.parentWidget();
    auto* form = &layout;
    const QString strong(ControllerFeedbackCommands::Strong), weak(ControllerFeedbackCommands::Weak);
    const auto standard = std::ranges::all_of(port.availableCommands, [&](const auto& command) {
        return command == strong || command == weak;
    });
    if (standard) {
        auto* selector = new RumbleSelector(group);
        selector->refresh = [&source, text, selector, target = &port, strong, weak] {
            const QSignalBlocker blocker(selector);
            const auto selected = target->outputs.value(strong, target->outputs.value(weak)).deviceId;
            selector->clear();
            selector->addItem(text(QLatin1String(NoControllerKey)), QString{});
            for (const auto& device : namedRumbleDevices(source)) {
                if ((device.motors & (Platform::RumbleLowFrequency | Platform::RumbleHighFrequency)) != 0)
                    selector->addItem(device.displayName, device.deviceId);
            }
            if (!selected.isEmpty() && selector->findData(selected) < 0) selector->addItem(selected, selected);
            selector->setCurrentIndex(selector->findData(selected));
        };
        selector->refresh();
        QObject::connect(selector, &QComboBox::activated, group, [changed, selector, target = &port] {
            assignControllerRumbleDevice(*target, selector->currentData().toString());
            changed();
        });
        form->addRow(text(QLatin1String(RumbleDeviceKey)), selector);
        return;
    }
    auto commands = port.availableCommands.values();
    std::ranges::sort(commands);
    for (const auto& command : commands) {
        auto* selector = new RumbleSelector(group);
        selector->refresh = [&source, text, selector, target = &port, command] {
            const QSignalBlocker blocker(selector);
            const auto selected = target->outputs.constFind(command);
            int selectedRow = 0;
            selector->clear();
            selector->addItem(text(QLatin1String(NoControllerKey)), QString{});
            const auto addMotor = [&](const QString& name, const QString& id, const RumbleMotorChoice& motor) {
                selector->addItem(QString::fromUtf16(DeviceMotorFormat).arg(name, text(QLatin1String(motor.resourceKey))), id);
                const auto row = selector->count() - 1;
                selector->setItemData(row, static_cast<int>(motor.motor), RumbleMotorRole);
                if (selected != target->outputs.cend() && selected->deviceId == id && selected->motor == motor.motor)
                    selectedRow = row;
            };
            for (const auto& device : namedRumbleDevices(source))
                for (const auto& motor : RumbleMotorChoices)
                    if ((device.motors & motor.capability) != 0)
                        addMotor(device.displayName, device.deviceId, motor);
            if (selectedRow == 0 && selected != target->outputs.cend())
                for (const auto& motor : RumbleMotorChoices)
                    if (motor.motor == selected->motor) addMotor(selected->deviceId, selected->deviceId, motor);
            selector->setCurrentIndex(selectedRow);
        };
        selector->refresh();
        QObject::connect(selector, &QComboBox::activated, group, [changed, selector, target = &port, command] {
            const auto deviceId = selector->currentData().toString();
            if (deviceId.isEmpty()) target->outputs.remove(command);
            else target->outputs.insert(command, {deviceId, static_cast<ControllerMotor>(selector->currentData(RumbleMotorRole).toInt())});
            changed();
        });
        const auto key = port.commandResourceKeys.value(command);
        const auto label = !key.isEmpty() ? moduleText(key) :
            command == strong ? text(QLatin1String(StrongRumbleKey)) :
            command == weak ? text(QLatin1String(WeakRumbleKey)) : command;
        form->addRow(label, selector);
    }
}
}
