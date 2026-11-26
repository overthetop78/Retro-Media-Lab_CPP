#pragma once

#include <RetroMediaLab/Platform/Gamepads.h>
#include <GameInput.h>
#include <algorithm>
#include <span>

namespace RetroMediaLab::Platform {
class GameInputReportControls {
public:
    GameInputReportControls() = default;

    explicit GameInputReportControls(std::span<const GameInputRawDeviceReportInfo> reports) {
        qsizetype axes{}, buttons{}, switches{};
        for (const auto& report : reports) {
            if (report.kind != GameInputRawInputReport || !report.items || report.itemCount > MaximumControls)
                continue;
            for (uint32_t index = 0; index < report.itemCount; ++index) {
                if (bindings_.size() == MaximumControls) return;
                const auto& item = report.items[index];
                if (!item.usages || item.usageCount != 1 ||
                    (item.flags & IgnoredFlags) != 0 || item.logicalMax <= item.logicalMin)
                    continue;
                const auto usage = item.usages[0];
                const bool hat = usage.page == DesktopPage && usage.id == HatSwitchUsage;
                const auto kind = hat ? Kind::Switch : usage.page == ButtonPage ? Kind::Button : Kind::Axis;
                const auto control = hat ? switches++ : kind == Kind::Button ? buttons++ : axes++;
                const auto prefix = hat ? RawGamepad::SwitchPrefix :
                    kind == Kind::Button ? RawGamepad::ButtonPrefix : RawGamepad::AxisPrefix;
                bindings_.append({report.id, index, kind, item.logicalMin, item.logicalMax,
                    RawGamepad::controlName(QLatin1String(prefix), control)});
            }
        }
    }

    static GameInputReportControls fromDeviceInfo(const GameInputDeviceInfo& info) {
        if (!info.inputReportInfo || info.inputReportCount > MaximumControls ||
            (info.supportedInput & GameInputKindRawDeviceReport) == 0)
            return {};
        return GameInputReportControls({info.inputReportInfo, info.inputReportCount});
    }

    bool isEmpty() const { return bindings_.isEmpty(); }

    bool decode(uint32_t reportId, auto readItem, QHash<QString, float>& controls) const {
        if (std::ranges::none_of(bindings_, [reportId](const auto& binding) { return binding.report == reportId; }))
            return false;
        controls.reserve(bindings_.size());
        for (const auto& binding : bindings_) {
            int64_t raw{};
            const auto neutral = binding.kind == Kind::Axis ? RawGamepad::AxisCenter : 0.0f;
            const bool read = binding.report == reportId && readItem(binding.item, raw);
            controls.insert(binding.name, read ? binding.normalized(raw) : neutral);
        }
        return true;
    }

    static constexpr uint16_t DesktopPage = 0x01;
    static constexpr uint16_t JoystickUsage = 0x04;
    static constexpr uint16_t GamepadUsage = 0x05;
    static constexpr uint16_t MultiAxisControllerUsage = 0x08;

private:
    static constexpr uint16_t HatSwitchUsage = 0x39;
    static constexpr uint16_t ButtonPage = 0x09;
    static constexpr uint32_t MaximumControls = 1024;
    static constexpr uint32_t IgnoredFlags = GameInputConstantItem | GameInputArrayItem |
        GameInputRelativeItem | GameInputBufferedItem;

    enum class Kind { Axis, Button, Switch };
    struct Binding {
        uint32_t report;
        uint32_t item;
        Kind kind;
        int64_t minimum;
        int64_t maximum;
        QString name;

        float normalized(int64_t value) const {
            if (kind == Kind::Button) return value > minimum && value <= maximum ? 1.0f : 0.0f;
            const auto offset = static_cast<double>(value) - static_cast<double>(minimum);
            if (kind == Kind::Switch) {
                constexpr int HatPositions = 8;
                return offset >= 0.0 && offset < HatPositions && value <= maximum
                    ? static_cast<float>(offset + 1.0) : 0.0f;
            }
            const auto range = static_cast<double>(maximum) - static_cast<double>(minimum);
            return range > 0.0 ? static_cast<float>(std::clamp(offset / range, 0.0, 1.0)) : RawGamepad::AxisCenter;
        }
    };

    QList<Binding> bindings_;
};
}
