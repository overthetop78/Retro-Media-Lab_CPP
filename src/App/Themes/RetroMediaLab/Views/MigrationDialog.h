#pragma once
#include "State/WorkspaceState.h"
#include "Themes/ThemeService.h"
#include "Media/MigrationSession.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QDialog>
class QLineEdit;class QLabel;class QComboBox;class QCheckBox;class QPushButton;class QTreeWidget;class QProgressBar;
namespace RetroMediaLab {
class MigrationDialog final:public QDialog {
public:
    MigrationDialog(ThemeService&,Translations::TranslationService&,WorkspaceState&,QWidget*);
    ~MigrationDialog() override;
    void synchronizeState();
protected:
    void done(int)override;
    void showEvent(QShowEvent*)override;
private:
    QString text(const char*)const;
    void refresh();
    void appearance();
    ThemeService& themes_;
    Translations::TranslationService& translations_;
    WorkspaceState& state_;
    std::shared_ptr<Infrastructure::MigrationSession> session_;
    QLineEdit *source_,*output_;
    QLabel *sourceLabel_,*destinationLabel_,*outputLabel_,*summary_,*status_;
    QComboBox* targets_;
    QCheckBox* accept_;
    QTreeWidget* losses_;
    QPushButton *browseSource_,*browseOutput_,*execute_,*close_;
    QProgressBar* progress_;
    bool closing_=false;
    Infrastructure::ExplorerSession::Subscription subscription_;
    Translations::TranslationService::Subscription language_;
    ThemeService::Subscription theme_;
};
}
