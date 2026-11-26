#include "../ThemeConstants.h"
#include "MainMenu.h"
#include "Shared/Controls/LocalizedAction.h"
#include "../ThemeIcons.h"
#include "State/WorkspaceState.h"
#include <RetroMediaLab/Emulation/Host.h>
#include <QMenu>
#include <QCollator>
#include <algorithm>

namespace RetroMediaLab {
MainMenu::MainMenu(Translations::TranslationService& translations, std::function<void()> openPreferences, std::function<void()> openGreaseweazle, std::function<void()> openEmulationSettings,
    WorkspaceState& state, std::function<void(const QString&)> openModuleOptions, QWidget* parent)
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
    auto* greaseweazle = action(options, ThemeConstants::GreaseweazleDialog::Title);
    greaseweazle->setObjectName(ThemeConstants::GreaseweazleDialog::Action);
    greaseweazle->setEnabled(true);
    connect(greaseweazle, &QAction::triggered, this, [openGreaseweazle = std::move(openGreaseweazle)] { openGreaseweazle(); });
    auto* emulationMenu = menu(ThemeConstants::MenuEmulation);
    auto* emulation = action(emulationMenu, ThemeConstants::MenuEmulationPreferences);
    emulation->setEnabled(true);
    connect(emulation, &QAction::triggered, this, [open = std::move(openEmulationSettings)] { open(); });
    connect(emulationMenu, &QMenu::aboutToShow, emulationMenu,
        [emulationMenu, emulation, &state, &translations, open = std::move(openModuleOptions)] {
            for (auto* item : emulationMenu->actions()) {
                if (item == emulation) continue;
                emulationMenu->removeAction(item);
                item->deleteLater();
            }
            if (!state.emulation) return;
            QList<std::pair<QString, QString>> entries;
            const auto language = QString::fromStdString(translations.language());
            for (const auto& module : state.emulation->modules()) {
                auto title = state.emulation->translate(module.id, module.displayResourceKey, language);
                if (title.isEmpty() || title == module.displayResourceKey)
                    title = QString::fromStdString(translations.text(module.displayResourceKey.toStdString()));
                if (title.isEmpty() || title == module.displayResourceKey) title = module.id;
                entries.emplaceBack(std::move(title), module.id);
            }
            if (entries.isEmpty()) return;
            QCollator collator;
            collator.setCaseSensitivity(Qt::CaseInsensitive);
            std::stable_sort(entries.begin(), entries.end(), [&](const auto& left, const auto& right) {
                return collator.compare(left.first, right.first) < 0;
            });
            emulationMenu->addSeparator();
            const auto icon = themeIcon(QLatin1String(AppConstants::Emulation), emulationMenu->palette().color(QPalette::WindowText));
            for (const auto& [title, id] : entries) {
                auto* item = emulationMenu->addAction(icon, title);
                QObject::connect(item, &QAction::triggered, emulationMenu, [open, id] { open(id); });
            }
        });
    auto* help = menu(ThemeConstants::MenuHelp);
    action(help, ThemeConstants::MenuDocumentation);
    action(help, ThemeConstants::MenuAbout);
}
}

