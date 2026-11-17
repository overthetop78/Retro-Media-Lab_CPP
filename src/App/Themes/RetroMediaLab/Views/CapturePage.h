#pragma once
#include "Media/CaptureSession.h"
#include "Themes/ThemeService.h"
#include "State/WorkspaceState.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QWidget>
class QLabel;
class QLineEdit;
class QPushButton;
class QComboBox;
class QRadioButton;
class QCheckBox;
class QSpinBox;
class QToolButton;
namespace RetroMediaLab {
class CaptureProgressView;
class CapturePage final:public QWidget {
public:
    CapturePage(Infrastructure::CaptureMode,ThemeService&,Translations::TranslationService&,WorkspaceState&,std::function<void(const QString&)>,QWidget*);
protected:
    void showEvent(QShowEvent*) override;
private:
    QString text(const char*)const;
    void refresh();
    void rebuildFormats();
    void execute();
    void browse();
    void saveProfile();
    void openResult(const char*);
    bool confirm(const QString&,const QString&);
    std::shared_ptr<Infrastructure::CaptureSession> session_;
    Translations::TranslationService& translations_;
    WorkspaceState& state_;
    std::function<void(const QString&)> navigate_;
    CaptureProgressView* progress_;
    QWidget *inputs_,*advanced_,*sequencePanel_;
    QLabel *pathLabel_,*nameLabel_,*profileLabel_,*expertLabel_,*summary_;
    QLineEdit *path_,*name_,*expert_,*sequence_;
    QComboBox *devices_,*formats_,*profiles_,*sequenceKind_,*extensions_;
    QSpinBox* sequenceWidth_;
    QRadioButton *raw_=nullptr,*known_=nullptr;
    QCheckBox* autoNumber_;
    QToolButton* advancedToggle_;
    QPushButton *browse_,*execute_,*save_,*reset_,*visualize_,*explore_;
    struct OptionField {Infrastructure::CaptureOptionDefinition definition;QCheckBox* check;QLineEdit* value;QComboBox* choices;QPushButton* browse;};
    std::vector<OptionField> fields_;
    Infrastructure::ExplorerSession::Subscription subscription_;
    WorkspaceState::DirectorySubscription hardwareSubscription_,engineSubscription_;
    Translations::TranslationService::Subscription languageSubscription_;
    bool sourceEdited_=false;
};
}
