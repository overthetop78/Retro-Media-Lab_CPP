#pragma once
#include "Shared/Presentation/ThemeService.h"
#include "State/WorkspaceState.h"
#include "Media/ExplorerSession.h"
#include "MediaPageConstants.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QWidget>
#include <array>
class QCheckBox;
class QComboBox;
class QLabel;
class QFormLayout;
class QPushButton;
class QToolButton;
class QTreeWidget;
class QTreeWidgetItem;
class QTableWidget;
namespace RetroMediaLab {
class ClassificationPanel;
class MediaLoadingOverlay;
class ExplorerPage final : public QWidget {
public:
    ExplorerPage(ThemeService&, Translations::TranslationService&, WorkspaceState&, QWidget*);
private:
    std::shared_ptr<Infrastructure::VisualizationSession> visualization_;
    Infrastructure::ExplorerSession::Subscription visualizationSubscription_;
    bool visualLoading_=false;
    MediaLoadingOverlay* loading_;
    QToolButton* classificationToggle_;
    ClassificationPanel* classification_;
    QString text(const char*) const;
    void refresh();
    void refreshLoading();
    void rebuildVolumes();
    void rebuildTree();
    void showFolder(const std::vector<std::size_t>&);
    QString diskFormatName(const std::string&) const;
    QString entryType(const MediaEngine::FileSystemEntryInfo&) const;
    const MediaEngine::FileSystemEntryInfo* entryAt(int) const;
    QString volumeName(const MediaEngine::FileSystemVolumeInfo&) const;
    void showDetails(const MediaEngine::FileSystemEntryInfo*);
    std::shared_ptr<const MediaEngine::FileSystemVolumeInfo> volume() const;
    ThemeService& themes_;
    Translations::TranslationService& translations_;
    std::shared_ptr<Infrastructure::ExplorerSession> session_;
    std::shared_ptr<Infrastructure::ExplorerReadSession> reading_;
    QPushButton *open_, *read_;
    QComboBox *volumes_, *sessions_, *formats_;
    QLabel *status_, *detailTitle_, *detailIcon_, *detailWarnings_, *presentationLabel_, *readStatus_, *mediaIcon_, *mediaSummary_;
    QFormLayout* detailRows_;
    QCheckBox* automatic_;
    std::array<QLabel*, MediaPageConstants::SummaryKeys.size()> summaryLabels_, summaryValues_;
    QTreeWidget* folders_;
    QTableWidget* contents_;
    int sortColumn_ = MediaPageConstants::NameColumn;
    Qt::SortOrder sortOrder_ = Qt::AscendingOrder;
    std::vector<const MediaEngine::FileSystemEntryInfo*> rows_;
    std::vector<std::size_t> visibleVolumes_;
    std::map<QTreeWidgetItem*, std::vector<std::size_t>> paths_;
    std::shared_ptr<const MediaEngine::MediaOpeningAnalysisResult> displayed_;
    std::optional<std::string> displayedFormat_;
    Infrastructure::ExplorerSession::Subscription operationSubscription_,readSubscription_;
    Translations::TranslationService::Subscription languageSubscription_;
};
}
