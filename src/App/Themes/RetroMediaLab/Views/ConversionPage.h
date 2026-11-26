#pragma once
#include "Media/ConversionSession.h"
#include "Shared/Presentation/ThemeService.h"
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
class QComboBox;
namespace RetroMediaLab {
class MediaLoadingOverlay;
class ConversionPage final:public QWidget {
public:
    ConversionPage(ThemeService&,Translations::TranslationService&,WorkspaceState&,std::function<void()>,QWidget*);
protected:
    void showEvent(QShowEvent*)override;
private:
    MediaLoadingOverlay* loading_;
    QString text(const char*)const;
    void refresh();
    void rebuildFormats();
    void selectFormats();
    QString selectionSignature()const;
    void openSource();
    void execute();
    void askLosses();
    void saveProfile();
    Translations::TranslationService& translations_;
    WorkspaceState& state_;
    std::shared_ptr<Infrastructure::ConversionSession> session_;
    QWidget *inputs_,*advanced_;
    QLineEdit *source_,*name_,*expert_;
    QLabel *sourceLabel_,*nameLabel_,*selectedLabel_,*formatLabel_,*expertLabel_,*summary_;
    QCheckBox *tags_,*rare_;
    QToolButton* advancedToggle_;
    QPushButton *browse_,*execute_,*migration_;
    QTreeWidget* formats_;
    QListWidget* selected_;
    QProgressBar* progress_;
    QComboBox* profiles_;
    QLabel* profileLabel_;
    QPushButton *save_,*reset_;
    struct Field {Infrastructure::CaptureOptionDefinition definition;QCheckBox* check;QLineEdit* value;};
    std::vector<Field> fields_;
    std::vector<std::string> displayedIds_;
    QString selectionSignature_;
    bool showingLosses_=false,rebuild_=true;
    Infrastructure::ExplorerSession::Subscription subscription_;
    WorkspaceState::DirectorySubscription engineSubscription_,settingsSubscription_;
    Translations::TranslationService::Subscription languageSubscription_;
};
}
