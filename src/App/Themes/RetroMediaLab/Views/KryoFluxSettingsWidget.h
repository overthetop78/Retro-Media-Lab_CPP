#pragma once
#include "State/WorkspaceState.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QWidget>
#include <memory>
class QLabel;
class QLineEdit;
class QPushButton;
class QFileDialog;
namespace RetroMediaLab {
class KryoFluxSettingsWidget final : public QWidget {
public:
    KryoFluxSettingsWidget(Translations::TranslationService&,WorkspaceState&,QWidget*);
    ~KryoFluxSettingsWidget() override;
    void synchronizeState();
private:
    void refreshTexts();
    void findTool();
    Translations::TranslationService& translations_;
    WorkspaceState& state_;
    QLabel *title_,*description_,*status_;
    QLineEdit* path_;
    QPushButton *browse_,*find_;
    bool notFound_=false;
    std::unique_ptr<QFileDialog> picker_;
    Translations::TranslationService::Subscription subscription_;
};
}
