#include <ProcessConstants.h>
#include <ErrorKeys.h>
#include <Constants/AppConstants.h>
#include "Application.h"
#include <Errors.h>
#include "Shared/Presentation/ThemeService.h"
#include "Shared/Emulation/EmulationWorkspace.h"
#include "Windows/WindowCoordinator.h"
#include "State/WorkspaceState.h"
#include "State/PreferencesStore.h"
#include "Logging/LogPaths.h"
#include "Logging/DiagnosticLog.h"
#include "Hardware/HardwareAvailability.h"
#include <RetroMediaLab/Platform/Gamepads.h>
#include <RetroMediaLab/Translations/TranslationService.h>
#include <RetroMediaLab/Translations/LanguageCatalog.h>
#include <QApplication>
#include <QDebug>
#include <QLocale>
#include <QFileInfo>
#include <QMessageBox>
#include <QAbstractButton>
#include <QStandardPaths>
#include <exception>
#include <filesystem>
#include <stdexcept>
namespace RetroMediaLab {
namespace {
constexpr auto GameInputProduct = "Emulation.GameInput.Product";
constexpr auto GameInputInstallPrompt = "Emulation.GameInput.InstallPrompt";
constexpr auto GameInputInstallFailed = "Emulation.GameInput.InstallFailed";
constexpr auto Install = "Emulation.Core.Install";
constexpr auto No = "Common.No";
constexpr auto Close = "Preferences.Close";

void offerGameInputInstaller(const Translations::TranslationService& translations) {
    if (!Platform::canOfferGameInputInstaller()) return;
    const auto text = [&](const char* key) { return QString::fromStdString(translations.text(key)); };
    const auto title = text(GameInputProduct);
    QMessageBox question(QMessageBox::Question, title, text(GameInputInstallPrompt), QMessageBox::Yes | QMessageBox::No);
    question.setTextFormat(Qt::PlainText);
    question.button(QMessageBox::Yes)->setText(text(Install));
    question.button(QMessageBox::No)->setText(text(No));
    question.setDefaultButton(QMessageBox::No);
    if (question.exec() == QMessageBox::Yes &&
        Platform::launchGameInputInstaller() == Platform::GameInputInstallResult::Failed) {
        QMessageBox error(QMessageBox::Warning, title, text(GameInputInstallFailed), QMessageBox::Ok);
        error.setTextFormat(Qt::PlainText);
        error.button(QMessageBox::Ok)->setText(text(Close));
        error.exec();
    }
}

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
    QStringList arguments;
    for (int index = 1; index < argc; ++index) arguments.push_back(QString::fromUtf8(argv[index]));
    // Keep executable theme code alive until QApplication has destroyed its style.
    ThemeModuleHandle themeModule;
    GuardedApplication app(argc, argv);
    app.setApplicationName(AppConstants::RetroMediaLab);
    app.setOrganizationName(AppConstants::RetroMediaLab);
    app.setQuitOnLastWindowClosed(false);
    WorkspaceState state;
    ensureEmulationHost(state);
    if (const auto result = state.emulation->tryHandleHostCommand(arguments)) return *result;

