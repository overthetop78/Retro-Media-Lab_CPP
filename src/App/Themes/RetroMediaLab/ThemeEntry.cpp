#include <ErrorKeys.h>
#include "ThemeConstants.h"
#include "RetroMediaLabPresentation.h"
#include "RetroMediaLabConfiguration.h"
#include "RetroMediaLabStyle.h"
#include "Shared/Presentation/ThemeService.h"
#include <cstdio>
#include <exception>
#include <stdexcept>
#if defined(_WIN32)
# define THEME_EXPORT __declspec(dllexport)
#else
# define THEME_EXPORT __attribute__((visibility("default")))
#endif
using namespace RetroMediaLab;
namespace {
void report(char* output, std::size_t size, const char* message) noexcept {
    if (output && size) std::snprintf(output, size, ThemeConstants::ErrorBufferFormat, message);
}
}
extern "C" THEME_EXPORT unsigned RetroMediaLabThemeApiVersion() noexcept { return ThemeConstants::Entry::ApiVersion; }
extern "C" THEME_EXPORT const char* RetroMediaLabThemeResourcePrefix() noexcept { return ThemeConstants::ThemesRetroMediaLab; }
extern "C" THEME_EXPORT IThemePresentation* RetroMediaLabCreateTheme(ThemeService* themes,
    Translations::TranslationService* translations, WorkspaceState* state, char* error, std::size_t size) noexcept {
    try {
        if (!themes || !translations || !state) throw std::invalid_argument(ErrorKeys::InvalidThemeServices);
        return new RetroMediaLabPresentation(*themes, *translations, *state);
    } catch (const std::exception& exception) { report(error, size, exception.what()); }
      catch (...) { report(error, size, ErrorKeys::ThemeInitializationFailed); }
    return nullptr;
}
extern "C" THEME_EXPORT void RetroMediaLabDestroyTheme(IThemePresentation* presentation) noexcept { delete presentation; }
extern "C" THEME_EXPORT bool RetroMediaLabShowTheme(IThemePresentation* presentation, char* error, std::size_t size) noexcept {
    try { presentation->show(); return true; }
    catch (const std::exception& exception) { report(error, size, exception.what()); }
    catch (...) { report(error, size, ErrorKeys::ThemeDisplayFailed); }
    return false;
}
extern "C" THEME_EXPORT bool RetroMediaLabSynchronizeTheme(IThemePresentation* presentation, char* error, std::size_t size) noexcept {
    try { presentation->synchronizeState(); return true; }
    catch (const std::exception& exception) { report(error, size, exception.what()); }
    catch (...) { report(error, size, ErrorKeys::ThemeStateSynchronizationFailed); }
    return false;
}
extern "C" THEME_EXPORT QStyle* RetroMediaLabCreateThemeStyle(const std::shared_ptr<const ThemeData>* data,
    char* error, std::size_t size) noexcept {
    try {
        if (!data || !*data) throw std::invalid_argument(ErrorKeys::InvalidThemeData);
        return new RetroMediaLabStyle(*data);
    } catch (const std::exception& exception) { report(error, size, exception.what()); }
      catch (...) { report(error, size, ErrorKeys::ThemeStyleInitializationFailed); }
    return nullptr;
}

extern "C" THEME_EXPORT const char* RetroMediaLabThemeIdentifier() noexcept { return ThemeConstants::RetroMediaLab; }
extern "C" THEME_EXPORT void* RetroMediaLabCreateThemeConfiguration(ThemeService* service, char* error, std::size_t size) noexcept {
    try {
        if (!service) throw std::invalid_argument(ErrorKeys::MissingThemeService);
        return new RetroMediaLabConfiguration(*service);
    } catch (const std::exception& exception) { report(error, size, exception.what()); }
      catch (...) { report(error, size, ErrorKeys::ThemeConfigurationCreationFailed); }
    return nullptr;
}
extern "C" THEME_EXPORT void RetroMediaLabDestroyThemeConfiguration(void* configuration) noexcept {
    delete static_cast<RetroMediaLabConfiguration*>(configuration);
}
extern "C" THEME_EXPORT bool RetroMediaLabConfigureTheme(void* configuration, const QJsonObject* values,
    char* error, std::size_t size) noexcept {
    try {
        if (!configuration || !values) throw std::invalid_argument(ErrorKeys::MissingThemeConfiguration);
        static_cast<RetroMediaLabConfiguration*>(configuration)->apply(*values);
        return true;
    } catch (const std::exception& exception) { report(error, size, exception.what()); }
      catch (...) { report(error, size, ErrorKeys::ThemeConfigurationFailed); }
    return false;
}
