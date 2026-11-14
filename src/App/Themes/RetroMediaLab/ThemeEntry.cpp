#include "RetroMediaLabPresentation.h"
#include "RetroMediaLabStyle.h"
#include "Themes/ThemeService.h"
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
    if (output && size) std::snprintf(output, size, "%s", message);
}
}
extern "C" THEME_EXPORT unsigned RetroMediaLabThemeApiVersion() noexcept { return 1; }
extern "C" THEME_EXPORT const char* RetroMediaLabThemeResourcePrefix() noexcept { return ":/Themes/RetroMediaLab"; }
extern "C" THEME_EXPORT IThemePresentation* RetroMediaLabCreateTheme(ThemeService* themes,
    Translations::TranslationService* translations, WorkspaceState* state, char* error, std::size_t size) noexcept {
    try {
        if (!themes || !translations || !state) throw std::invalid_argument("Invalid theme services");
        return new RetroMediaLabPresentation(*themes, *translations, *state);
    } catch (const std::exception& exception) { report(error, size, exception.what()); }
      catch (...) { report(error, size, "Theme initialization failed"); }
    return nullptr;
}
extern "C" THEME_EXPORT void RetroMediaLabDestroyTheme(IThemePresentation* presentation) noexcept { delete presentation; }
extern "C" THEME_EXPORT bool RetroMediaLabShowTheme(IThemePresentation* presentation, char* error, std::size_t size) noexcept {
    try { presentation->show(); return true; }
    catch (const std::exception& exception) { report(error, size, exception.what()); }
    catch (...) { report(error, size, "Theme display failed"); }
    return false;
}
extern "C" THEME_EXPORT bool RetroMediaLabSynchronizeTheme(IThemePresentation* presentation, char* error, std::size_t size) noexcept {
    try { presentation->synchronizeState(); return true; }
    catch (const std::exception& exception) { report(error, size, exception.what()); }
    catch (...) { report(error, size, "Theme state synchronization failed"); }
    return false;
}
extern "C" THEME_EXPORT QStyle* RetroMediaLabCreateThemeStyle(const std::shared_ptr<const ThemeData>* data,
    char* error, std::size_t size) noexcept {
    try {
        if (!data || !*data) throw std::invalid_argument("Invalid theme data");
        return new RetroMediaLabStyle(*data);
    } catch (const std::exception& exception) { report(error, size, exception.what()); }
      catch (...) { report(error, size, "Theme style initialization failed"); }
    return nullptr;
}
