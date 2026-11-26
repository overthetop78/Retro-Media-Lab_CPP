#include "EmulationConfigurationsPanel.h"
#include "State/WorkspaceState.h"

#include <RetroMediaLab/Emulation/InputSettings.h>
#include <RetroMediaLab/Emulation/StorageSettings.h>
#include <RetroMediaLab/Translations/TranslationService.h>
#include <RetroMediaLab/VideoPresentation/Presentation.h>
#include <QComboBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTableWidget>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
#include <limits>

namespace RetroMediaLab {
namespace {
using namespace Emulation;
constexpr auto BrandKey = "Emulation.Configuration.Brand";
constexpr auto MachineKey = "Emulation.Configuration.Machine";
constexpr auto CpuTab = "Emulation.Tab.Cpu";
constexpr auto RamTab = "Emulation.Tab.Ram";
constexpr auto CpuModel = "Emulation.Cpu.Model";
constexpr auto DeleteKey = "Common.Delete";
constexpr auto ErrorKey = "Emulation.Error.EmulatorRejected";
constexpr auto ConfirmKey = "Emulation.Configuration.DeleteConfirm";
constexpr qint64 Kibibyte = 1024;
constexpr qint64 Mebibyte = Kibibyte * 1024;
constexpr qint64 Gibibyte = Mebibyte * 1024;
constexpr std::pair<const char*, qint64> MemoryUnits[]{
    {"GiB", Gibibyte}, {"MiB", Mebibyte}, {"KiB", Kibibyte}, {"bytes", 1}, {"B", 1}
};
constexpr const char* ColumnKeys[]{
    MachineKey, CpuTab, "Emulation.Configuration.TotalRam", "Emulation.Configuration.Readers",
    "Emulation.Configuration.Peripherals", "Emulation.Configuration.Actions"
};
enum Column { Machine, Cpu, Ram, Readers, Peripherals, Actions };

qint64 memoryBytes(const QString& display) {
    for (const auto& [unit, multiplier] : MemoryUnits) {
        const auto suffix = QLatin1String(unit);
        if (!display.endsWith(suffix, Qt::CaseInsensitive)) continue;
        bool valid = false;
        const auto number = QLocale::c().toDouble(display.chopped(suffix.size()).trimmed(), &valid);
        const auto bytes = number * multiplier;
        if (valid && std::isfinite(bytes) && bytes >= 0 && bytes < double(std::numeric_limits<qint64>::max()))
            return static_cast<qint64>(bytes);
    }
    return 0;
}

struct ConfigurationRow {
    QString module, brand, machine, cpu, ram, readers, peripherals;
    Configuration configuration;
};

class ConfigurationsPanel final : public QWidget {
public:
    ConfigurationsPanel(QWidget* parent, Translations::TranslationService& translations, WorkspaceState& state,
            const VideoPresentation::ProfileStore& profiles, EditSavedConfiguration edit)
        : QWidget(parent), translations_(translations), state_(state), profiles_(profiles), edit_(std::move(edit)) {
        auto* layout = new QVBoxLayout(this);
        auto* filters = new QFormLayout;
        brands_ = new QComboBox(this);
        machines_ = new QComboBox(this);
        filters->addRow(text(BrandKey), brands_);
        filters->addRow(text(MachineKey), machines_);
        layout->addLayout(filters);
        table_ = new QTableWidget(0, static_cast<int>(std::size(ColumnKeys)), this);
        QStringList columns;
        for (const auto* key : ColumnKeys) columns.append(text(key));
        table_->setHorizontalHeaderLabels(columns);
        table_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        table_->horizontalHeader()->setSectionResizeMode(Machine, QHeaderView::Stretch);
        table_->verticalHeader()->hide();
        table_->setSelectionBehavior(QAbstractItemView::SelectRows);
        table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table_->setAlternatingRowColors(true);
        layout->addWidget(table_, 1);
        status_ = new QLabel(this);
        status_->setTextFormat(Qt::PlainText);
        status_->setWordWrap(true);
        layout->addWidget(status_);
        connect(brands_, &QComboBox::currentIndexChanged, this, [this] { refreshMachines(); });
        connect(machines_, &QComboBox::currentIndexChanged, this, [this] { refreshTable(); });
    }

private:
    QString text(const char* key) const { return QString::fromStdString(translations_.text(key)); }

    QString moduleText(const QString& module, const QString& key, const QString& fallback) const {
        if (key.isEmpty()) return fallback;
        const auto localized = state_.emulation->translate(module, key, QString::fromStdString(translations_.language()));
        if (localized != key && !localized.isEmpty()) return localized;
        const auto shared = QString::fromStdString(translations_.text(key.toStdString()));
        return shared.isEmpty() || shared == key ? fallback : shared;
    }

