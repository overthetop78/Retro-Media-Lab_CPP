#pragma once

#include <RetroMediaLab/Emulation/StorageSettings.h>
#include <QWidget>

class QLabel;
class QDialog;
class QFormLayout;
class QLineEdit;
class QListWidget;
class QPushButton;

namespace RetroMediaLab {
struct WorkspaceState;
namespace Translations { class TranslationService; }

class EmulationStoragePanel final : public QWidget {
public:
    using Localize = std::function<QString(const QString&)>;
    EmulationStoragePanel(Emulation::StorageSettings, WorkspaceState&, const Translations::TranslationService&,
        QString scope, Localize, QHash<QString, QString> detailedHelpResourceKeys,
        QHash<QString, Emulation::SettingsChoicePresentation> choicePresentation, QWidget* parent);
    static std::optional<QString> selectMemoryCard(QWidget&, const Emulation::StorageDevice&, QString title,
        const QString& current, WorkspaceState&, const Translations::TranslationService&, const QString& scope,
        const Localize&);
    const Emulation::StorageSettings& settings() const noexcept { return settings_; }
    bool changed() const noexcept { return changed_; }
    void onChanged(std::function<void()> callback) { onChanged_ = std::move(callback); }

private:
    QString label(const char*) const;
    QString deviceLabel(const Emulation::StorageDevice&) const;
    QString associatedPath(const Emulation::StorageDevice&) const;
    const Emulation::StorageDevice* currentDevice() const;
    void rebuild(const Emulation::MediaSlot* current = nullptr);
    void updateActions();
    void addDevice();
    void removeDevice();
    void configureDevice();
    QString createBlankFloppy(QWidget&, const Emulation::StorageDevice&, const QString& modelId);
    QString createHardDisk(QWidget&, const Emulation::StorageDevice&, const QString& directory, const QString& scope);
    static void addMemoryCardPathControls(QDialog&, QFormLayout&, QLineEdit&, const Emulation::StorageDevice&,
        WorkspaceState&, const Translations::TranslationService&, const QString& scope, const Localize&);
    static bool confirmImageCreation(QWidget&, const Translations::TranslationService&, const QString& title, const QString& path,
        const std::function<void(std::stop_token)>& write, bool canAttach);

    Emulation::StorageSettings settings_;
    WorkspaceState& workspace_;
    const Translations::TranslationService& translations_;
    QString scope_;
    Localize localize_;
    QHash<QString, QString> detailedHelpResourceKeys_;
    QHash<QString, Emulation::SettingsChoicePresentation> choicePresentation_;
    QList<int> rows_;
    QListWidget* devices_;
    QPushButton* add_;
    QPushButton* remove_;
    QPushButton* configure_;
    QLabel* status_;
    bool changed_ = false;
    std::function<void()> onChanged_;
};
}
