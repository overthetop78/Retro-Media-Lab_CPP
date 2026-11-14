#include "RetroMediaLabPresentation.h"
#include "RetroMediaLabWindow.h"
namespace RetroMediaLab {
RetroMediaLabPresentation::RetroMediaLabPresentation(ThemeService& themes, Translations::TranslationService& translations, WorkspaceState& state)
    : window_(std::make_unique<RetroMediaLabWindow>(themes, translations, state)) {}
RetroMediaLabPresentation::~RetroMediaLabPresentation() = default;
void RetroMediaLabPresentation::synchronizeState() { window_->synchronizeState(); }
void RetroMediaLabPresentation::show() { window_->show(); }
}