    void showEvent(QShowEvent* event) override {
        QWidget::showEvent(event);
        if (!loaded_) reload();
    }

    void summarize(ConfigurationRow& row) const {
        qint64 memory = 0;
        bool hasMemory = false;
        for (const auto& block : state_.emulation->describe(row.module, row.configuration)) {
            if (!block.visible) continue;
            for (const auto& field : block.fields) {
                if (!field.visible) continue;
                const auto choice = std::ranges::find(field.choices, field.value, &SettingsChoice::id);
                const auto display = choice == field.choices.end() || choice->invariantDisplayValue.isEmpty()
                    ? field.value : choice->invariantDisplayValue;
                if (row.cpu.isEmpty() && block.tabResourceKey == QLatin1String(CpuTab) && field.labelResourceKey == QLatin1String(CpuModel)) {
                    row.cpu = choice == field.choices.end() ? display : moduleText(row.module, choice->displayResourceKey, display);
                } else if (block.tabResourceKey == QLatin1String(RamTab)) {
                    hasMemory = true;
                    const auto bytes = memoryBytes(display);
                    if (bytes <= std::numeric_limits<qint64>::max() - memory) memory += bytes;
                }
            }
        }
        if (hasMemory) row.ram = QLocale(QString::fromStdString(translations_.language())).formattedDataSize(memory);
        if (const auto input = state_.emulation->inputSettings(row.module, row.configuration)) {
            constexpr auto KeyboardKey = "Emulation.Tab.Keyboard";
            constexpr auto MouseKey = "Emulation.Tab.Mouse";
            constexpr auto NoneKey = "Emulation.Controller.None";
            QStringList peripherals;
            if (input->keyboard) peripherals.append(text(KeyboardKey));
            if (input->mouse) peripherals.append(text(MouseKey));
            for (const auto& port : input->controllerPorts) {
                const auto selected = std::ranges::find(port.controllerChoices, port.selectedControllerId, &ControllerChoice::id);
                if (selected == port.controllerChoices.end() || selected->category == ControllerCategory::None ||
                    selected->displayResourceKey == QLatin1String(NoneKey)) continue;
                if (selected->id == QLatin1String("Keyboard")) peripherals.append(text(KeyboardKey));
                else if (selected->id == QLatin1String("Mouse")) peripherals.append(text(MouseKey));
                else peripherals.append(moduleText(row.module, selected->displayResourceKey,
                    selected->invariantDisplayValue.isEmpty() ? selected->id : selected->invariantDisplayValue));
            }
            row.peripherals = peripherals.join(QLatin1String(" · "));
        }
        if (const auto storage = state_.emulation->storageSettings(row.module, row.configuration)) {
            QStringList readers;
            for (const auto& slot : storage->configuredSlots) {
                const auto found = std::ranges::find_if(storage->availableDevices, [&](const auto& item) {
                    return item.slot.id == slot.id && item.slot.kind == slot.kind;
                });
                if (found == storage->availableDevices.end()) continue;
                const auto key = found->slot.displayResourceKey.isEmpty() ? found->typeResourceKey : found->slot.displayResourceKey;
                readers.append(moduleText(row.module, key, found->displayLabel.isEmpty() ? slot.id : found->displayLabel));
            }
            row.readers = readers.join(QLatin1String(" · "));
        }
    }

    void reload() {
        loaded_ = true;
        rows_.clear();
        status_->clear();
        const auto selectedBrand = brands_->currentData();
        const QSignalBlocker block(brands_);
        brands_->clear();
        if (!state_.emulation) return;
        for (const auto& module : state_.emulation->modules()) {
            try {
                const auto brand = moduleText(module.id, module.displayResourceKey, module.id);
                const auto saved = state_.emulation->configurations(module.id);
                const auto before = rows_.size();
                for (const auto& configuration : saved) {
                    if (configuration.id.isNull()) continue;
                    const auto machine = std::ranges::find(module.machines, configuration.machineId, &MachineDefinition::id);
                    if (machine == module.machines.end()) continue;
                    ConfigurationRow row{module.id, brand,
                        moduleText(module.id, machine->displayResourceKey, machine->id), {}, {}, {}, {}, configuration};
                    try { summarize(row); } catch (...) { status_->setText(text(ErrorKey)); }
                    rows_.append(std::move(row));
                }
                if (rows_.size() > before) brands_->addItem(brand, module.id);
            } catch (...) {
                status_->setText(text(ErrorKey));
            }
        }
        std::stable_sort(rows_.begin(), rows_.end(), [](const auto& left, const auto& right) {
            return QString::localeAwareCompare(left.machine, right.machine) < 0;
        });
        const auto index = brands_->findData(selectedBrand);
        if (index >= 0) brands_->setCurrentIndex(index);
        refreshMachines();
    }

