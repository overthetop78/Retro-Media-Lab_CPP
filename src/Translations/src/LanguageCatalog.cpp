#include <ErrorKeys.h>
#include <RetroMediaLab/Translations/TranslationConstants.h>
#include <RetroMediaLab/Translations/LanguageCatalog.h>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QLocale>
#include <algorithm>
#include <set>
#include <stdexcept>
namespace RetroMediaLab::Translations {
const std::vector<Language>& availableLanguages() {
    static const auto languages = [] {
        QFile file(TranslationConstants::TranslationsLanguagesJson);
        if (!file.open(QIODevice::ReadOnly)) throw std::runtime_error(ErrorKeys::CannotReadLanguageList);
        const auto document = QJsonDocument::fromJson(file.readAll());
        if (!document.isArray()) throw std::runtime_error(ErrorKeys::InvalidLanguageList);
        std::vector<Language> result;
        std::set<std::string> codes;
        for (const auto& entry : document.array()) {
            const auto item = entry.toObject();
            Language language{item.value(TranslationConstants::Code).toString().toStdString(),
                item.value(TranslationConstants::Culture).toString().toStdString(), item.value(TranslationConstants::NativeName).toString().toStdString()};
            if (language.code.empty() || language.culture.empty() || language.nativeName.empty()
                || !codes.insert(language.code).second) throw std::runtime_error(ErrorKeys::InvalidLanguageEntry);
            result.push_back(std::move(language));
        }
        if (!codes.contains(TranslationConstants::EnUS)) throw std::runtime_error(ErrorKeys::MissingEnglishFallbackLanguage);
        return result;
    }();
    return languages;
}
bool isSupportedLanguage(const std::string& code) {
    return std::ranges::any_of(availableLanguages(), [&](const Language& language) { return language.code == code; });
}
std::string resolveLanguage(const std::string& culture) {
    auto normalized = QString::fromStdString(culture).replace(TranslationConstants::Underscore, TranslationConstants::LanguageSeparator);
    for (const auto& language : availableLanguages())
        if (normalized.compare(QString::fromStdString(language.code), Qt::CaseInsensitive) == 0
            || normalized.compare(QString::fromStdString(language.culture), Qt::CaseInsensitive) == 0) return language.code;
    const auto prefix = normalized.section(TranslationConstants::LanguageSeparator, 0, 0);
    const bool supported = std::ranges::any_of(availableLanguages(), [&](const Language& language) {
        return prefix.compare(QString::fromStdString(language.culture).section(TranslationConstants::LanguageSeparator, 0, 0), Qt::CaseInsensitive) == 0;
    });
    if (!supported) return TranslationConstants::EnUS;
    const QLocale requested(normalized);
    // Qt includes script tags in its ordered UI languages (for example traditional Chinese).
    for (const auto& language : availableLanguages()) {
        const QLocale candidate(QString::fromStdString(language.culture));
        if (requested.language() == candidate.language() && requested.script() == candidate.script()
            && requested.territory() == candidate.territory()) return language.code;
    }
    for (const auto& language : availableLanguages()) {
        const QLocale candidate(QString::fromStdString(language.culture));
        if (requested.language() == candidate.language() && requested.script() == candidate.script()) return language.code;
    }
    for (const auto& language : availableLanguages())
        if (prefix.compare(QString::fromStdString(language.culture).section(TranslationConstants::LanguageSeparator, 0, 0), Qt::CaseInsensitive) == 0)
            return language.code;
    return TranslationConstants::EnUS;
}
}
