#include "../ThemeConstants.h"
#include "MainMenu.h"
#include "Shared/Controls/LocalizedAction.h"
#include <QMenu>

namespace RetroMediaLab {
MainMenu::MainMenu(Translations::TranslationService& translations, std::function<void()> openPreferences, QWidget* parent)
    : QMenuBar(parent) {
    setObjectName(ThemeConstants::MainMenu);
    setNativeMenuBar(false);
    setContentsMargins(ThemeConstants::Menu::HorizontalInset, ThemeConstants::Menu::VerticalInset, ThemeConstants::Menu::HorizontalInset, ThemeConstants::Menu::VerticalInset);

    const auto menu = [&](const char* key, QMenu* owner = nullptr) {
        auto* result = new QMenu(this);
        auto* title = new LocalizedAction(translations, key, result);
        result->setTitle(title->text());
        connect(title, &QAction::changed, result, [result, title] {
            result->setTitle(title->text());
        });
        if (owner) owner->addMenu(result);
        else addMenu(result);
        return result;
    };
    const auto action = [&](QMenu* owner, const char* key) {
        auto* result = new LocalizedAction(translations, key, owner);
        result->setEnabled(false);
        owner->addAction(result);
        return result;
    };

    auto* options = menu(ThemeConstants::MenuOptions);
    auto* preferences = action(options, ThemeConstants::MenuPreferences);
    preferences->setEnabled(true);
    connect(preferences, &QAction::triggered, this, [openPreferences = std::move(openPreferences)] { openPreferences(); });
    for (const auto* key : {ThemeConstants::UpdatesTitle, ThemeConstants::LogsHistory})
        action(options, key);
    options->addSeparator();
    auto* diagnostics = menu(ThemeConstants::MenuDiagnostics, options);
    for (const auto* key : {ThemeConstants::MenuInformation, ThemeConstants::MenuBandwidth, ThemeConstants::MenuRpm, ThemeConstants::MenuSeek, ThemeConstants::MenuAlign})
        action(diagnostics, key);
    auto* hardware = menu(ThemeConstants::MenuHardware, options);
    for (const auto* key : {ThemeConstants::MenuPins, ThemeConstants::MenuReset, ThemeConstants::MenuDelays})
        action(hardware, key);
    hardware->addSeparator();
    action(hardware, ThemeConstants::MenuFirmware);
    action(menu(ThemeConstants::MenuEmulation), ThemeConstants::MenuEmulationPreferences);
    auto* help = menu(ThemeConstants::MenuHelp);
    action(help, ThemeConstants::MenuDocumentation);
    action(help, ThemeConstants::MenuAbout);
}
}

