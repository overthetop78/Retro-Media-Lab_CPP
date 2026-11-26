#pragma once
#include "MediaGraph.h"
#include "VisualizationConstants.h"
#include "State/WorkspaceState.h"
#include <RetroMediaLab/Translations/TranslationService.h>
class QLabel;
class QCheckBox;
class QPushButton;
class QToolButton;
class QSpinBox;
class QComboBox;
class QTableWidget;


namespace RetroMediaLab {
class ClassificationPanel;
class MediaLoadingOverlay;
class HexGrid;
class CaptureProgressView;
class VisualizationPage final : public QWidget {
public:
    VisualizationPage(ThemeService&, Translations::TranslationService&, WorkspaceState&, QWidget*);
protected:
    bool eventFilter(QObject*, QEvent*) override;
private:
    void setSummary(const QString&);
    void resizeSummary();
    void refresh();
    void refreshLoading();
    void refreshScpLegend();
    bool updateScpIdentity(const Infrastructure::LoadingProgress&);
    void showSector(int);
    void showBytes(const std::vector<std::uint8_t>&);
    MediaLoadingOverlay* loading_;
    QToolButton* classificationToggle_;
    ClassificationPanel* classification_;
    QString text(const char*) const;
    Translations::TranslationService& translations_;
    ThemeService& themes_;
    QWidget* legend_;
    std::array<QLabel*,VisualizationConstants::LegendCapacity> legendDots_,legendLabels_;
    ThemeService::Subscription themeSubscription_;
    std::shared_ptr<Infrastructure::VisualizationSession> session_;
    QCheckBox* linkZoom_;
    QLabel *mediaIcon_,*summary_,*elementDetails_,*status_, *sideLabel_, *trackLabel_, *revolutionLabel_, *zoom_,*decoderLabel_;
    QPushButton *open_, *reset_, *zoomIn_, *zoomOut_;
    QSpinBox *side_, *track_;
    QComboBox *revolutions_,*representation_,*decoder_,*diskFormat_;
    QWidget* overviewBar_;
    CaptureProgressView* overview_;
    QWidget* faceInspector_;
    std::array<QLabel*, VisualizationConstants::InspectorFaces> faceTitles_, faceDetails_;
    void fillSectorTable();
    QTableWidget* sectors_;
    HexGrid* hex_;
    MediaGraph* graph_ = nullptr;
    std::vector<const MediaEngine::DiskSectorInfo*> rows_;
    std::vector<MediaEngine::SectorVisualizationState> rowStates_;
    Infrastructure::ExplorerSession::Subscription subscription_, openingSubscription_;
    Translations::TranslationService::Subscription languageSubscription_;
};
}
