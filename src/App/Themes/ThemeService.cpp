#include "ThemeService.h"
#include "IThemePresentation.h"
#include <QApplication>
#include <QFile>
#include <QFont>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPalette>
#include <QLibrary>
#include <QStyle>
#include <algorithm>
#include <stdexcept>
namespace RetroMediaLab {
struct ThemeModule {
    using Create = IThemePresentation* (*)(ThemeService*, Translations::TranslationService*, WorkspaceState*, char*, std::size_t) noexcept;
    using Destroy = void (*)(IThemePresentation*) noexcept;
    using Style = QStyle* (*)(const std::shared_ptr<const ThemeData>*, char*, std::size_t) noexcept;
    using Operation = bool (*)(IThemePresentation*, char*, std::size_t) noexcept;
    explicit ThemeModule(const QString& path) : library(path) {}
    ~ThemeModule() { if (library.isLoaded()) library.unload(); }
    QLibrary library;
    QString prefix;
    Create create = nullptr;
    Destroy destroy = nullptr;
    Style style = nullptr;
    Operation show = nullptr;
    Operation synchronize = nullptr;
};
namespace {
template<class Function> Function symbol(QLibrary& library, const char* name) {
    const auto found = library.resolve(name);
    if (!found) throw std::runtime_error(std::string("Missing theme entry point: ") + name);
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
        char error[1024] = {};
        if (!operation(view_.get(), error, sizeof(error))) throw std::runtime_error(error[0] ? error : "Theme operation failed");
    }
    ThemeModuleHandle module_; // Declared first: released after the concrete view is destroyed.
    OwnedView view_;
};
QJsonObject readObject(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) throw std::runtime_error("Cannot read theme: " + path.toStdString());
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !document.isObject()) throw std::runtime_error("Invalid theme JSON");
    return document.object();
}
}
QColor ThemeData::color(const QString& key) const {
    const auto found = colors.constFind(key);
    if (found == colors.cend()) throw std::runtime_error("Missing theme color: " + key.toStdString());
    return found.value();
}
ThemeModuleHandle ThemeService::loadModule(const QString& path) {
    auto module = std::make_shared<ThemeModule>(path);
    if (!module->library.load()) throw std::runtime_error("Cannot load theme: " + path.toStdString());
    const auto version = symbol<unsigned (*)() noexcept>(module->library, "RetroMediaLabThemeApiVersion");
    if (version() != 1) throw std::runtime_error("Incompatible theme API");
    const auto prefix = symbol<const char* (*)() noexcept>(module->library, "RetroMediaLabThemeResourcePrefix");
    module->prefix = QString::fromUtf8(prefix());
    if (!module->prefix.startsWith(":/Themes/")) throw std::runtime_error("Invalid theme resource prefix");
    module->create = symbol<ThemeModule::Create>(module->library, "RetroMediaLabCreateTheme");
    module->destroy = symbol<ThemeModule::Destroy>(module->library, "RetroMediaLabDestroyTheme");
    module->style = symbol<ThemeModule::Style>(module->library, "RetroMediaLabCreateThemeStyle");
    module->show = symbol<ThemeModule::Operation>(module->library, "RetroMediaLabShowTheme");
    module->synchronize = symbol<ThemeModule::Operation>(module->library, "RetroMediaLabSynchronizeTheme");
    return module;
}
ThemeService::ThemeService(ThemeModuleHandle module) : module_(std::move(module)), data_(std::make_shared<ThemeData>()) {
    if (!module_) throw std::invalid_argument("Missing theme module");
    directory_ = module_->prefix;
    const auto metrics = readObject(directory_ + "/metrics.json");
    auto metric = [&](const char* key) {
        const auto value = metrics.value(key);
        if (!value.isDouble() || value.toInt() < 1 || value.toInt() > 256) throw std::runtime_error("Invalid theme metric");
        return value.toInt();
    };
    data_->fontSize = metric("fontSize");
    data_->controlHeight = metric("controlHeight");
    data_->cardPadding = metric("cardPadding");
    data_->cardRadius = metric("cardRadius");
    data_->controlRadius = metric("controlRadius");
    setVariant(ThemeVariant::Light);
}
ThemeVariant ThemeService::variant() const noexcept { return data_->dark ? ThemeVariant::Dark : ThemeVariant::Light; }
std::unique_ptr<IThemePresentation> ThemeService::createPresentation(Translations::TranslationService& translations, WorkspaceState& state) {
    char error[1024] = {};
    LoadedThemePresentation::OwnedView view(module_->create(this, &translations, &state, error, sizeof(error)), module_->destroy);
    if (!view) throw std::runtime_error(error[0] ? error : "Theme returned no presentation");
    return std::make_unique<LoadedThemePresentation>(module_, std::move(view));
}
QStyle* ThemeService::createStyle() const {
    char error[1024] = {};
    const std::shared_ptr<const ThemeData> data = data_;
    auto* style = module_->style(&data, error, sizeof(error));
    if (!style) throw std::runtime_error(error[0] ? error : "Theme returned no style");
    return style;
}
void ThemeService::setVariant(ThemeVariant variant) {
    ThemeData next = *data_;
    next.dark = variant == ThemeVariant::Dark;
    const auto palette = readObject(directory_ + (next.dark ? "/dark.json" : "/light.json"));
    next.colors.clear();
    for (auto it = palette.begin(); it != palette.end(); ++it) {
        const QColor color(it.value().toString());
        if (!it.value().isString() || !color.isValid()) throw std::runtime_error("Invalid theme color");
        next.colors.insert(it.key(), color);
    }
    for (const auto* key : {"WindowBrush", "CardBrush", "ControlBrush", "TextBrush", "BorderBrush", "AccentBrush", "MutedTextBrush", "SelectedBrush", "HoverBrush"})
        (void)next.color(key);
    *data_ = std::move(next);
    QPalette colors;
    colors.setColor(QPalette::Window, data_->color("WindowBrush"));
    colors.setColor(QPalette::WindowText, data_->color("TextBrush"));
    colors.setColor(QPalette::Text, data_->color("TextBrush"));
    colors.setColor(QPalette::ButtonText, data_->color("TextBrush"));
    colors.setColor(QPalette::Base, data_->color("ControlBrush"));
    colors.setColor(QPalette::AlternateBase, data_->color("CardBrush"));
    colors.setColor(QPalette::Button, data_->color("ControlBrush"));
    colors.setColor(QPalette::Highlight, data_->color("SelectedBrush"));
    colors.setColor(QPalette::HighlightedText, data_->color("TextBrush"));
    colors.setColor(QPalette::PlaceholderText, data_->color("MutedTextBrush"));
    colors.setColor(QPalette::ToolTipBase, data_->color("CardBrush"));
    colors.setColor(QPalette::ToolTipText, data_->color("TextBrush"));
    colors.setColor(QPalette::Disabled, QPalette::Text, data_->color("MutedTextBrush"));
    colors.setColor(QPalette::Disabled, QPalette::ButtonText, data_->color("MutedTextBrush"));
    QApplication::setPalette(colors);
    QFont font = QApplication::font();
#ifdef Q_OS_WIN
    font.setFamily("Segoe UI");
#endif
    font.setPixelSize(data_->fontSize);
    QApplication::setFont(font);
    const auto observers = observers_;
    std::erase_if(observers_, [](const auto& observer) { return observer.expired(); });
    for (const auto& observer : observers) if (const auto live = observer.lock()) (*live)();
}
ThemeService::Subscription ThemeService::subscribe(std::function<void()> observer) {
    auto subscription = std::make_shared<const std::function<void()>>(std::move(observer));
    observers_.push_back(subscription);
    return subscription;
}
}
