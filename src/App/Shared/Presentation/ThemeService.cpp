#include <ErrorKeys.h>
#include <Constants/AppConstants.h>
#include "Shared/Presentation/ThemeService.h"
#include "Shared/Presentation/IThemePresentation.h"
#include <QApplication>
#include <QFont>
#include <QLibrary>
#include <QPalette>
#include <QStyle>
#include <algorithm>
#include <stdexcept>
namespace RetroMediaLab {
struct ThemeModule {
    using Create = IThemePresentation* (*)(ThemeService*, Translations::TranslationService*, WorkspaceState*, char*, std::size_t) noexcept;
    using Destroy = void (*)(IThemePresentation*) noexcept;
    using Style = QStyle* (*)(const std::shared_ptr<const ThemeData>*, char*, std::size_t) noexcept;
    using Operation = bool (*)(IThemePresentation*, char*, std::size_t) noexcept;
    using CreateConfiguration = void* (*)(ThemeService*, char*, std::size_t) noexcept;
    using DestroyConfiguration = void (*)(void*) noexcept;
    using Configure = bool (*)(void*, const QJsonObject*, char*, std::size_t) noexcept;
    explicit ThemeModule(const QString& path) : library(path) {}
    ~ThemeModule() { if (library.isLoaded()) library.unload(); }
    QLibrary library;
    QString id;
    Create create = nullptr;
    Destroy destroy = nullptr;
    Style style = nullptr;
    Operation show = nullptr, synchronize = nullptr;
    CreateConfiguration createConfiguration = nullptr;
    DestroyConfiguration destroyConfiguration = nullptr;
    Configure configure = nullptr;
};
struct ThemeService::Configuration {
    void* instance = nullptr;
    ThemeModule::DestroyConfiguration destroy = nullptr;
    ~Configuration() { if (instance) destroy(instance); }
};
namespace {
template<class Function> Function symbol(QLibrary& library, const char* name) {
    const auto found = library.resolve(name);
    if (!found) throw std::runtime_error(std::string(ErrorKeys::MissingThemeEntryPointWithContext) + name);
    return reinterpret_cast<Function>(found);
}
class LoadedThemePresentation final : public IThemePresentation {
public:
    using OwnedView = std::unique_ptr<IThemePresentation, ThemeModule::Destroy>;
    LoadedThemePresentation(ThemeModuleHandle module, OwnedView view) : module_(std::move(module)), view_(std::move(view)) {}
    void synchronizeState() override { invoke(module_->synchronize); }
    void show() override { invoke(module_->show); }
private:
    void invoke(ThemeModule::Operation operation) {
        char error[AppConstants::Service::ErrorBufferCapacity] = {};
        if (!operation(view_.get(), error, sizeof(error))) throw std::runtime_error(error[0] ? error : ErrorKeys::ThemeOperationFailed);
    }
    ThemeModuleHandle module_;
    OwnedView view_;
};
}
QColor ThemeData::color(const QString& key) const {
    const auto found = colors.constFind(key);
    if (found == colors.cend()) throw std::runtime_error(ErrorKeys::MissingThemeColorWithContext + key.toStdString());
    return found.value();
}
ThemeModuleHandle ThemeService::loadModule(const QString& path) {
    auto module = std::make_shared<ThemeModule>(path);
    if (!module->library.load()) throw std::runtime_error(ErrorKeys::CannotLoadThemeWithContext + path.toStdString());
    const auto version = symbol<unsigned (*)() noexcept>(module->library, AppConstants::RetroMediaLabThemeApiVersion);
    if (version() != AppConstants::Service::ApiVersion) throw std::runtime_error(ErrorKeys::IncompatibleThemeAPI);
    const auto identifier = symbol<const char* (*)() noexcept>(module->library, AppConstants::RetroMediaLabThemeIdentifier);
    module->id = QString::fromUtf8(identifier());
    if (module->id.isEmpty()) throw std::runtime_error(ErrorKeys::MissingThemeIdentifier);
    module->create = symbol<ThemeModule::Create>(module->library, AppConstants::RetroMediaLabCreateTheme);
    module->destroy = symbol<ThemeModule::Destroy>(module->library, AppConstants::RetroMediaLabDestroyTheme);
    module->style = symbol<ThemeModule::Style>(module->library, AppConstants::RetroMediaLabCreateThemeStyle);
    module->show = symbol<ThemeModule::Operation>(module->library, AppConstants::RetroMediaLabShowTheme);
    module->synchronize = symbol<ThemeModule::Operation>(module->library, AppConstants::RetroMediaLabSynchronizeTheme);
    module->createConfiguration = symbol<ThemeModule::CreateConfiguration>(module->library, AppConstants::RetroMediaLabCreateThemeConfiguration);
    module->destroyConfiguration = symbol<ThemeModule::DestroyConfiguration>(module->library, AppConstants::RetroMediaLabDestroyThemeConfiguration);
    module->configure = symbol<ThemeModule::Configure>(module->library, AppConstants::RetroMediaLabConfigureTheme);
    return module;
}
ThemeService::ThemeService(ThemeModuleHandle module)
    : ThemeService(std::move(module), false, QApplication::palette()) {}
ThemeService::ThemeService(ThemeModuleHandle module, bool preview, const QPalette& systemPalette)
    : module_(std::move(module)), data_(std::make_shared<ThemeData>()), preview_(preview),
      systemPalette_(systemPalette), palette_(systemPalette), configuration_(std::make_unique<Configuration>()) {
    if (!module_) throw std::invalid_argument(ErrorKeys::MissingThemeModule);
    id_ = module_->id;
    char error[AppConstants::Service::ErrorBufferCapacity] = {};
    configuration_->destroy = module_->destroyConfiguration;
    configuration_->instance = module_->createConfiguration(this, error, sizeof(error));
    if (!configuration_->instance) throw std::runtime_error(error[0] ? error : ErrorKeys::ThemeConfigurationCreationFailed);
    applySettings({});
}
ThemeService::~ThemeService() = default;
std::unique_ptr<IThemePresentation> ThemeService::createPresentation(Translations::TranslationService& translations, WorkspaceState& state) {
    char error[AppConstants::Service::ErrorBufferCapacity] = {};
    LoadedThemePresentation::OwnedView view(module_->create(this, &translations, &state, error, sizeof(error)), module_->destroy);
    if (!view) throw std::runtime_error(error[0] ? error : ErrorKeys::ThemeReturnedNoPresentation);
    return std::make_unique<LoadedThemePresentation>(module_, std::move(view));
}
QStyle* ThemeService::createStyle() const {
    char error[AppConstants::Service::ErrorBufferCapacity] = {};
    const std::shared_ptr<const ThemeData> data = data_;
    auto* style = module_->style(&data, error, sizeof(error));
    if (!style) throw std::runtime_error(error[0] ? error : ErrorKeys::ThemeReturnedNoStyle);
    return style;
}
void ThemeService::applySettings(const QJsonObject& values) {
    char error[AppConstants::Service::ErrorBufferCapacity] = {};
    if (!module_->configure(configuration_->instance, &values, error, sizeof(error)))
        throw std::runtime_error(error[0] ? error : ErrorKeys::ThemeConfigurationFailed);
}
void ThemeService::publish(const ThemeData& data, const QJsonObject& values, const QPalette* palette) {
    const bool changed = *data_ != data || settings_ != values;
    *data_ = data;
    settings_ = values;
    palette_ = palette ? *palette : systemPalette_;
    if (!preview_) {
        if (palette) QApplication::setPalette(*palette);
        auto font = QApplication::font();
        if (!data.fontFamily.isEmpty()) font.setFamily(data.fontFamily);
        font.setPixelSize(data.fontSize);
        QApplication::setFont(font);
    }
    if (!changed) return;
    const auto observers = observers_;
    std::erase_if(observers_, [](const auto& observer) { return observer.expired(); });
    for (const auto& observer : observers) if (const auto live = observer.lock()) (*live)();
}
std::unique_ptr<ThemeService> ThemeService::createPreview(const QJsonObject& values) const {
    auto result = std::unique_ptr<ThemeService>(new ThemeService(module_, true, systemPalette_));
    result->applySettings(values);
    return result;
}
ThemeService::Subscription ThemeService::subscribe(std::function<void()> observer) {
    auto subscription = std::make_shared<const std::function<void()>>(std::move(observer));
    observers_.push_back(subscription);
    return subscription;
}
}
