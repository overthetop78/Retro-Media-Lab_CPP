#include <RetroMediaLab/Emulation/HardwareSettings.h>
#include <algorithm>
#include <utility>

namespace RetroMediaLab::Emulation::HardwareSettings {
namespace {
constexpr auto CpuTab = "Emulation.Tab.Cpu";
constexpr auto RamTab = "Emulation.Tab.Ram";
constexpr auto VideoTab = "Emulation.Tab.Video";
constexpr auto Cpu = "Emulation.Cpu.Processor";
constexpr auto CpuModel = "Emulation.Cpu.Model";
constexpr auto CpuSpeed = "Emulation.Cpu.SpeedOriginal";
constexpr auto Memory = "Emulation.Memory.Main";
constexpr auto Resolution = "Emulation.Video.Resolution";
constexpr auto CpuModelSuffix = "_cpu_model";
constexpr auto CpuSpeedSuffix = "_cpu_frequency";
constexpr auto MainMemorySuffix = "_main_memory";
constexpr auto DisplayMemorySuffix = "_display_memory";
constexpr auto ResolutionSuffix = "_display_resolution";
constexpr auto Enabled = "enabled";
constexpr auto Disabled = "disabled";
constexpr auto Megahertz = "MHz";
constexpr auto Kilohertz = "kHz";
constexpr auto Byte = "B";
constexpr auto Kibibyte = "KiB";
constexpr qint64 HertzPerMegahertz = 1'000'000, HertzPerKilohertz = 1'000, BytesPerKibibyte = 1024;
constexpr int FrequencyDecimals = 6, KilohertzDecimals = 3, MaximumDecimals = 15;

QString withUnit(QString value, const QString& unit) {
    return unit.isEmpty() ? value : value + u' ' + unit;
}

QLocale ungrouped(QLocale locale) {
    locale.setNumberOptions(locale.numberOptions() | QLocale::OmitGroupSeparator);
    return locale;
}

QString decimal(double value, int minimum, int maximum, const QLocale& locale) {
    auto number = ungrouped(locale).toString(value, 'f', maximum);
    for (int digits = maximum; digits > minimum && number.endsWith(locale.zeroDigit()); --digits)
        number.chop(locale.zeroDigit().size());
    if (number.endsWith(locale.decimalPoint())) number.chop(locale.decimalPoint().size());
    return number;
}
}

SettingsChoice invariantChoice(QString id, QString text) {
    return {std::move(id), {}, std::move(text)};
}

SettingsField information(QString id, QString label, QString value, QString help) {
    return {.id = std::move(id), .labelResourceKey = std::move(label), .editor = SettingsEditor::Information,
        .value = std::move(value), .explanationResourceKey = std::move(help)};
}

SettingsField number(QString id, QString label, QString value, QString help) {
    return {.id = std::move(id), .labelResourceKey = std::move(label), .editor = SettingsEditor::Number,
        .value = std::move(value), .explanationResourceKey = std::move(help)};
}

SettingsField toggle(QString id, QString label, bool value, QString help) {
    return {.id = std::move(id), .labelResourceKey = std::move(label), .editor = SettingsEditor::Toggle,
        .value = QLatin1String(value ? Enabled : Disabled), .explanationResourceKey = std::move(help),
        .enabledValue = QLatin1String(Enabled), .disabledValue = QLatin1String(Disabled)};
}

SettingsField select(QString id, QString label, QString value, QList<SettingsChoice> choices,
        QString help, bool enabled) {
    return {.id = std::move(id), .labelResourceKey = std::move(label), .editor = SettingsEditor::Choice,
        .value = std::move(value), .choices = std::move(choices), .enabled = enabled,
        .explanationResourceKey = std::move(help)};
}

SettingsField audioSelection(const QHash<QString, QString>& options, QString id, QString label,
        int fallback, const QList<int>& values, const QString& unit, const QLocale& locale) {
    const auto displayLocale = ungrouped(locale);
    QList<SettingsChoice> choices;
    choices.reserve(values.size());
    for (const int value : values)
        choices.append(invariantChoice(QString::number(value), withUnit(displayLocale.toString(value), unit)));
    auto selected = options.value(id, QString::number(fallback));
    return select(std::move(id), std::move(label), std::move(selected), std::move(choices));
}

void appendBlocks(QList<SettingsBlock>& blocks, const Description& description) {
    const auto& prefix = description.fieldPrefix;
    auto cpuModel = information(prefix + QLatin1String(CpuModelSuffix), QLatin1String(CpuModel), description.cpuResourceKey);
    cpuModel.choices.append(SettingsChoice{description.cpuResourceKey, description.cpuResourceKey, {}});
    SettingsBlock cpu{QLatin1String(Cpu), QLatin1String(CpuTab), QLatin1String(Cpu), {std::move(cpuModel)}};
    if (description.cpuFrequency)
        cpu.fields.append(information(prefix + QLatin1String(CpuSpeedSuffix), QLatin1String(CpuSpeed), *description.cpuFrequency));
    blocks.append(std::move(cpu));
    SettingsBlock memory{QLatin1String(Memory), QLatin1String(RamTab), QLatin1String(Memory), {}};
    if (description.mainMemory)
        memory.fields.append(information(prefix + QLatin1String(MainMemorySuffix), QLatin1String(Memory), *description.mainMemory));
    if (description.displayMemory)
        memory.fields.append(information(prefix + QLatin1String(DisplayMemorySuffix), description.displayMemoryResourceKey, *description.displayMemory));
    if (!memory.fields.isEmpty()) blocks.append(std::move(memory));
    if (description.resolution)
        blocks.append(SettingsBlock{QLatin1String(Resolution), QLatin1String(VideoTab), QLatin1String(Resolution),
            {information(prefix + QLatin1String(ResolutionSuffix), QLatin1String(Resolution), *description.resolution)}});
}

void appendBlocks(QList<SettingsBlock>& blocks, const Description& description,
        SettingsPresentation& presentation, bool compactCpu) {
    appendBlocks(blocks, description);
    if (compactCpu) presentation.compactBlocks.insert(QLatin1String(Cpu));
}

QString formatFrequency(qint64 hertz, bool useKilohertz, const QLocale& locale) {
    if (!useKilohertz || hertz >= HertzPerMegahertz) return formatMegahertz(hertz, 0, FrequencyDecimals, locale);
    return withUnit(decimal(double(hertz) / HertzPerKilohertz, 0, KilohertzDecimals, locale), QLatin1String(Kilohertz));
}

QString formatMegahertz(qint64 hertz, int minimumDecimals, int maximumDecimals, const QLocale& locale) {
    const int maximum = std::clamp(maximumDecimals, 0, MaximumDecimals);
    const int minimum = std::clamp(minimumDecimals, 0, maximum);
    return withUnit(decimal(double(hertz) / HertzPerMegahertz, minimum, maximum, locale), QLatin1String(Megahertz));
}

QString formatMemory(qint64 bytes) {
    if (bytes < BytesPerKibibyte) return withUnit(QString::number(bytes), QLatin1String(Byte));
    return withUnit(QString::number(bytes / BytesPerKibibyte), QLatin1String(Kibibyte));
}

}