    themeModule = ThemeService::loadModule(executableDirectory + AppConstants::LibThemesRetroMediaLab);
    const auto directory = QApplication::applicationDirPath();
    const auto preferencesPath = PreferencesStore::defaultPath();
    const PreferencesStore preferences(preferencesPath);
    auto selected = preferences.loadOrCreate({QString::fromStdString(Translations::resolveLanguage(systemCulture.toStdString()))});
    errorLanguage = selected.language.toStdString();
    Translations::TranslationService translations(std::filesystem::path((directory + AppConstants::LibLanguages).toStdU16String()));
    translations.setLanguage(errorLanguage);
    ThemeService themes(themeModule);
    themes.applySettings(selected.themeSettings.value(themes.id()).toObject());
    state.profiles->reset(selected.profiles);
    state.logsDirectory = logsDirectory(executableDirectory);
    for (const auto& diagnostic : state.emulation->diagnostics()) {
        const auto severity = diagnostic.issue == Emulation::ModuleIssue::LooseLibrary
            ? DiagnosticSeverity::Warning : DiagnosticSeverity::Error;
        const auto detail = QString::fromStdString(translations.text(EmulationModuleIssueKey)).arg(diagnostic.directory);
        writeDiagnostic(state.logsDirectory, severity, detail, LogConstants::ApplicationContext,
            QString::fromStdString(errorLanguage), QCoreApplication::applicationFilePath());
    }
    state.setLogging(selected.logging);
    state.setConversion(selected.conversion);
    state.setHardware(selected.hardware);
    state.setEngines(selected.engines);
    state.setInterfaceScale(selected.interfaceScale);
    state.setDefaultImageDirectory(selected.defaultImageDirectory);
    state.setEmulationFolders(selected.emulationFolders);
    state.setEmulationShortcuts(selected.emulationShortcuts);
    state.setShowFullscreenEmulationStatistics(selected.showFullscreenEmulationStatistics);
    state.setFullscreenOsdTransparency(selected.fullscreenOsdTransparency);
    state.restoreFileDialogDirectories(selected.fileDialogDirectories.value_or(QJsonObject{}));
    const auto savePreferences = [&] {
        selected.language = QString::fromStdString(translations.language());
        errorLanguage = translations.language();
        selected.defaultImageDirectory = state.defaultImageDirectory();
        selected.fileDialogDirectories = state.fileDialogDirectories();
        selected.emulationFolders = state.emulationFolders();
        selected.emulationShortcuts = state.emulationShortcuts();
        selected.showFullscreenEmulationStatistics = state.showFullscreenEmulationStatistics();
        selected.fullscreenOsdTransparency = state.fullscreenOsdTransparency();
        selected.logging = state.logging();
        selected.conversion = state.conversion();
        selected.hardware = state.hardware();
        selected.engines = state.engines();
        selected.profiles = state.profiles->toJson();
        selected.interfaceScale = state.interfaceScale();
        selected.themeSettings.insert(themes.id(), themes.settings());
        preferences.save(selected);
    };
    const auto languagePreference = translations.subscribe(savePreferences);
    const auto themePreference = themes.subscribe(savePreferences);
    const auto directoryPreference = state.subscribeToDefaultImageDirectory(savePreferences);
    const auto fileDialogPreference = state.subscribeToFileDialogDirectories(savePreferences);
    const auto emulationFolderPreference = state.subscribeToEmulationFolders(savePreferences);
    const auto emulationShortcutPreference = state.subscribeToEmulationShortcuts(savePreferences);
    const auto emulationStatisticsPreference = state.subscribeToEmulationStatistics(savePreferences);
    const auto osdTransparencyPreference = state.subscribeToOsdTransparency(savePreferences);
    const auto hardwarePreference = state.subscribeToHardware(savePreferences);
    const auto displayPreference = state.subscribeToInterfaceScale(savePreferences);
    const auto enginePreference = state.subscribeToEngines(savePreferences);
    const auto conversionPreference = state.subscribeToConversion(savePreferences);
    const auto loggingPreference = state.subscribeToLogging(savePreferences);
    const auto profilesPreference = state.profiles->subscribe(savePreferences);
    app.setStyle(themes.createStyle()); // QApplication owns the style; themeModule outlives it.
    offerGameInputInstaller(translations);
    WindowCoordinator windows([&] {
        return themes.createPresentation(translations, state);
    });
    windows.rebuild();
    HardwareAvailability availability;
    availability.refresh(state.hardware(),state.logsDirectory,state.logging(),[&](HardwareAvailabilityResult result){
        auto current=state.hardware();result.applyTo(current);state.setHardware(std::move(current));
        for(const auto& key:result.errors){
            auto message=QString::fromStdString(translations.text(key.toStdString()));
            message.replace(HardwareConstants::Argument,HardwareConstants::Greaseweazle);
            writeDiagnostic(state.logsDirectory,DiagnosticSeverity::Error,message,HardwareConstants::DiagnosticContext,QString::fromStdString(translations.language()),HardwareConstants::DiagnosticProcess);
        }
    });
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
