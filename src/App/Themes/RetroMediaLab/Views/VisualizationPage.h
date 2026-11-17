#pragma once
#include "MediaGraph.h"
#include "VisualizationConstants.h"
#include "State/WorkspaceState.h"
#include <RetroMediaLab/Translations/TranslationService.h>
class QLabel;
class QPushButton;
class QSpinBox;
class QComboBox;
class QTableWidget;

namespace RetroMediaLab {
class HexGrid;
class VisualizationPage final : public QWidget {
public:
    VisualizationPage(ThemeService&, Translations::TranslationService&, WorkspaceState&, QWidget*);
private:
    void refresh();
    void showSector(int);
    void showBytes(const std::vector<std::uint8_t>&);
    QString text(const char*) const;
    Translations::TranslationService& translations_;
    ThemeService& themes_;
    QWidget* legend_;
    std::array<QLabel*,VisualizationConstants::SectorLabels.size()> legendDots_,legendLabels_;
    ThemeService::Subscription themeSubscription_;
    std::shared_ptr<Infrastructure::VisualizationSession> session_;
    QLabel *summary_,*elementDetails_,*status_, *sideLabel_, *trackLabel_, *revolutionLabel_, *zoom_;
    QPushButton *open_, *reset_, *zoomIn_, *zoomOut_;
    QSpinBox *side_, *track_;
    QComboBox *revolutions_,*representation_;
    QTableWidget* sectors_;
    HexGrid* hex_;
    std::vector<const MediaEngine::DiskSectorInfo*> rows_;
    std::vector<MediaEngine::SectorVisualizationState> rowStates_;
    Infrastructure::ExplorerSession::Subscription subscription_, openingSubscription_;
    Translations::TranslationService::Subscription languageSubscription_;
};
}
