#include <RetroMediaLab/Emulation/NativeCoreOptions.h>
#include <RetroMediaLab/Emulation/Messages.h>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace RetroMediaLab::Emulation {
namespace {
constexpr qsizetype MaximumVariableCount = 4096;
constexpr qsizetype MaximumDefinitions = 1024;
constexpr qsizetype MaximumCategories = 256;
constexpr QChar DescriptionSeparator = u';';
constexpr QChar ValueSeparator = u'|';
constexpr auto InvalidOptionDiagnostic = "%1 = %2";
constexpr auto ErrorKey = "Emulation.Error.EmulatorRejected";

[[noreturn]] void throwInvalidOption(const QString& key, const QString& value) {
    const auto diagnostic = QString::fromLatin1(InvalidOptionDiagnostic).arg(key, value);
    const auto cause = std::make_exception_ptr(std::invalid_argument(diagnostic.toStdString()));
    std::rethrow_exception(translateError(cause, MessageCategory::Emulator, MessageCode::OptionInvalid));
}

QString localized(const char* text, QString fallback) {
    return text ? QString::fromUtf8(text) : std::move(fallback);
}

template<class Entry>
QHash<QString, const Entry*> indexNativeTable(const Entry* table, qsizetype limit) {
    QHash<QString, const Entry*> entries;
    if (!table) return entries;
    for (qsizetype index = 0; index < limit && table[index].key; ++index) {
        const auto key = QString::fromUtf8(table[index].key);
        if (entries.contains(key)) throw std::invalid_argument(ErrorKey);
        entries.insert(key, table + index);
    }
    return entries;
}

template<class Definition>
QList<NativeCoreOption> copyDefinitions(const Definition* english, const Definition* local,
    const QHash<QString, const NativeCoreOptionCategory*>& categories = {}) {
    QList<NativeCoreOption> result;
    if (!english) return result;
    const auto translated = indexNativeTable(local, MaximumDefinitions);
    for (qsizetype index = 0; index < MaximumDefinitions && english[index].key; ++index) {
        const auto& source = english[index];
        NativeCoreOption option;
        option.key = QString::fromUtf8(source.key);
        const auto* translation = translated.value(option.key);
        option.label = localized(translation ? translation->name : nullptr, localized(source.name, option.key));
        option.description = localized(translation ? translation->description : nullptr, QString::fromUtf8(source.description));
        if constexpr (std::is_same_v<Definition, NativeCoreOptionDefinitionV2>) {
            option.category = QString::fromUtf8(source.category);
            if (!categories.contains(option.category)) option.category.clear();
            option.categorizedName = localized(translation ? translation->categorizedName : nullptr,
                QString::fromUtf8(source.categorizedName));
            option.categorizedDescription = localized(translation ? translation->categorizedDescription : nullptr,
                QString::fromUtf8(source.categorizedDescription));
        }
        QHash<QString, const char*> translatedLabels;
        if (translation) {
            for (const auto& choice : translation->values) {
                if (!choice.value) break;
                translatedLabels.insert(QString::fromUtf8(choice.value), choice.label);
            }
        }
        for (const auto& choice : source.values) {
            if (!choice.value) break;
            const auto value = QString::fromUtf8(choice.value);
            option.choices.append(value);
            option.choiceLabels.insert(value, localized(translatedLabels.value(value), localized(choice.label, value)));
        }
        const auto declaredDefault = QString::fromUtf8(source.defaultValue);
        option.defaultValue = option.choices.contains(declaredDefault) ? declaredDefault : option.choices.value(0, QStringLiteral(""));
        result.append(std::move(option));
    }
    return result;
}
}

NativeCoreLegacyOptions::NativeCoreLegacyOptions(QHash<QString, QString> configured)
    : values_(std::move(configured)) {}

bool NativeCoreLegacyOptions::registerVariables(const NativeCoreVariable* definitions) {
    if (!definitions) return false;
    for (qsizetype i = 0; i < MaximumVariableCount; ++i) {
        const auto& definition = definitions[i];
        if (!definition.key) return true;
        if (!definition.value) throw std::invalid_argument(ErrorKey);

        const auto key = QString::fromUtf8(definition.key);
        const auto description = QString::fromUtf8(definition.value);
        const auto separator = description.indexOf(DescriptionSeparator);
        if (separator < 0) throw std::invalid_argument(ErrorKey);
        auto choices = description.sliced(separator + 1).trimmed().split(ValueSeparator, Qt::SkipEmptyParts);
        if (choices.isEmpty()) throw std::invalid_argument(ErrorKey);
        const auto initial = choices.front();
        const auto current = values_.value(key, initial);
        if (!choices.contains(current, Qt::CaseSensitive)) throwInvalidOption(key, current);

        NativeCoreOption option;
        option.key = key;
        option.label = description.first(separator).trimmed();
        option.defaultValue = initial;
        option.currentValue = current;
        option.choices = std::move(choices);
        option.visible = visibility_.value(key, true);
        const auto index = indices_.constFind(key);
        if (index != indices_.cend()) options_[*index] = std::move(option);
        else {
            const auto position = options_.size();
            options_.append(std::move(option));
            indices_.insert(key, position);
        }
        values_.insert(key, current);
    }
    throw std::invalid_argument(ErrorKey);
}

