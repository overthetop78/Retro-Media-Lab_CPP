#include "ConversionSettings.h"
#include <ErrorKeys.h>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <stdexcept>
namespace RetroMediaLab {
using namespace ConversionConstants;
const QVector<TagDefinition>& conversionPresets() {
    static const QVector<TagDefinition> values{
        {PresetPattern0, PresetKey0}, {PresetPattern1, PresetKey1}, {PresetPattern2, PresetKey2},
        {PresetPattern3, PresetKey3}, {PresetPattern4, PresetKey4}};
    return values;
}
const QVector<TagDefinition>& conversionVariables() {
    static const QVector<TagDefinition> values{
        {Token0, VariableKey0}, {Token1, VariableKey1}, {Token2, VariableKey2}, {Token3, VariableKey3},
        {Token4, VariableKey4}, {Token5, VariableKey5}, {Token6, VariableKey6}, {Token7, VariableKey7},
        {Token8, VariableKey8}, {Token9, VariableKey9}};
    return values;
}
void ConversionSettings::rememberCustomPattern() {
    for (const auto& preset : conversionPresets())
        if (tagPattern.compare(preset.value, Qt::CaseInsensitive) == 0) return;
    const auto pattern = tagPattern.trimmed();
    if (pattern.isEmpty()) return;
    recentCustomTagPatterns.removeIf([&](const QString& value) { return value.compare(pattern, Qt::CaseInsensitive) == 0; });
    recentCustomTagPatterns.prepend(pattern);
    while (recentCustomTagPatterns.size() > RecentLimit) recentCustomTagPatterns.removeLast();
}
QJsonObject ConversionSettings::toJson() const {
    return {{AddTags, addTags}, {Pattern, tagPattern}, {Recent, QJsonArray::fromStringList(recentCustomTagPatterns)}};
}
ConversionSettings ConversionSettings::fromJson(const QJsonValue& value) {
    if (value.isUndefined()) return {};
    if (!value.isObject()) throw std::runtime_error(ErrorKeys::InvalidPreferencesFile);
    const auto object = value.toObject();
    if (!object.value(AddTags).isBool() || !object.value(Pattern).isString() || !object.value(Recent).isArray())
        throw std::runtime_error(ErrorKeys::InvalidPreferencesFile);
    ConversionSettings result;
    result.addTags = object.value(AddTags).toBool();
    result.tagPattern = object.value(Pattern).toString();
    for (const auto item : object.value(Recent).toArray()) {
        if (!item.isString()) throw std::runtime_error(ErrorKeys::InvalidPreferencesFile);
        const auto pattern = item.toString().trimmed();
        if (!pattern.isEmpty() && !result.recentCustomTagPatterns.contains(pattern, Qt::CaseInsensitive)
            && result.recentCustomTagPatterns.size() < RecentLimit) result.recentCustomTagPatterns.append(pattern);
    }
    return result;
}
QString conversionExample(const QString& pattern, int index) {
    struct Example { const char* name; const char* family; const char* format; const char* extension; };
    static const QVector<Example> examples{
        {ExampleName, PC, Format720, IMA}, {Workbench, Amiga, DD, ADF},
        {GameName, ST, Format720, ST}, {ArchiveName, PC, Format1440, IMG}};
    const auto& sample = examples.at(qAbs(index % examples.size()));
    auto rendered = pattern;
    rendered.replace(LegacyTag, QString(sample.family) + Dash + sample.format, Qt::CaseInsensitive);
    rendered.replace(NameToken, sample.name, Qt::CaseInsensitive);
    rendered.replace(FamilyToken, sample.family, Qt::CaseInsensitive);
    rendered.replace(FormatToken, sample.format, Qt::CaseInsensitive);
    rendered.replace(ExtensionToken, sample.extension, Qt::CaseInsensitive);
    const auto timestamp = QDateTime::fromString(ExampleTimestamp, Qt::ISODate);
    const QVector<const char*> formats{DateFormat0, DateFormat1, DateFormat2, DateFormat3, DateFormat4, DateFormat5};
    auto variable = conversionVariables().cbegin() + DateTokenOffset;
    for (const auto* format : formats) {
        rendered.replace(variable->value, timestamp.toString(format), Qt::CaseInsensitive);
        ++variable;
    }
    if (!pattern.contains(NameToken, Qt::CaseInsensitive)) rendered += sample.name;
    return rendered + Dot + QString(sample.extension).toLower();
}
}
