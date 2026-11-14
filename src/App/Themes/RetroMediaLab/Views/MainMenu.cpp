#include "MainMenu.h"
#include "Shared/Controls/LocalizedAction.h"
#include <QMenu>

namespace RetroMediaLab {
MainMenu::MainMenu(Translations::TranslationService& translations, std::function<void()> openPreferences, QWidget* parent)
    : QMenuBar(parent) {
    setObjectName("mainMenu");
    setNativeMenuBar(false);
    setContentsMargins(3, 2, 3, 2);

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

    auto* options = menu("Menu.Options");
    auto* preferences = action(options, "Menu.Preferences");
    preferences->setEnabled(true);
    connect(preferences, &QAction::triggered, this, [openPreferences = std::move(openPreferences)] { openPreferences(); });
    for (const auto* key : {"Updates.Title", "Logs.History"})
        action(options, key);
    options->addSeparator();
    auto* diagnostics = menu("Menu.Diagnostics", options);
    for (const auto* key : {"Menu.Information", "Menu.Bandwidth", "Menu.Rpm", "Menu.Seek", "Menu.Align"})
        action(diagnostics, key);
    auto* hardware = menu("Menu.Hardware", options);
    for (const auto* key : {"Menu.Pins", "Menu.Reset", "Menu.Delays"})
        action(hardware, key);
    hardware->addSeparator();
    action(hardware, "Menu.Firmware");
    action(menu("Menu.Emulation"), "Menu.EmulationPreferences");
    auto* help = menu("Menu.Help");
    action(help, "Menu.Documentation");
    action(help, "Menu.About");
}
}

