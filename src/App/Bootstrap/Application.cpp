#include <ProcessConstants.h>
#include <ErrorKeys.h>
#include <Constants/AppConstants.h>
#include "Application.h"
#include <Errors.h>
#include "Themes/ThemeService.h"
#include "Windows/WindowCoordinator.h"
#include "State/WorkspaceState.h"
#include "State/PreferencesStore.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <RetroMediaLab/Translations/LanguageCatalog.h>
#include <QApplication>
#include <QDebug>
#include <QLocale>
#include <QFileInfo>
#include <QStandardPaths>
#include <exception>
#include <filesystem>
#include <stdexcept>
namespace RetroMediaLab {
namespace {
class GuardedApplication final : public QApplication {
public:
    using QApplication::QApplication;
    bool notify(QObject* receiver, QEvent* event) override {
        try { return QApplication::notify(receiver, event); }
        catch (const std::exception& error) {
            // Copy the error while theme code is still loaded; do not retain a plugin exception object.
            failure = std::make_exception_ptr(std::runtime_error(error.what()));
            exit(ProcessConstants::Failure);
            return false;
        }
        catch (...) {
            failure = std::make_exception_ptr(std::runtime_error(ErrorKeys::UnexpectedEventHandlerException));
            exit(ProcessConstants::Failure);
            return false;
        }
    }
    std::exception_ptr failure;
};
}
int Application::run(int argc, char** argv) {
    auto& errorLanguage = Errors::presentationLanguage;
    const auto systemCulture = QLocale::system().uiLanguages().value(0, QLocale::system().name());
    errorLanguage = Errors::resolveLanguage(systemCulture.toStdString());
    try {
    const auto executableDirectory = QFileInfo(QString::fromUtf8(argv[0])).absolutePath();
    QCoreApplication::setLibraryPaths({executableDirectory + AppConstants::LibQt6Plugins});
    // Keep executable theme code alive until QApplication has destroyed its style.
    auto themeModule = ThemeService::loadModule(executableDirectory + AppConstants::LibThemesRetroMediaLab);
    GuardedApplication app(argc, argv);
    app.setApplicationName(AppConstants::RetroMediaLab);
    app.setOrganizationName(AppConstants::RetroMediaLab);
    app.setQuitOnLastWindowClosed(false);
    const auto directory = QApplication::applicationDirPath();
    const auto preferencesPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + AppConstants::PreferencesJson;
    const PreferencesStore preferences(preferencesPath);
    auto selected = preferences.loadOrCreate({QString::fromStdString(Translations::resolveLanguage(systemCulture.toStdString()))});
    errorLanguage = selected.language.toStdString();
    Translations::TranslationService translations(std::filesystem::path((directory + AppConstants::LibLanguages).toStdU16String()));
    translations.setLanguage(errorLanguage);
    ThemeService themes(themeModule);
    themes.applySettings(selected.themeSettings.value(themes.id()).toObject());
    WorkspaceState state;
    state.setDefaultImageDirectory(selected.defaultImageDirectory);
    const auto savePreferences = [&] {
        selected.language = QString::fromStdString(translations.language());
        errorLanguage = translations.language();
        selected.defaultImageDirectory = state.defaultImageDirectory();
        selected.themeSettings.insert(themes.id(), themes.settings());
        preferences.save(selected);
    };
    const auto languagePreference = translations.subscribe(savePreferences);
    const auto themePreference = themes.subscribe(savePreferences);
    const auto directoryPreference = state.subscribeToDefaultImageDirectory(savePreferences);
    app.setStyle(themes.createStyle()); // QApplication owns the style; themeModule outlives it.
    WindowCoordinator windows([&] {
        return themes.createPresentation(translations, state);
    });
    windows.rebuild();
    const int result = app.exec();
    if (app.failure) std::rethrow_exception(app.failure);
    return result;
    } catch (const std::exception& error) {
        throw std::runtime_error(Errors::render(errorLanguage, error.what()));
    } catch (...) {
        throw std::runtime_error(Errors::render(errorLanguage, ErrorKeys::UnexpectedApplicationException));
    }
}
}
