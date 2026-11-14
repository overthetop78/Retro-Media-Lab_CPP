#include <RetroMediaLab/Translations/LanguageCatalog.h>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <algorithm>
#include <set>
#include <stdexcept>
namespace RetroMediaLab::Translations {
const std::vector<Language>& availableLanguages() {
    static const auto languages = [] {
        QFile file(":/Translations/Languages.json");
        if (!file.open(QIODevice::ReadOnly)) throw std::runtime_error("Cannot read language list");
        const auto document = QJsonDocument::fromJson(file.readAll());
        if (!document.isArray()) throw std::runtime_error("Invalid language list");
        std::vector<Language> result;
        std::set<std::string> codes;
        for (const auto& entry : document.array()) {
            const auto item = entry.toObject();
            Language language{item.value("code").toString().toStdString(),
                item.value("culture").toString().toStdString(), item.value("nativeName").toString().toStdString()};
            if (language.code.empty() || language.culture.empty() || language.nativeName.empty()
                || !codes.insert(language.code).second) throw std::runtime_error("Invalid language entry");
            result.push_back(std::move(language));
        }
        if (!codes.contains("en-US")) throw std::runtime_error("Missing English fallback language");
        return result;
    }();
    return languages;
}
bool isSupportedLanguage(const std::string& code) {
    return std::ranges::any_of(availableLanguages(), [&](const Language& language) { return language.code == code; });
}
std::string resolveLanguage(const std::string& culture) {
    auto normalized = QString::fromStdString(culture).replace('_', '-');
    for (const auto& language : availableLanguages())
        if (normalized.compare(QString::fromStdString(language.code), Qt::CaseInsensitive) == 0
            || normalized.compare(QString::fromStdString(language.culture), Qt::CaseInsensitive) == 0) return language.code;
    const auto prefix = normalized.section('-', 0, 0);
    for (const auto& language : availableLanguages())
        if (prefix.compare(QString::fromStdString(language.culture).section('-', 0, 0), Qt::CaseInsensitive) == 0)
            return language.code;
    return "en-US";
}
}
