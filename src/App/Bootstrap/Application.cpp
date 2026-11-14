#include "Application.h"
#include "Themes/ThemeService.h"
#include "Windows/WindowCoordinator.h"
#include "State/WorkspaceState.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QApplication>
#include <QDebug>
#include <QLocale>
#include <QFileInfo>
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
            exit(1);
            return false;
        }
        catch (...) {
            failure = std::make_exception_ptr(std::runtime_error("Unexpected event handler exception"));
            exit(1);
            return false;
        }
    }
    std::exception_ptr failure;
};
}
int Application::run(int argc, char** argv) {
    const auto executableDirectory = QFileInfo(QString::fromUtf8(argv[0])).absolutePath();
    QCoreApplication::setLibraryPaths({executableDirectory + "/lib/Qt6/plugins"});
    // Keep executable theme code alive until QApplication has destroyed its style.
    auto themeModule = ThemeService::loadModule(executableDirectory + "/lib/Themes/RetroMediaLab");
    GuardedApplication app(argc, argv);
    app.setApplicationName("RetroMediaLab");
    app.setOrganizationName("RetroMediaLab");
    app.setQuitOnLastWindowClosed(false);
    const auto directory = QApplication::applicationDirPath();
    Translations::TranslationService translations(std::filesystem::path((directory + "/lib/Languages").toStdU16String()));
    if (QLocale::system().name().startsWith("fr")) translations.setLanguage("fr-FR");
    ThemeService themes(themeModule);
    app.setStyle(themes.createStyle()); // QApplication owns the style; themeModule outlives it.
    WorkspaceState state;
    WindowCoordinator windows([&] {
        return themes.createPresentation(translations, state);
    });
    windows.rebuild();
    const int result = app.exec();
    if (app.failure) std::rethrow_exception(app.failure);
    return result;
}
}
