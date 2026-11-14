#pragma once
#include "State/WorkspaceState.h"
#include "Hardware/HostToolsService.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QWidget>
#include <memory>
class QLabel;
class QLineEdit;
class QPushButton;
class QProgressBar;
class QFileDialog;
class QMessageBox;
namespace RetroMediaLab {
class HostToolsSettingsWidget final : public QWidget {
public:
    HostToolsSettingsWidget(Translations::TranslationService&,WorkspaceState&,QWidget*);
    ~HostToolsSettingsWidget() override;
    void synchronizeState();
private:
    void refreshTexts();
    void selectPath(const QString&);
    void check(bool install);
    void busy(bool);
    void setStatus(const char*,const QString& = {});
    void failure(const char*);
    QString managedRoot()const;
    Translations::TranslationService& translations_;
    WorkspaceState& state_;
    QLabel *title_,*status_;
    QLineEdit* path_;
    QPushButton *browse_,*detect_,*check_,*download_,*rollback_;
    QProgressBar* progress_;
    const char* statusKey_ = HardwareConstants::HostNoneKey;
    QString statusArgument_;
    std::unique_ptr<HostToolsService> service_;
    std::unique_ptr<QFileDialog> picker_;
    std::unique_ptr<QMessageBox> confirmation_;
    Translations::TranslationService::Subscription subscription_;
};
}
