#include <ErrorKeys.h>
#include <RetroMediaLab/Translations/TranslationConstants.h>
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
    if (!library.load()) throw std::runtime_error(ErrorKeys::CannotLoadTranslationCatalogWithContext + qtPath(path).toStdString());
    struct Unload { QLibrary& library; ~Unload() { library.unload(); } } unload{library};
    using Catalog = const unsigned char* (*)(std::size_t*);
    const auto catalog = reinterpret_cast<Catalog>(library.resolve(TranslationConstants::RetroMediaLabCatalog));
    if (!catalog) throw std::runtime_error(ErrorKeys::TranslationCatalogEntryPointMissing);
    std::size_t size = 0;
    const auto* bytes = catalog(&size);
    if (!bytes || size == 0 || size > TranslationConstants::TranslationServiceValues::MaximumCatalogMegabytes * TranslationConstants::TranslationServiceValues::BytesPerKibibyte * TranslationConstants::TranslationServiceValues::BytesPerKibibyte) throw std::runtime_error(ErrorKeys::InvalidTranslationCatalogSize);
    // Copy before unloading. No pointer into the resource DLL escapes this function.
    const QByteArray payload(reinterpret_cast<const char*>(bytes), static_cast<qsizetype>(size));
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(payload, &error);
    if (error.error != QJsonParseError::NoError || !document.isObject())
        throw std::runtime_error(ErrorKeys::InvalidCompiledTranslationCatalog);
    const auto domains = document.object().value(TranslationConstants::Catalogs).toObject();
    if (!domains.value(QString::fromStdString(domain)).isObject()) throw std::runtime_error(ErrorKeys::TranslationDomainNotFoundWithContext + domain);
    const auto object = domains.value(QString::fromStdString(domain)).toObject();
    if (!object.value(TranslationConstants::Strings).isObject()) throw std::runtime_error(ErrorKeys::MissingTranslationStrings);
    const auto values = object.value(TranslationConstants::Strings).toObject();
    for (auto it = values.begin(); it != values.end(); ++it) {
        if (!it.value().isString() || !strings.emplace(it.key().toStdString(), it.value().toString().toStdString()).second)
            throw std::runtime_error(ErrorKeys::InvalidOrDuplicateTranslationKeyWithContext + it.key().toStdString());
    }
    if (invariants) {
        for (const auto& key : object.value(TranslationConstants::Invariants).toArray()) {
            if (!key.isString()) throw std::runtime_error(ErrorKeys::InvalidInvariantKey);
            invariants->insert(key.toString().toStdString());
        }
    }
}
void validateSegment(const std::string& segment) {
    if (!QRegularExpression(TranslationConstants::SafeSegmentPattern).match(QString::fromStdString(segment)).hasMatch())
        throw std::invalid_argument(ErrorKeys::InvalidTranslationDomainOrLanguage);
}
}
struct TranslationService::Implementation {
    std::filesystem::path root;
    std::string domain;
    std::string language;
    Strings constants;
    Strings localized;
    std::map<std::string, std::unique_ptr<TranslationService>> domains;
    std::set<std::string> invariants;
    std::vector<std::weak_ptr<const std::function<void()>>> observers;
};
TranslationService::TranslationService(std::filesystem::path catalogs, std::string domain)
    : implementation_(std::make_unique<Implementation>()) {
    validateSegment(domain);
    auto& data = *implementation_;
    data.root = std::move(catalogs);
    data.domain = std::move(domain);
    load(data.root / TranslationConstants::Constants, data.domain, data.constants, &data.invariants);
    for (const auto& key : data.invariants)
        if (!data.constants.contains(key)) throw std::runtime_error(ErrorKeys::InvariantHasNoConstant);
    setLanguage(TranslationConstants::EnUS);
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
    data.domains.clear();
    // A snapshot allows observers to detach/attach during notification.
    auto observers = data.observers;
    std::erase_if(data.observers, [](const auto& item) { return item.expired(); });
    for (const auto& observer : observers) if (const auto live = observer.lock()) (*live)();
}
std::string TranslationService::language() const { return implementation_->language; }
std::string TranslationService::text(const std::string& key, const std::string& domain) const {
    auto& data = *implementation_;
    if (domain.empty() || domain == data.domain) return text(key);
    validateSegment(domain);
    auto found = data.domains.find(domain);
    if (found == data.domains.end()) {
        auto service = std::make_unique<TranslationService>(data.root, domain);
        service->setLanguage(data.language);
        found = data.domains.emplace(domain, std::move(service)).first;
    }
    return found->second->text(key);
}
std::string TranslationService::text(const std::string& key) const {
    const auto& data = *implementation_;
    if (!data.invariants.contains(key))
        if (const auto value = data.localized.find(key); value != data.localized.end()) return value->second;
    if (const auto value = data.constants.find(key); value != data.constants.end()) return value->second;
    return TranslationConstants::MissingKeyOpen + key + TranslationConstants::MissingKeyClose;
}
TranslationService::Subscription TranslationService::subscribe(std::function<void()> observer) {
    std::erase_if(implementation_->observers, [](const auto& item) { return item.expired(); });
    auto subscription = std::make_shared<const std::function<void()>>(std::move(observer));
    implementation_->observers.push_back(subscription);
    return subscription;
}
}
