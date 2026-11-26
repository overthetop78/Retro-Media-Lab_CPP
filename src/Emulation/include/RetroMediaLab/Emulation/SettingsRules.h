#pragma once

#include <RetroMediaLab/Emulation/Sdk.h>

#include <algorithm>

namespace RetroMediaLab::Emulation::SettingsRules {
using Values = QHash<QString, QString>;

inline Values values(const QList<SettingsBlock>& blocks) {
    Values result;
    for (const auto& block : blocks)
        for (const auto& field : block.fields)
            if (!result.contains(field.id)) result.insert(field.id, field.value);
    return result;
}

inline bool equals(const Values& values, const QString& id, const QString& expected) {
    const auto value = values.constFind(id);
    return value != values.cend() && *value == expected;
}

inline bool fieldVisible(const SettingsDependencies& dependencies, const Values& values, const QString& id) {
    return std::ranges::none_of(dependencies.rules, [&](const SettingsRule& rule) {
        return rule.category == SettingsRuleCategory::VisibleWhenSourceDiffers && rule.targetFieldId == id &&
            equals(values, rule.sourceFieldId, rule.comparedValue);
    });
}

inline bool blockVisible(const SettingsDependencies& dependencies, const Values& values, const SettingsBlock& block) {
    if (!block.visible || dependencies.hiddenTabs.contains(block.tabResourceKey)) return false;
    const auto dependency = dependencies.blocks.constFind(block.id);
    return dependency == dependencies.blocks.cend() || equals(values, dependency->fieldId, dependency->visibleValue);
}

inline QJsonObject changesFor(const SettingsDependencies& dependencies, const Values& base,
    const QString& id, const QString& value) {
    auto current = base;
    current.insert(id, value);
    QJsonObject changes{{id, value}};
    for (const auto& rule : dependencies.rules) {
        if (rule.category != SettingsRuleCategory::MutuallyExclusive) continue;
        const QString* other = rule.sourceFieldId == id ? &rule.targetFieldId :
            rule.targetFieldId == id ? &rule.sourceFieldId : nullptr;
        if (!other || *other == id || !base.contains(*other)) continue;
        if (equals(current, rule.sourceFieldId, rule.comparedValue) ||
            equals(current, rule.targetFieldId, rule.comparedValue)) continue;
        current.insert(*other, rule.comparedValue);
        changes.insert(*other, rule.comparedValue);
    }
    return changes;
}
}
