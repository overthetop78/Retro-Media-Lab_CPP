#pragma once
#include "Shared/Presentation/IThemePresentation.h"
#include <memory>
namespace RetroMediaLab {
class RetroMediaLabWindow;
class ThemeService;
struct WorkspaceState;
namespace Translations { class TranslationService; }
class RetroMediaLabPresentation final : public IThemePresentation {
public:
    RetroMediaLabPresentation(ThemeService&, Translations::TranslationService&, WorkspaceState&);
    ~RetroMediaLabPresentation() override;
    void synchronizeState() override;
    void show() override;
private:
    std::unique_ptr<RetroMediaLabWindow> window_;
};
}
