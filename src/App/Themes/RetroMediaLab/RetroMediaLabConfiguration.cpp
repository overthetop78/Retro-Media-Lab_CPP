#include <ErrorKeys.h>
#include "ThemeConstants.h"
#include "RetroMediaLabConfiguration.h"
#include <QApplication>
#include <QEvent>
#include <QFile>
#include <QJsonDocument>
#include <QPalette>
#include <QScopedValueRollback>
#include <QStyleHints>
#include <QTimer>
#include <stdexcept>
namespace RetroMediaLab {
namespace {
QJsonObject readObject(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) throw std::runtime_error(ErrorKeys::CannotReadRetroMediaLabThemeData);
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !document.isObject()) throw std::runtime_error(ErrorKeys::InvalidThemeJSON);
    return document.object();
}
}
RetroMediaLabConfiguration::RetroMediaLabConfiguration(ThemeService& service) : service_(service) {
    const auto metrics = readObject(ThemeConstants::ThemesRetroMediaLabMetricsJson);
    const auto metric = [&](const char* key) {
        const auto value = metrics.value(key);
        if (!value.isDouble() || value.toInt() < ThemeConstants::Configuration::MinimumMetric || value.toInt() > ThemeConstants::Configuration::MaximumMetric) throw std::runtime_error(ErrorKeys::InvalidThemeMetric);
        return value.toInt();
    };
    metrics_.fontSize = metric(ThemeConstants::FontSize);
    metrics_.controlHeight = metric(ThemeConstants::ControlHeight);
    metrics_.cardPadding = metric(ThemeConstants::CardPadding);
    metrics_.cardRadius = metric(ThemeConstants::CardRadius);
    metrics_.controlRadius = metric(ThemeConstants::ControlRadius);
    metrics_.tabPaddingX = metric(ThemeConstants::TabPaddingX);
    metrics_.tabPaddingY = metric(ThemeConstants::TabPaddingY);
    metrics_.tabRadius = metric(ThemeConstants::TabRadius);
#ifdef Q_OS_WIN
    metrics_.fontFamily = ThemeConstants::SegoeUI;
#endif
    qApp->installEventFilter(this);
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this,
        [this] { scheduleSystemRefresh(); });
}
ThemeData RetroMediaLabConfiguration::paletteData(bool dark) const {
    auto data = metrics_;
    data.dark = dark;
    const auto colors = readObject(dark ? ThemeConstants::ThemesRetroMediaLabDarkJson : ThemeConstants::ThemesRetroMediaLabLightJson);
    for (auto it = colors.begin(); it != colors.end(); ++it) {
        const QColor color(it.value().toString());
        if (!it.value().isString() || !color.isValid()) throw std::runtime_error(ErrorKeys::InvalidThemeColor);
        data.colors.insert(it.key(), color);
    }
    for (const auto* role : {ThemeConstants::WindowBrush, ThemeConstants::CardBrush, ThemeConstants::ControlBrush, ThemeConstants::TextBrush, ThemeConstants::BorderBrush, ThemeConstants::AccentBrush,
        ThemeConstants::MutedTextBrush, ThemeConstants::SelectedBrush, ThemeConstants::HoverBrush, ThemeConstants::StatusBrush, ThemeConstants::TerminalBackground, ThemeConstants::TerminalBorder,
        ThemeConstants::TerminalText, ThemeConstants::TerminalCommand, ThemeConstants::TerminalPrompt, ThemeConstants::CaptionBrush}) (void)data.color(role);
    return data;
}
void RetroMediaLabConfiguration::apply(const QJsonObject& values) {
    const auto appearance = values.value(ThemeConstants::Appearance).toString(ThemeConstants::Light);
    if (appearance != ThemeConstants::System && appearance != ThemeConstants::Light && appearance != ThemeConstants::Dark)
        throw std::invalid_argument(ErrorKeys::InvalidRetroMediaLabAppearance);
    const QScopedValueRollback guard(applying_, true);
    if (appearance == ThemeConstants::System) {
        QApplication::setPalette(QPalette()); // Remove application color overrides.
        settings_ = {{ThemeConstants::Appearance, appearance}};
        refreshSystem();
        return;
    }
    const auto data = paletteData(appearance == ThemeConstants::Dark);
    QPalette palette;
    palette.setColor(QPalette::Window, data.color(ThemeConstants::WindowBrush));
    palette.setColor(QPalette::WindowText, data.color(ThemeConstants::TextBrush));
    palette.setColor(QPalette::Text, data.color(ThemeConstants::TextBrush));
    palette.setColor(QPalette::ButtonText, data.color(ThemeConstants::TextBrush));
    palette.setColor(QPalette::Base, data.color(ThemeConstants::ControlBrush));
    palette.setColor(QPalette::AlternateBase, data.color(ThemeConstants::CardBrush));
    palette.setColor(QPalette::Button, data.color(ThemeConstants::ControlBrush));
    palette.setColor(QPalette::Highlight, data.color(ThemeConstants::SelectedBrush));
    palette.setColor(QPalette::HighlightedText, data.color(ThemeConstants::TextBrush));
    palette.setColor(QPalette::PlaceholderText, data.color(ThemeConstants::MutedTextBrush));
    palette.setColor(QPalette::ToolTipBase, data.color(ThemeConstants::CardBrush));
    palette.setColor(QPalette::ToolTipText, data.color(ThemeConstants::TextBrush));
    palette.setColor(QPalette::Disabled, QPalette::Text, data.color(ThemeConstants::MutedTextBrush));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, data.color(ThemeConstants::MutedTextBrush));
    settings_ = {{ThemeConstants::Appearance, appearance}};
    service_.publish(data, settings_, &palette);
}
void RetroMediaLabConfiguration::refreshSystem() {
    const auto palette = QApplication::palette();
    auto data = paletteData(palette.color(QPalette::Window).lightnessF() < ThemeConstants::Configuration::DarkLightnessThreshold);
    data.nativeSystemColors = true;
    const auto set = [&](const char* role, QPalette::ColorRole source) { data.colors[role] = palette.color(source); };
    set(ThemeConstants::WindowBrush, QPalette::Window);
    set(ThemeConstants::CardBrush, QPalette::Base);
    set(ThemeConstants::ControlBrush, QPalette::Button);
    set(ThemeConstants::TextBrush, QPalette::WindowText);
    set(ThemeConstants::BorderBrush, QPalette::Mid);
    set(ThemeConstants::AccentBrush, QPalette::Accent);
    set(ThemeConstants::HoverBrush, QPalette::AlternateBase);
    set(ThemeConstants::SelectedBrush, QPalette::Highlight);
    set(ThemeConstants::ExplorerSelectionBrush, QPalette::Highlight);
    set(ThemeConstants::StatusBrush, QPalette::Button);
    set(ThemeConstants::TerminalBackground, QPalette::Base);
    set(ThemeConstants::TerminalBorder, QPalette::Mid);
    set(ThemeConstants::TerminalText, QPalette::Text);
    set(ThemeConstants::TerminalCommand, QPalette::Text);
    set(ThemeConstants::TerminalPrompt, QPalette::Accent);
    set(ThemeConstants::CaptionBrush, QPalette::Window);
    data.colors[ThemeConstants::MutedTextBrush] = palette.color(QPalette::Disabled, QPalette::WindowText);
    service_.publish(data, settings_);
}
void RetroMediaLabConfiguration::scheduleSystemRefresh() {
    if (applying_ || scheduled_ || settings_.value(ThemeConstants::Appearance) != ThemeConstants::System) return;
    scheduled_ = true;
    QTimer::singleShot(0, this, [this] {
        scheduled_ = false;
        if (!applying_ && settings_.value(ThemeConstants::Appearance) == ThemeConstants::System) refreshSystem();
    });
}
bool RetroMediaLabConfiguration::eventFilter(QObject*, QEvent* event) {
    if (event->type() == QEvent::ApplicationPaletteChange || event->type() == QEvent::ThemeChange)
        scheduleSystemRefresh();
    return false;
}
}