bool NativeCoreLegacyOptions::registerDefinitions(const NativeCoreOptionDefinitionV1* english,
    const NativeCoreOptionDefinitionV1* local) {
    if (!english) return false;
    replaceDefinitions(copyDefinitions(english, local));
    categories_.clear();
    return true;
}

bool NativeCoreLegacyOptions::registerDefinitions(const NativeCoreOptionTableV2* english,
    const NativeCoreOptionTableV2* local) {
    if (!english) return false;
    const auto categoryKeys = indexNativeTable(english->categories, MaximumCategories);
    const auto translations = indexNativeTable(local ? local->categories : nullptr, MaximumCategories);
    QList<NativeCoreOptionCategoryInfo> categories;
    if (english->categories) {
        for (qsizetype index = 0; index < MaximumCategories && english->categories[index].key; ++index) {
            const auto& source = english->categories[index];
            const auto key = QString::fromUtf8(source.key);
            const auto* translation = translations.value(key);
            categories.append({key, localized(translation ? translation->name : nullptr, localized(source.name, key)),
                localized(translation ? translation->description : nullptr, QString::fromUtf8(source.description))});
        }
    }
    replaceDefinitions(copyDefinitions(english->definitions, local ? local->definitions : nullptr, categoryKeys));
    categories_ = std::move(categories);
    return true;
}

void NativeCoreLegacyOptions::replaceDefinitions(QList<NativeCoreOption> options) {
    QHash<QString, qsizetype> indices;
    auto values = values_;
    for (qsizetype position = 0; position < options.size(); ++position) {
        auto& option = options[position];
        if (indices.contains(option.key)) throw std::invalid_argument(ErrorKey);
        indices.insert(option.key, position);
        option.currentValue = values.value(option.key, option.defaultValue);
        option.visible = visibility_.value(option.key, true);
        values.insert(option.key, option.currentValue);
    }
    indices_ = std::move(indices);
    values_ = std::move(values);
    options_ = std::move(options);
    modern_ = true;
}

bool NativeCoreLegacyOptions::returnValue(NativeCoreVariable* variable) {
    if (!variable) return modern_;
    if (!variable->key) {
        variable->value = nullptr;
        return modern_;
    }
    const auto key = QString::fromUtf8(variable->key);
    const auto value = values_.constFind(key);
    if (value == values_.cend()) {
        variable->value = nullptr;
        return modern_;
    }
    const auto text = strings_.try_emplace(key, *value).first;
    variable->value = text->second.pointer();
    return true;
}

bool NativeCoreLegacyOptions::returnUpdated(bool* updated) {
    if (!updated) return false;
    *updated = updated_.exchange(false);
    return true;
}

bool NativeCoreLegacyOptions::applyVisibility(const NativeCoreOptionVisibility& display) {
    const auto key = QString::fromUtf8(display.key);
    if (key.trimmed().isEmpty()) return false;
    visibility_.insert(key, display.visible);
    const auto index = indices_.constFind(key);
    if (index != indices_.cend()) options_[*index].visible = display.visible;
    return true;
}

void NativeCoreLegacyOptions::captureDisplayUpdate(DisplayUpdate callback) noexcept {
    displayUpdate_ = callback;
}

void NativeCoreLegacyOptions::releaseNativeReferences() noexcept {
    displayUpdate_ = nullptr;
    strings_.clear();
}

bool NativeCoreLegacyOptions::setNativeValue(const NativeCoreVariable& variable) {
    const auto key = QString::fromUtf8(variable.key);
    return variable.value && !key.trimmed().isEmpty() && trySetValue(key, QString::fromUtf8(variable.value));
}

bool NativeCoreLegacyOptions::trySetValue(const QString& key, const QString& value) {
    const auto index = indices_.constFind(key);
    if (index == indices_.cend()) return false;
    auto& option = options_[*index];
    if (!option.choices.isEmpty() && !option.choices.contains(value, Qt::CaseSensitive)) return false;
    if (!modern_ && option.currentValue == value) return true;
    values_.insert(key, value);
    option.currentValue = value;
    strings_.erase(key);
    updated_ = true;
    if (displayUpdate_) displayUpdate_();
    return true;
}

void NativeCoreLegacyOptions::setValue(const QString& key, const QString& value) {
    if (!trySetValue(key, value)) throwInvalidOption(key, value);
}

const QHash<QString, QString>& NativeCoreLegacyOptions::values() const noexcept { return values_; }

void NativeCoreLegacyOptions::validateConfiguredValues(const QHash<QString, QString>& configured) const {
    for (const auto& option : options_) {
        if (!configured.contains(option.key) || option.choices.isEmpty() ||
            option.choices.contains(option.currentValue, Qt::CaseSensitive)) continue;

        throwInvalidOption(option.key, option.currentValue);
    }
}

const QList<NativeCoreOption>& NativeCoreLegacyOptions::options() const noexcept { return options_; }
const QList<NativeCoreOptionCategoryInfo>& NativeCoreLegacyOptions::categories() const noexcept { return categories_; }
}
