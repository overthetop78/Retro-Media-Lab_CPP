#pragma once

#include <RetroMediaLab/Emulation/Sdk.h>

#include <QLocale>
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <utility>

namespace RetroMediaLab {
template<class Localize>
QString moduleChoiceDisplay(const Emulation::SettingsChoice& choice, const QString& unitKey,
                            const Localize& localize) {
    auto text = choice.invariantDisplayValue.isEmpty()
        ? (choice.displayResourceKey.isEmpty() ? QString{} : localize(choice.displayResourceKey))
        : choice.invariantDisplayValue;
    if (unitKey.isEmpty()) return text;
    bool numeric = false;
    text.toDouble(&numeric);
    if (numeric) text += QLatin1Char(' ') + localize(unitKey);
    return text;
}

template<class Localize>
QString moduleFieldDisplay(const Emulation::SettingsField& field, const QString& unitKey,
                            const Localize& localize) {
    const auto choice = std::ranges::find(field.choices, field.value, &Emulation::SettingsChoice::id);
    const auto selected = choice == field.choices.end() ? field.choices.begin() : choice;
    if (selected != field.choices.end()) return moduleChoiceDisplay(*selected, unitKey, localize);
    return moduleChoiceDisplay(Emulation::SettingsChoice{{}, {}, field.value}, unitKey, localize);
}

class ModuleNumericSetting {
public:
    static std::optional<ModuleNumericSetting> from(Emulation::SettingsNumericRange range) {
        if (range.useChoiceLabels || !std::isfinite(range.minimum) || !std::isfinite(range.maximum) ||
            !std::isfinite(range.step) || range.step <= 0 || range.maximum < range.minimum) return {};
        const auto intervals = std::ceil((range.maximum - range.minimum) / range.step);
        if (!std::isfinite(intervals) || intervals >= std::numeric_limits<int>::max()) return {};
        return ModuleNumericSetting(std::move(range), int(intervals));
    }

    int minimum() const { return range_.automaticValue ? Automatic : 0; }
    int maximum() const { return maximum_; }
    bool vertical() const { return range_.vertical; }

    int index(const QString& value) const {
        if (range_.automaticValue && value == *range_.automaticValue) return Automatic;
        bool valid = false;
        const double number = value.toDouble(&valid);
        if (!valid || !std::isfinite(number) || maximum_ == 0) return 0;
        const double clamped = std::clamp(number, range_.minimum, range_.maximum);
        if (clamped >= std::midpoint(numericValue(maximum_ - 1), range_.maximum)) return maximum_;
        return std::clamp(int(std::lround((clamped - range_.minimum) / range_.step)), 0, maximum_ - 1);
    }

    QString value(int index) const {
        if (index == Automatic && range_.automaticValue) return *range_.automaticValue;
        return QString::number(numericValue(index), 'g', Precision);
    }

    QString dialogDisplay(int index, const QString& automatic, const QString& translatedUnit) const {
        if (index == Automatic && range_.automaticValue) return automatic;
        constexpr int DecimalPlaces = 8;
        auto value = QString::number(numericValue(index), 'f', DecimalPlaces);
        while (value.endsWith(u'0')) value.chop(1);
        if (value.endsWith(u'.')) value.chop(1);
        if (!translatedUnit.isEmpty()) value += u' ' + translatedUnit;
        return value + range_.unitSuffix;
    }

    QString display(int index, const QString& automatic, const QString& translatedUnit) const {
        if (index == Automatic && range_.automaticValue) return automatic;
        const auto value = QLocale().toString(numericValue(index), 'g', Precision);
        const auto& unit = range_.unitSuffix.isEmpty() ? translatedUnit : range_.unitSuffix;
        return unit.isEmpty() ? value : value + QLatin1Char(' ') + unit;
    }

private:
    static constexpr int Automatic = -1;
    static constexpr int Precision = 15;
    ModuleNumericSetting(Emulation::SettingsNumericRange range, int maximum)
        : range_(std::move(range)), maximum_(maximum) {}

    double numericValue(int index) const {
        const auto position = std::clamp(index, 0, maximum_);
        if (position == maximum_) return range_.maximum;
        return std::clamp(std::fma(double(position), range_.step, range_.minimum), range_.minimum, range_.maximum);
    }

    Emulation::SettingsNumericRange range_;
    int maximum_;
};
}
