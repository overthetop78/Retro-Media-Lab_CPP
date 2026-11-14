#pragma once
#include <array>
#include <QtGlobal>
namespace RetroMediaLab::DisplayConstants {
inline constexpr auto Preference = "interfaceScale";
inline constexpr int DefaultPercent = 100;
inline constexpr std::array<int, 6> Percentages{50, 75, 100, 125, 150, 200};
inline constexpr auto Title = "Display.Size";
inline constexpr auto PercentLabel = "Display.Percent";
inline constexpr auto Argument = "{0}";
inline constexpr auto Card = "interfaceSizeCard";
inline constexpr auto Selector = "interfaceSizeSelector";
inline constexpr auto FactorProperty = "rmlDisplayDensity";
inline constexpr auto AppliedProperty = "rmlAppliedDensity";
inline constexpr int UnboundedSize = 16777215;
inline constexpr qreal Unit = 1.0;
inline bool supported(int value) {
    for (int allowed : Percentages) if (allowed == value) return true;
    return false;
}
}
