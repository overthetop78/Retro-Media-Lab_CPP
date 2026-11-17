#pragma once
#include "Themes/ThemeService.h"
#include "State/WorkspaceState.h"
#include "Media/ExplorerSession.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QWidget>
class QComboBox;
class QLabel;
class QPushButton;
class QToolButton;
class QTreeWidget;
class QTreeWidgetItem;
class QTableWidget;
namespace RetroMediaLab {
class ClassificationPanel;
class ExplorerPage final : public QWidget {
public:
    ExplorerPage(ThemeService&, Translations::TranslationService&, WorkspaceState&, QWidget*);
private:
    QToolButton* classificationToggle_;
    ClassificationPanel* classification_;
    QString text(const char*) const;
    void refresh();
    void rebuildVolumes();
    void rebuildTree();
    void showFolder(const std::vector<std::size_t>&);
    QString volumeName(const MediaEngine::FileSystemVolumeInfo&) const;
    void showDetails(const MediaEngine::FileSystemEntryInfo*);
    std::shared_ptr<const MediaEngine::FileSystemVolumeInfo> volume() const;
    Translations::TranslationService& translations_;
    std::shared_ptr<Infrastructure::ExplorerSession> session_;
    std::shared_ptr<Infrastructure::ExplorerReadSession> reading_;
    QPushButton *open_, *cancel_,*read_;
    QComboBox *volumes_, *sessions_, *formats_;
    QLabel *status_, *details_,*readStatus_;
    QTreeWidget* folders_;
    QTableWidget* contents_;
    std::vector<const MediaEngine::FileSystemEntryInfo*> rows_;
    std::vector<std::size_t> visibleVolumes_;
    std::map<QTreeWidgetItem*, std::vector<std::size_t>> paths_;
    std::shared_ptr<const MediaEngine::MediaOpeningAnalysisResult> displayed_;
    Infrastructure::ExplorerSession::Subscription operationSubscription_,readSubscription_;
    Translations::TranslationService::Subscription languageSubscription_;
};
}