    void refreshMachines() {
        const auto selected = machines_->currentData();
        const auto module = brands_->currentData().toString();
        const QSignalBlocker block(machines_);
        machines_->clear();
        machines_->addItem(text("Visual.All"), QString{});
        for (const auto& row : rows_) {
            if (row.module == module && machines_->findData(row.configuration.machineId) < 0)
                machines_->addItem(row.machine, row.configuration.machineId);
        }
        const auto index = machines_->findData(selected);
        if (index >= 0) machines_->setCurrentIndex(index);
        machines_->setEnabled(!module.isEmpty());
        refreshTable();
    }

    bool hasSession(const ConfigurationRow& row) const {
        return std::ranges::any_of(state_.emulation->sessions(), [&](const auto& session) {
            return session->moduleId() == row.module && session->configuration().id == row.configuration.id;
        });
    }

    void remove(const ConfigurationRow& row) {
        if (hasSession(row)) return;
        auto message = text(ConfirmKey);
        message.replace(QLatin1String("{0}"), row.brand).replace(QLatin1String("{1}"), row.machine);
        QMessageBox confirmation(QMessageBox::Warning, text(DeleteKey), message, QMessageBox::Yes | QMessageBox::No, this);
        confirmation.setTextFormat(Qt::PlainText);
        confirmation.button(QMessageBox::Yes)->setText(text("Common.Yes"));
        confirmation.button(QMessageBox::No)->setText(text("Common.No"));
        confirmation.setDefaultButton(QMessageBox::No);
        if (confirmation.exec() != QMessageBox::Yes || hasSession(row)) return;
        try {
            state_.emulation->deleteConfiguration(row.module, row.configuration.id);
            const bool removed = profiles_.remove(row.module, row.configuration.id);
            reload();
            if (!removed) status_->setText(text(ErrorKey));
        } catch (...) {
            status_->setText(text(ErrorKey));
        }
    }

    void refreshTable() {
        table_->setRowCount(0);
        const auto brand = brands_->currentData().toString();
        const auto machine = machines_->currentData().toString();
        for (const auto& row : rows_) {
            if (row.module != brand || (!machine.isEmpty() && row.configuration.machineId != machine)) continue;
            const auto index = table_->rowCount();
            table_->insertRow(index);
            const auto name = row.configuration.name.trimmed();
            auto* description = new QTableWidgetItem(name.isEmpty() || name == row.machine ? row.machine : row.machine + u'\n' + name);
            table_->setItem(index, Machine, description);
            table_->setItem(index, Cpu, new QTableWidgetItem(row.cpu));
            table_->setItem(index, Ram, new QTableWidgetItem(row.ram));
            table_->setItem(index, Readers, new QTableWidgetItem(row.readers));
            table_->setItem(index, Peripherals, new QTableWidgetItem(row.peripherals));
            auto* controls = new QWidget(table_);
            auto* layout = new QHBoxLayout(controls);
            layout->setContentsMargins(0, 0, 0, 0);
            auto* edit = new QPushButton(text("Common.Modify"), controls);
            auto* removeButton = new QPushButton(text(DeleteKey), controls);
            removeButton->setEnabled(!hasSession(row));
            layout->addWidget(edit);
            layout->addWidget(removeButton);
            table_->setCellWidget(index, Actions, controls);
            connect(edit, &QPushButton::clicked, this, [this, row] {
                try {
                    edit_(this, row.module, row.configuration);
                    reload();
                } catch (...) { status_->setText(text(ErrorKey)); }
            });
            connect(removeButton, &QPushButton::clicked, this, [this, row] { remove(row); });
        }
        table_->resizeRowsToContents();
    }

    Translations::TranslationService& translations_;
    WorkspaceState& state_;
    const VideoPresentation::ProfileStore& profiles_;
    EditSavedConfiguration edit_;
    QList<ConfigurationRow> rows_;
    QComboBox* brands_ = nullptr;
    QComboBox* machines_ = nullptr;
    QTableWidget* table_ = nullptr;
    QLabel* status_ = nullptr;
    bool loaded_ = false;
};
}

QWidget* createEmulationConfigurationsPanel(QWidget* parent, Translations::TranslationService& translations,
        WorkspaceState& state, const VideoPresentation::ProfileStore& profiles, EditSavedConfiguration edit) {
    return new ConfigurationsPanel(parent, translations, state, profiles, std::move(edit));
}
}
