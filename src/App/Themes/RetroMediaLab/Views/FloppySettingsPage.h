#pragma once
#include "Shared/Presentation/ThemeService.h"
#include "State/WorkspaceState.h"
#include "Hardware/GreaseweazleDiscovery.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QWidget>
class QComboBox;
class QPushButton;
class QLabel;
class QVBoxLayout;
class QScrollArea;
class QMessageBox;
namespace RetroMediaLab {
class HostToolsSettingsWidget;
class KryoFluxSettingsWidget;
class FloppySettingsPage final : public QWidget {
public:
    FloppySettingsPage(ThemeService&,Translations::TranslationService&,WorkspaceState&,QWidget*);
    ~FloppySettingsPage() override;
    void synchronizeState();
private:
    struct ReaderRow;
    void refreshTexts();
    void rememberReaderEdits();
    void rebuildRows();
    void addReader();
    void saveReader(const std::shared_ptr<ReaderRow>&);
    void forgetReader(const std::shared_ptr<ReaderRow>&);
    void scan();
    void failure(const char*);
    ThemeService& themes_;Translations::TranslationService& translations_;WorkspaceState& state_;
    QPushButton *scan_,*add_;
    QComboBox *controllers_,*backend_;
    QLabel* backendLabel_;
    QLabel *persistence_,*status_;
    QString statusKey_;
    QWidget* rowsContainer_=nullptr;
    QVBoxLayout* rowsLayout_;
    QScrollArea* readerScroll_;
    HostToolsSettingsWidget* tools_;
    KryoFluxSettingsWidget* kryoTools_;
    QVector<Infrastructure::HostDevice> detected_;
    QVector<HardwareDevice> drafts_;
    QVector<std::shared_ptr<ReaderRow>> rows_;
    std::unique_ptr<GreaseweazleDiscovery> discovery_;
    std::unique_ptr<QMessageBox> confirmation_;
    Translations::TranslationService::Subscription subscription_;
};
}
