#pragma once

#include <RetroMediaLab/Platform/Gamepads.h>
#include <QtCore/QByteArray>
#include <QtCore/QByteArrayView>
#include <Windows.h>

extern "C" {
#include <hidsdi.h>
}

#include <algorithm>
#include <bit>
#include <cstring>
#include <limits>
#include <memory>

namespace RetroMediaLab::Platform {
class HidReportControls final {
public:
    static std::shared_ptr<const HidReportControls> open(const QString& path) {
        if (path.isEmpty()) return {};
        auto controls = std::shared_ptr<HidReportControls>(new HidReportControls);
        controls->handle_ = CreateFileW(reinterpret_cast<LPCWSTR>(path.utf16()), 0,
            FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
        if (controls->handle_ == INVALID_HANDLE_VALUE ||
            !HidD_GetPreparsedData(controls->handle_, &controls->data_) || !controls->data_) return {};

        HIDP_CAPS caps{};
        if (HidP_GetCaps(controls->data_, &caps) != HIDP_STATUS_SUCCESS || caps.InputReportByteLength == 0) return {};
        controls->reportSize_ = caps.InputReportByteLength;
        controls->readBindings(caps);
        if (controls->bindings_.isEmpty()) return {};
        controls->numbered_ = std::ranges::any_of(controls->bindings_,
            [](const auto& binding) { return binding.reportId != 0; });
        return controls;
    }

    ~HidReportControls() {
        if (data_) HidD_FreePreparsedData(data_);
        if (handle_ != INVALID_HANDLE_VALUE) CloseHandle(handle_);
    }

    void decode(QByteArrayView bytes, QHash<QString, float>& controls) const {
        QByteArray report;
        if (!bytes.empty()) {
            report = QByteArray(reportSize_, '\0');
            const auto offset = !numbered_ && bytes.size() + 1 == reportSize_ ? 1 : 0;
            std::memcpy(report.data() + offset, bytes.data(), std::min(bytes.size(), qsizetype(reportSize_ - offset)));
        }
        controls.reserve(bindings_.size());
        for (const auto& binding : bindings_) {
            const bool matches = !report.isEmpty() &&
                (binding.reportId == 0 || UCHAR(report.front()) == binding.reportId);
            controls.insert(binding.name, matches ? read(binding, report) : binding.neutral());
        }
    }

    static constexpr USAGE DesktopPage = 0x01;
    static constexpr USAGE JoystickUsage = 0x04;
    static constexpr USAGE GamepadUsage = 0x05;
    static constexpr USAGE MultiAxisControllerUsage = 0x08;

private:
    static constexpr USAGE HatUsage = 0x39;
    static constexpr USHORT MaximumCapabilityCount = 1024;
    static constexpr int MaximumUsageRange = 1024;
    static constexpr ULONG MaximumUsageListLength = 4096;
    static constexpr int HatPositions = 8;

    enum class Kind { Axis, Button, Switch };
    struct Binding {
        Kind kind;
        QString name;
        UCHAR reportId;
        USAGE page;
        USAGE usage;
        USHORT collection;
        USHORT bitSize = 0;
        LONG minimum = 0;
        LONG maximum = 0;
        ULONG usageCount = 0;

        float neutral() const { return kind == Kind::Axis ? RawGamepad::AxisCenter : 0.0f; }
    };

    HidReportControls() = default;
    HidReportControls(const HidReportControls&) = delete;
    HidReportControls& operator=(const HidReportControls&) = delete;

    template<class Capability>
    static auto usages(const Capability& cap) {
        const auto minimum = cap.IsRange ? cap.Range.UsageMin : cap.NotRange.Usage;
        const auto maximum = cap.IsRange ? cap.Range.UsageMax : cap.NotRange.Usage;
        return std::pair{int(minimum), int(maximum)};
    }

    void readBindings(const HIDP_CAPS& caps) {
        qsizetype buttons{}, axes{}, switches{};
        auto buttonCount = caps.NumberInputButtonCaps;
        if (buttonCount > 0 && buttonCount <= MaximumCapabilityCount) {
            QList<HIDP_BUTTON_CAPS> capabilities(buttonCount);
            if (HidP_GetButtonCaps(HidP_Input, capabilities.data(), &buttonCount, data_) == HIDP_STATUS_SUCCESS) {
                for (const auto& cap : std::as_const(capabilities).first(buttonCount)) {
                    const auto [minimum, maximum] = usages(cap);
                    if (maximum < minimum || maximum - minimum > MaximumUsageRange) continue;
                    const auto usageCount = std::clamp(HidP_MaxUsageListLength(HidP_Input, cap.UsagePage, data_),
                        ULONG{1}, MaximumUsageListLength);
                    for (auto usage = minimum; usage <= maximum; ++usage)
                        bindings_.append({Kind::Button, RawGamepad::controlName(
                            QLatin1String(RawGamepad::ButtonPrefix), buttons++), cap.ReportID,
                            cap.UsagePage, USAGE(usage), cap.LinkCollection, 0, 0, 0, usageCount});
                }
            }
        }
        auto valueCount = caps.NumberInputValueCaps;
        if (valueCount > 0 && valueCount <= MaximumCapabilityCount) {
            QList<HIDP_VALUE_CAPS> capabilities(valueCount);
            if (HidP_GetValueCaps(HidP_Input, capabilities.data(), &valueCount, data_) == HIDP_STATUS_SUCCESS) {
                for (const auto& cap : std::as_const(capabilities).first(valueCount)) {
                    const auto [minimum, maximum] = usages(cap);
                    if (maximum < minimum || maximum - minimum > MaximumUsageRange) continue;
                    for (auto usage = minimum; usage <= maximum; ++usage) {
                        const bool hat = cap.UsagePage == DesktopPage && usage == HatUsage;
                        const auto prefix = hat ? RawGamepad::SwitchPrefix : RawGamepad::AxisPrefix;
                        bindings_.append({hat ? Kind::Switch : Kind::Axis, RawGamepad::controlName(
                            QLatin1String(prefix), hat ? switches++ : axes++), cap.ReportID,
                            cap.UsagePage, USAGE(usage), cap.LinkCollection, cap.BitSize, cap.LogicalMin, cap.LogicalMax});
                    }
                }
            }
        }
    }

    float read(const Binding& binding, QByteArray& report) const {
        if (binding.kind == Kind::Button) {
            auto count = binding.usageCount;
            QList<USAGE> active(count);
            const auto status = HidP_GetUsages(HidP_Input, binding.page, binding.collection, active.data(),
                &count, data_, report.data(), reportSize_);
            return status == HIDP_STATUS_SUCCESS &&
                std::as_const(active).first(std::min(qsizetype(count), active.size())).contains(binding.usage) ? 1.0f : 0.0f;
        }

        ULONG raw{};
        if (HidP_GetUsageValue(HidP_Input, binding.page, binding.collection, binding.usage,
            &raw, data_, report.data(), reportSize_) != HIDP_STATUS_SUCCESS) return binding.neutral();
        constexpr auto ValueBits = std::numeric_limits<ULONG>::digits;
        const auto shift = binding.minimum < 0 && binding.bitSize > 0 && binding.bitSize < ValueBits
            ? ValueBits - binding.bitSize : 0;
        const auto value = std::bit_cast<LONG>(raw << shift) >> shift;
        const auto offset = double(value) - binding.minimum;
        if (binding.kind == Kind::Switch)
            return offset >= 0 && offset < HatPositions ? float(offset + 1) : 0.0f;

        const auto range = double(binding.maximum) - binding.minimum;
        return range > 0 ? float(std::clamp(offset / range, 0.0, 1.0)) : 0.0f;
    }

    HANDLE handle_ = INVALID_HANDLE_VALUE;
    PHIDP_PREPARSED_DATA data_{};
    USHORT reportSize_{};
    bool numbered_{};
    QList<Binding> bindings_;
};
}
