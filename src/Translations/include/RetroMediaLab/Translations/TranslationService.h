#pragma once
#include <RetroMediaLab/Translations/TranslationConstants.h>
#include "Export.h"
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
namespace RetroMediaLab::Translations {
class TranslationService final {
public:
    // Keep the subscription alive while the observer exists. All calls use the UI thread.
    using Subscription = std::shared_ptr<const std::function<void()>>;
    RETROMEDIALAB_TRANSLATIONS_API explicit TranslationService(std::filesystem::path catalogs, std::string domain = TranslationConstants::App);
    RETROMEDIALAB_TRANSLATIONS_API ~TranslationService();
    TranslationService(const TranslationService&) = delete;
    TranslationService& operator=(const TranslationService&) = delete;
    RETROMEDIALAB_TRANSLATIONS_API void setLanguage(const std::string& language);
    RETROMEDIALAB_TRANSLATIONS_API std::string language() const;
    RETROMEDIALAB_TRANSLATIONS_API std::string text(const std::string& key) const;
    RETROMEDIALAB_TRANSLATIONS_API Subscription subscribe(std::function<void()> observer);
private:
    struct Implementation;
    std::unique_ptr<Implementation> implementation_;
};
}
