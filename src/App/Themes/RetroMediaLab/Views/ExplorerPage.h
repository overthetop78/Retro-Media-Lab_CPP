#pragma once
#include "Themes/ThemeService.h"
#include "State/WorkspaceState.h"
#include "Media/ExplorerSession.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QWidget>
class QComboBox;
class QLabel;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;
class QTableWidget;
namespace RetroMediaLab {
class ExplorerPage final : public QWidget {
public:
    ExplorerPage(ThemeService&, Translations::TranslationService&, WorkspaceState&, QWidget*);
private:
    QString text(const char*) const;
    void refresh();
    void rebuildVolumes();
    void rebuildTree();
    void showFolder(const std::vector<std::size_t>&);
    void showDetails(const MediaEngine::FileSystemEntryInfo*);
    std::shared_ptr<const MediaEngine::FileSystemVolumeInfo> volume() const;
    Translations::TranslationService& translations_;
    std::shared_ptr<Infrastructure::ExplorerSession> session_;
    QPushButton *open_, *cancel_;
    QComboBox *volumes_, *sessions_, *formats_;
    QLabel *status_, *details_;
    QTreeWidget* folders_;
    QTableWidget* contents_;
    std::vector<const MediaEngine::FileSystemEntryInfo*> rows_;
    std::vector<std::size_t> visibleVolumes_;
    std::map<QTreeWidgetItem*, std::vector<std::size_t>> paths_;
    std::shared_ptr<const MediaEngine::MediaOpeningAnalysisResult> displayed_;
    Infrastructure::ExplorerSession::Subscription operationSubscription_;
    Translations::TranslationService::Subscription languageSubscription_;
};
}
