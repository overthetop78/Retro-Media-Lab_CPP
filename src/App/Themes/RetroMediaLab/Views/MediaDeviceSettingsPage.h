#pragma once
#include "HardwareCapabilitiesTable.h"
#include "Shared/Presentation/ThemeService.h"
#include "State/WorkspaceState.h"
#include <RetroMediaLab/Infrastructure/HardwareDiscovery.h>
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QWidget>
#include <memory>
class QComboBox;
class QLabel;
class QPushButton;
class QVBoxLayout;
class QMessageBox;
namespace RetroMediaLab {
class OpticalProbe;
class DiskProbe;
class MediaDeviceSettingsPage final : public QWidget {
public:
    MediaDeviceSettingsPage(const HardwareCategory&,ThemeService&,Translations::TranslationService&,WorkspaceState&,QWidget*);
    ~MediaDeviceSettingsPage() override;
private:
    void scan();
    void addDevice();
    void rebuild();
    void refreshTexts();
    void forget(const QString&);
    void testOptical(const QString&,const QString&);
    QString deviceLabel(const Infrastructure::HostDevice&) const;
    void refreshDiskChoices();
    const Infrastructure::HostDevice* selectedDevice() const;
    void testDisk(const QString&,const QString&);
    QVector<CapabilityRow> diskRows(const Infrastructure::DiskAccessReport&) const;
    QVector<CapabilityRow> opticalRows(const Infrastructure::OpticalCapabilities&) const;
    QString category_,statusKey_;
    ThemeService& themes_;Translations::TranslationService& translations_;WorkspaceState& state_;
    QComboBox* choices_;
    QComboBox* diskControllers_ = nullptr;
    QLabel* diskInterfaceLabel_ = nullptr;
    QPushButton *scan_,*add_,*test_;
    QLabel *explanation_,*status_;
    QWidget* cards_=nullptr;
    QVBoxLayout* body_;
    QVector<Infrastructure::HostDevice> detected_;
    QVector<Infrastructure::DiskController> controllerInventory_;
    std::unique_ptr<QMessageBox> confirmation_;
    std::unique_ptr<OpticalProbe> probe_;
    std::unique_ptr<DiskProbe> diskProbe_;
    Translations::TranslationService::Subscription subscription_;
};
}
