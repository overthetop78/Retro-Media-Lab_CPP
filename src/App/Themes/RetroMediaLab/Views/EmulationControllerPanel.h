#pragma once
class QWidget;
namespace RetroMediaLab {
namespace Translations { class TranslationService; }
QWidget* createEmulationControllerPanel(QWidget* parent, Translations::TranslationService& translations);
inline constexpr auto EmulationControllerTitleKey = "Controllers.Title";
}
