#include <RetroMediaLab/Translations/TranslationService.h>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLibrary>
#include <QRegularExpression>
#include <algorithm>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>
namespace RetroMediaLab::Translations {
namespace {
using Strings = std::map<std::string, std::string>;
QString qtPath(const std::filesystem::path& path) { return QString::fromStdU16String(path.u16string()); }
void load(const std::filesystem::path& path, const std::string& domain, Strings& strings, std::set<std::string>* invariants = nullptr) {
    QLibrary library(qtPath(path));
    if (!library.load()) throw std::runtime_error("Cannot load translation catalog: " + path.string());
    struct Unload { QLibrary& library; ~Unload() { library.unload(); } } unload{library};
    using Catalog = const unsigned char* (*)(std::size_t*);
    const auto catalog = reinterpret_cast<Catalog>(library.resolve("RetroMediaLabCatalog"));
    if (!catalog) throw std::runtime_error("Translation catalog entry point missing");
    std::size_t size = 0;
    const auto* bytes = catalog(&size);
    if (!bytes || size == 0 || size > 16 * 1024 * 1024) throw std::runtime_error("Invalid translation catalog size");
    // Copy before unloading. No pointer into the resource DLL escapes this function.
    const QByteArray payload(reinterpret_cast<const char*>(bytes), static_cast<qsizetype>(size));
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(payload, &error);
    if (error.error != QJsonParseError::NoError || !document.isObject())
        throw std::runtime_error("Invalid compiled translation catalog");
    const auto domains = document.object().value("catalogs").toObject();
    if (!domains.value(QString::fromStdString(domain)).isObject()) throw std::runtime_error("Translation domain not found: " + domain);
    const auto object = domains.value(QString::fromStdString(domain)).toObject();
    if (!object.value("strings").isObject()) throw std::runtime_error("Missing translation strings");
    const auto values = object.value("strings").toObject();
    for (auto it = values.begin(); it != values.end(); ++it) {
        if (!it.value().isString() || !strings.emplace(it.key().toStdString(), it.value().toString().toStdString()).second)
            throw std::runtime_error("Invalid or duplicate translation key: " + it.key().toStdString());
    }
    if (invariants) {
        for (const auto& key : object.value("invariants").toArray()) {
            if (!key.isString()) throw std::runtime_error("Invalid invariant key");
            invariants->insert(key.toString().toStdString());
        }
    }
}
void validateSegment(const std::string& segment) {
    if (!QRegularExpression("^[A-Za-z0-9_-]+$").match(QString::fromStdString(segment)).hasMatch())
        throw std::invalid_argument("Invalid translation domain or language");
}
}
struct TranslationService::Implementation {
    std::filesystem::path root;
    std::string domain;
    std::string language;
    Strings constants;
    Strings localized;
    std::set<std::string> invariants;
    std::vector<std::weak_ptr<const std::function<void()>>> observers;
};
TranslationService::TranslationService(std::filesystem::path catalogs, std::string domain)
    : implementation_(std::make_unique<Implementation>()) {
    validateSegment(domain);
    auto& data = *implementation_;
    data.root = std::move(catalogs);
    data.domain = std::move(domain);
    load(data.root / "constants", data.domain, data.constants, &data.invariants);
    for (const auto& key : data.invariants)
        if (!data.constants.contains(key)) throw std::runtime_error("Invariant has no constant");
    setLanguage("en-US");
}
TranslationService::~TranslationService() = default;
void TranslationService::setLanguage(const std::string& language) {
    validateSegment(language);
    auto& data = *implementation_;
    if (data.language == language) return;
    Strings candidate;
    load(data.root / language, data.domain, candidate);
    data.localized.swap(candidate);
    data.language = language;
    // A snapshot allows observers to detach/attach during notification.
    auto observers = data.observers;
    std::erase_if(data.observers, [](const auto& item) { return item.expired(); });
    for (const auto& observer : observers) if (const auto live = observer.lock()) (*live)();
}
std::string TranslationService::language() const { return implementation_->language; }
std::string TranslationService::text(const std::string& key) const {
    const auto& data = *implementation_;
    if (!data.invariants.contains(key))
        if (const auto value = data.localized.find(key); value != data.localized.end()) return value->second;
    if (const auto value = data.constants.find(key); value != data.constants.end()) return value->second;
    return "[" + key + "]";
}
TranslationService::Subscription TranslationService::subscribe(std::function<void()> observer) {
    auto subscription = std::make_shared<const std::function<void()>>(std::move(observer));
    implementation_->observers.push_back(subscription);
    return subscription;
}
}
