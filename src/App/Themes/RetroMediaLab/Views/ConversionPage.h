#pragma once
#include "Media/ConversionSession.h"
#include "Themes/ThemeService.h"
#include "State/WorkspaceState.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QWidget>
class QLineEdit;
class QLabel;
class QPushButton;
class QCheckBox;
class QTreeWidget;
class QListWidget;
class QProgressBar;
class QToolButton;
namespace RetroMediaLab {
class ConversionPage final:public QWidget {
public:
    ConversionPage(ThemeService&,Translations::TranslationService&,WorkspaceState&,QWidget*);
protected:
    void showEvent(QShowEvent*)override;
private:
    QString text(const char*)const;
    void refresh();
    void rebuildFormats();
    void selectFormats();
    void openSource();
    void execute();
    void askLosses();
    Translations::TranslationService& translations_;
    WorkspaceState& state_;
    std::shared_ptr<Infrastructure::ConversionSession> session_;
    QWidget *inputs_,*advanced_;
    QLineEdit *source_,*name_,*expert_;
    QLabel *sourceLabel_,*nameLabel_,*selectedLabel_,*formatLabel_,*expertLabel_,*summary_;
    QCheckBox *tags_,*rare_;
    QToolButton* advancedToggle_;
    QPushButton *browse_,*execute_;
    QTreeWidget* formats_;
    QListWidget* selected_;
    QProgressBar* progress_;
    struct Field {Infrastructure::CaptureOptionDefinition definition;QCheckBox* check;QLineEdit* value;};
    std::vector<Field> fields_;
    std::vector<std::string> displayedIds_;
    bool showingLosses_=false,rebuild_=true;
    Infrastructure::ExplorerSession::Subscription subscription_;
    WorkspaceState::DirectorySubscription engineSubscription_,settingsSubscription_;
    Translations::TranslationService::Subscription languageSubscription_;
};
}
