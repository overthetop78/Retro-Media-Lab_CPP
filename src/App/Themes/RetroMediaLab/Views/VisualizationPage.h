#pragma once
#include "MediaGraph.h"
#include "State/WorkspaceState.h"
#include <RetroMediaLab/Translations/TranslationService.h>
class QLabel;
class QPushButton;
class QSpinBox;
class QComboBox;
class QTableWidget;
class QPlainTextEdit;
namespace RetroMediaLab {
class VisualizationPage final : public QWidget {
public:
    VisualizationPage(ThemeService&, Translations::TranslationService&, WorkspaceState&, QWidget*);
private:
    void refresh();
    void showSector(int);
    QString text(const char*) const;
    Translations::TranslationService& translations_;
    std::shared_ptr<Infrastructure::VisualizationSession> session_;
    QLabel *status_, *sideLabel_, *trackLabel_, *revolutionLabel_, *zoom_;
    QPushButton *open_, *reset_, *zoomIn_, *zoomOut_;
    QSpinBox *side_, *track_;
    QComboBox* revolutions_;
    QTableWidget* sectors_;
    QPlainTextEdit* hex_;
    std::vector<const MediaEngine::DiskSectorInfo*> rows_;
    Infrastructure::ExplorerSession::Subscription subscription_, openingSubscription_;
    Translations::TranslationService::Subscription languageSubscription_;
};
}
