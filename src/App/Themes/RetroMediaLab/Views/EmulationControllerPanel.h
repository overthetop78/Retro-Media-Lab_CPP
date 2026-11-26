#pragma once
class QString;
class QWidget;
namespace RetroMediaLab {
namespace Translations { class TranslationService; }
QWidget* createEmulationControllerPanel(QWidget* parent, Translations::TranslationService& translations,
    QString logsDirectory);
inline constexpr auto EmulationControllerTitleKey = "Controllers.Title";
}
