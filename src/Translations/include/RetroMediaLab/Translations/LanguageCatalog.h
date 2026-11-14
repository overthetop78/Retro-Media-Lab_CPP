#pragma once
#include "Export.h"
#include <string>
#include <vector>
namespace RetroMediaLab::Translations {
struct Language {
    std::string code, culture, nativeName;
};
RETROMEDIALAB_TRANSLATIONS_API const std::vector<Language>& availableLanguages();
RETROMEDIALAB_TRANSLATIONS_API bool isSupportedLanguage(const std::string&);
RETROMEDIALAB_TRANSLATIONS_API std::string resolveLanguage(const std::string& culture);
}
