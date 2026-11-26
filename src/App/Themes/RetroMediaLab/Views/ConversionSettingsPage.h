#pragma once
#include "Shared/Presentation/ThemeService.h"
#include "State/WorkspaceState.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QWidget>
#include <vector>
class QCheckBox;
class QComboBox;
class QLineEdit;
class QLabel;
class QListWidget;
class QPushButton;
namespace RetroMediaLab {
class ConversionSettingsPage final : public QWidget {
public:
    ConversionSettingsPage(ThemeService&, Translations::TranslationService&, WorkspaceState&, QWidget*);
    void commitEdits();
private:
    void refreshTexts();
    void refreshHistory();
    void refreshPreview();
    void selectPreset();
    Translations::TranslationService& translations_;
    WorkspaceState& state_;
    QCheckBox* enabled_;
    QComboBox* presets_;
    QLineEdit* pattern_;
    QListWidget* recent_;
    QLabel* preview_;
    QPushButton* next_;
    std::vector<std::pair<QLabel*, const char*>> labels_;
    std::vector<std::pair<QPushButton*, const char*>> variables_;
    int exampleIndex_ = 0;
    Translations::TranslationService::Subscription languageSubscription_;
};
}
