#include "EmulationStoragePanel.h"
#include "ModuleSettingPresentation.h"
#include "Shared/Emulation/EmulationWorkspace.h"
#include "Shared/Emulation/ModuleNumericSetting.h"
#include "Shared/Controls/RememberedFileDialog.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QLocale>
#include <QProgressBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
#include <limits>
#include <thread>

namespace RetroMediaLab {
namespace {
using namespace Emulation;
constexpr auto PathSeparator = " — ";
constexpr auto BlankImageName = "blank";
constexpr auto FileExtensionFilter = "%1 (*%2)";

const char* memoryCardError(StorageRules::MemoryCardPathIssue issue) {
    using Issue = StorageRules::MemoryCardPathIssue;
    switch (issue) {
    case Issue::Missing: return "Emulation.Storage.MemoryCard.ImageRequired";
    case Issue::InvalidFormat: return "Emulation.Storage.MemoryCard.InvalidFormat";
    case Issue::InvalidSize: return "Emulation.Storage.MemoryCard.InvalidSize";
    case Issue::None: return nullptr;
    }
    return nullptr;
}

bool sameSlot(const MediaSlot& left, const MediaSlot& right) {
    return left.kind == right.kind && left.id == right.id;
}

bool hasSlot(const QList<MediaSlot>& configured, const MediaSlot& target) {
    return std::ranges::any_of(configured, [&](const auto& slot) { return sameSlot(slot, target); });
}

const char* typeKey(MediaKind kind) {
    switch (kind) {
    case MediaKind::Floppy: return "Emulation.Storage.Floppy.Device";
    case MediaKind::HardDisk: return "Emulation.Storage.HardDisk.Device";
    case MediaKind::OpticalDisc: return "Emulation.Storage.Cd.Device";
    case MediaKind::Tape: return "Emulation.Storage.Tape.Device";
    case MediaKind::Cartridge: return "Emulation.Storage.Cartridge.Device";
    case MediaKind::MemoryCard: return "Emulation.Storage.MemoryCard.Device";
    }
    return "Emulation.Storage.Device.List";
}
}

EmulationStoragePanel::EmulationStoragePanel(StorageSettings settings, WorkspaceState& workspace,
    const Translations::TranslationService& translations, QString scope, Localize localize,
    QHash<QString, QString> detailedHelpResourceKeys, QHash<QString, SettingsChoicePresentation> choicePresentation,
    QWidget* parent)
    : QWidget(parent), settings_(std::move(settings)), workspace_(workspace), translations_(translations),
      scope_(std::move(scope)), localize_(std::move(localize)),
      detailedHelpResourceKeys_(std::move(detailedHelpResourceKeys)), choicePresentation_(std::move(choicePresentation)),
      devices_(new QListWidget(this)),
      add_(new QPushButton(label("Emulation.Storage.Device.Add"), this)),
      remove_(new QPushButton(label("Emulation.Storage.Media.Remove"), this)),
      configure_(new QPushButton(label("Emulation.Storage.Device.Configure"), this)), status_(new QLabel(this)) {
    auto* layout = new QVBoxLayout(this);
    auto* actions = new QHBoxLayout;
    actions->addWidget(add_);
    actions->addWidget(configure_);
    actions->addWidget(remove_);
    actions->addStretch();
    layout->addLayout(actions);
    layout->addWidget(devices_, 1);
    status_->setTextFormat(Qt::PlainText);
    status_->setWordWrap(true);
    layout->addWidget(status_);
    connect(devices_, &QListWidget::currentRowChanged, this, [this] { updateActions(); });
    connect(add_, &QPushButton::clicked, this, [this] { addDevice(); });
    connect(remove_, &QPushButton::clicked, this, [this] { removeDevice(); });
    connect(configure_, &QPushButton::clicked, this, [this] { configureDevice(); });
    rebuild();
}

QString EmulationStoragePanel::label(const char* key) const { return QString::fromStdString(translations_.text(key)); }

QString EmulationStoragePanel::deviceLabel(const StorageDevice& device) const {
    if (!device.slot.displayResourceKey.isEmpty()) return localize_(device.slot.displayResourceKey);
    return device.displayLabel.isEmpty() ? device.slot.id : device.displayLabel;
}

QString EmulationStoragePanel::associatedPath(const StorageDevice& device) const {
    if (device.slot.kind == MediaKind::MemoryCard) {
        const auto found = std::ranges::find_if(settings_.deviceSettings,
            [&](const auto& entry) { return sameSlot(entry.slot, device.slot); });
        return found == settings_.deviceSettings.end() ? QString{} : found->memoryCardPath;
    }
    const auto found = std::ranges::find_if(settings_.mountedMedia,
        [&](const auto& entry) { return sameSlot(entry.media.slot, device.slot); });
    return found == settings_.mountedMedia.end() ? QString{} : found->media.path;
}

const StorageDevice* EmulationStoragePanel::currentDevice() const {
    const auto row = devices_->currentRow();
    return row >= 0 && row < rows_.size() ? &settings_.availableDevices.at(rows_.at(row)) : nullptr;
}

void EmulationStoragePanel::rebuild(const MediaSlot* current) {
    devices_->clear();
    rows_.clear();
    for (const bool permanent : {true, false}) {
        for (qsizetype i = 0; i < settings_.availableDevices.size(); ++i) {
            const auto& device = settings_.availableDevices.at(i);
            if (device.permanent != permanent || !hasSlot(settings_.configuredSlots, device.slot)) continue;
            rows_.append(static_cast<int>(i));
            const auto path = associatedPath(device);
            const auto name = deviceLabel(device);
            auto* item = new QListWidgetItem(path.isEmpty() ? name : name + QString::fromUtf8(PathSeparator) +
                QFileInfo(path).fileName(), devices_);
            if (!path.isEmpty()) item->setToolTip(path);
            if (current && sameSlot(device.slot, *current)) devices_->setCurrentItem(item);
        }
    }
    if (devices_->currentRow() < 0 && devices_->count() > 0) devices_->setCurrentRow(0);
    updateActions();
}

void EmulationStoragePanel::updateActions() {
    const auto* device = currentDevice();
    remove_->setEnabled(device && !device->permanent);
    configure_->setEnabled(device && (device->configurationKind != StorageConfigurationKind::None));
    add_->setEnabled(std::ranges::any_of(settings_.availableDevices, [&](const auto& item) {
        return !hasSlot(settings_.configuredSlots, item.slot);
    }));
    status_->setText(devices_->count() == 0 ? label("Emulation.Storage.Media.None") : QString{});
}

void EmulationStoragePanel::addDevice() {
    QList<int> available;
    QStringList choices;
    QSet<MediaKind> offered;
    for (qsizetype i = 0; i < settings_.availableDevices.size(); ++i) {
        const auto& device = settings_.availableDevices.at(i);
        if (hasSlot(settings_.configuredSlots, device.slot) || offered.contains(device.slot.kind)) continue;
        offered.insert(device.slot.kind);
        available.append(static_cast<int>(i));
        choices.append(label(typeKey(device.slot.kind)));
    }
    if (choices.isEmpty()) return;
    bool accepted = false;
    const auto choice = QInputDialog::getItem(this, label("Emulation.Storage.Device.Add"),
        label("Emulation.Storage.Device.CapabilitiesHint"), choices, 0, false, &accepted);
    const auto row = choices.indexOf(choice);
    if (!accepted || row < 0) return;
    const auto& device = settings_.availableDevices.at(available.at(row));
    settings_.configuredSlots.append(device.slot);
    if (device.floppyOptions && !device.floppyOptions->models.isEmpty()) {
        StorageDeviceSettings entry;
        entry.slot = device.slot;
        entry.floppy = FloppyDriveSettings{device.floppyOptions->models.front().choice.id,
            StorageRules::standardDriveSpeedChoices().constFirst().id};
        settings_.deviceSettings.append(std::move(entry));
    }
    changed_ = true;
    rebuild(&device.slot);
    if (onChanged_) onChanged_();
}

void EmulationStoragePanel::removeDevice() {
    const auto* device = currentDevice();
    if (!device || device->permanent) return;
    settings_.configuredSlots.removeIf([&](const auto& slot) { return sameSlot(slot, device->slot); });
    settings_.mountedMedia.removeIf([&](const auto& mount) { return sameSlot(mount.media.slot, device->slot); });
    settings_.deviceSettings.removeIf([&](const auto& entry) { return sameSlot(entry.slot, device->slot); });
    changed_ = true;
    rebuild();
    if (onChanged_) onChanged_();
}

QString EmulationStoragePanel::createBlankFloppy(QWidget& parent, const StorageDevice& device, const QString& modelId) {
    const auto& options = *device.floppyOptions;
    const auto model = std::ranges::find_if(options.models, [&](const auto& item) { return item.choice.id == modelId; });
    const auto* capabilities = model != options.models.end() ? &*model
        : (device.floppyModel ? &*device.floppyModel : nullptr);
    auto formats = capabilities ? capabilities->creationFormats : QList<FloppyCreationFormat>{};
    if (formats.isEmpty() && capabilities) {
        auto extensions = capabilities->blankFormats.keys();
        extensions.sort(Qt::CaseInsensitive);
        for (const auto& extension : extensions)
            formats.append({capabilities->blankFormats.value(extension), extension, extension.mid(1).toUpper()});
    }
    if (formats.isEmpty()) {
        const auto& extension = options.defaultExtension;
        formats.append({{}, extension, extension.mid(1).toUpper()});
    }

    const auto title = label("Emulation.Storage.Floppy.CreateBlank");
    qsizetype row = 0;
    if (formats.size() > 1) {
        QStringList names;
        for (const auto& format : formats) names.append(format.displayName);
        bool accepted = false;
        const auto name = QInputDialog::getItem(&parent, title, label("Emulation.Storage.Floppy.ImageFormat"),
            names, 0, false, &accepted);
        if (!accepted) return {};
        row = names.indexOf(name);
        if (row < 0) return {};
    }
    const auto& format = formats.at(row);
    QFileDialog destination(&parent, title);
    destination.setAcceptMode(QFileDialog::AcceptSave);
    destination.setFileMode(QFileDialog::AnyFile);
    if (!format.extension.isEmpty()) {
        destination.setDefaultSuffix(format.extension.mid(1));
        destination.setNameFilter(QString::fromLatin1(FileExtensionFilter).arg(format.displayName, format.extension));
    }
    const auto scope = scope_ + u'/' + device.slot.id;
    const auto folder = options.imageDirectory.isEmpty() ? resolveEmulationFolders(workspace_.emulationFolders()).storage : options.imageDirectory;
    RememberedFileDialog::prepare(destination, workspace_, scope, folder);
    RememberedFileDialog::remember(destination, workspace_, scope);
    destination.selectFile(QString::fromLatin1(BlankImageName) + format.extension);
    if (destination.exec() != QDialog::Accepted) return {};
    const auto path = destination.selectedFiles().value(0);
    const auto suffix = QString(u'.') + QFileInfo(path).suffix();
    if (path.isEmpty() || (!format.extension.isEmpty() && suffix.compare(format.extension, Qt::CaseInsensitive) != 0)) {
        QMessageBox::warning(&parent, title, label("Emulation.Storage.MemoryCard.InvalidFormat"));
        return {};
    }

    const StorageMount candidate{{device.slot, path}, true};
    const auto canAttach = StorageRules::validMount(candidate, settings_.availableDevices, settings_.configuredSlots);
    const auto write = [&](std::stop_token token) {
        workspace_.emulation->createBlankFloppy(path, format.formatId, capabilities ? capabilities->blankImageSize : 0, token);
    };
    return confirmImageCreation(parent, translations_, title, path, write, canAttach) ? path : QString{};
}

QString EmulationStoragePanel::createHardDisk(QWidget& parent, const StorageDevice& device,
        const QString& directory, const QString& scope) {
    constexpr qint64 Mebibyte = 1024 * 1024;
    constexpr qint64 SizePresets[]{20, 40, 80, 120, 250, 500, 1024, 2047};
    const QLocale locale(QString::fromStdString(translations_.language()));
    const auto title = label("Emulation.Storage.HardDisk.Create");
    QDialog options(&parent);
    options.setWindowTitle(title);
    auto* form = new QFormLayout(&options);
    auto* formats = new QComboBox(&options);
    QList<const HardDiskImageFormat*> available;
    for (const auto& format : device.hardDiskFormats) {
        if (!format.canCreate || format.extension.isEmpty()) continue;
        available.append(&format);
        formats->addItem(format.displayName.isEmpty() ? format.interfaceName : format.displayName);
    }
    if (available.isEmpty()) return {};
    form->addRow(label("Emulation.Storage.Floppy.ImageFormat"), formats);

    auto* preparations = new QComboBox(&options);
    form->addRow(preparations);
    auto* capacities = new QComboBox(&options);
    form->addRow(label("Emulation.Storage.Geometry.SizeProfile"), capacities);
    auto* customSize = new QLineEdit(&options);
    form->addRow(label("Emulation.Storage.HardDisk.Size"), customSize);
    auto* preallocate = new QCheckBox(label("Emulation.Storage.File.Preallocate"), &options);
    form->addRow(preallocate);
    auto* automatic = new QCheckBox(label("Emulation.Storage.Geometry.AutomaticProfile"), &options);
    automatic->setChecked(true);
    form->addRow(automatic);
    auto* cylinders = new QLineEdit(&options);
    auto* heads = new QLineEdit(&options);
    auto* sectors = new QLineEdit(&options);
    auto* sectorSize = new QComboBox(&options);
    form->addRow(label("Emulation.Storage.Geometry.Cylinders"), cylinders);
    form->addRow(label("Emulation.Storage.Geometry.Heads"), heads);
    form->addRow(label("Emulation.Storage.Geometry.Sectors"), sectors);
    form->addRow(label("Emulation.Storage.Geometry.BytesPerSector"), sectorSize);
    auto* capacity = new QLabel(&options);
    form->addRow(capacity);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &options);
    buttons->button(QDialogButtonBox::Ok)->setText(label("Common.Browse"));
    buttons->button(QDialogButtonBox::Cancel)->setText(label("Common.Cancel"));
    form->addRow(buttons);

    const auto selectedFormat = [&]() -> const HardDiskImageFormat& { return *available.at(formats->currentIndex()); };
    HardDiskImageRequest request;
    const auto refreshSize = [&] {
        const auto& format = selectedFormat();
        const bool fixed = format.fixedGeometry.has_value();
        const bool needsGeometry = fixed || format.supportsExplicitGeometry || !format.mediaEngineFormatId.isEmpty();
        const bool manual = needsGeometry && !fixed && !automatic->isChecked();
        const bool custom = !fixed && !manual && capacities->currentIndex() >= 0 && !capacities->currentData().isValid();
        form->setRowVisible(customSize, custom);
        form->setRowVisible(automatic, needsGeometry && !fixed);
        capacities->setEnabled(!fixed && !manual);
        for (auto* widget : {cylinders, heads, sectors}) {
            form->setRowVisible(widget, needsGeometry);
            widget->setReadOnly(!manual);
        }
        form->setRowVisible(sectorSize, needsGeometry);
        sectorSize->setEnabled(manual);
        qint64 size = capacities->currentData().toLongLong();
        if (custom) {
            bool valid = false;
            const auto input = locale.toDouble(customSize->text(), &valid);
            const auto bytes = std::round(input * Mebibyte);
            size = valid && bytes > 0 && bytes < static_cast<double>(std::numeric_limits<qint64>::max())
                ? static_cast<qint64>(bytes) : 0;
        }
        request.geometry = format.fixedGeometry;
        if (manual) {
            request.geometry = HardDiskImageGeometry{cylinders->text().toLongLong(), heads->text().toInt(),
                sectors->text().toInt(), sectorSize->currentData().toInt()};
        } else if (!fixed && needsGeometry) {
            const int bytes = sectorSize->currentData().toInt();
            HardDiskImageGeometry geometry{1, format.preferredHeads, format.preferredSectorsPerTrack, bytes};
            if (const auto perCylinder = StorageRules::hardDiskCapacity(geometry)) {
                geometry.cylinders = size / *perCylinder + (size % *perCylinder != 0);
                request.geometry = geometry;
            }
        }
        if (!manual && request.geometry) {
            const QSignalBlocker blockCylinders(cylinders), blockHeads(heads), blockSectors(sectors), blockSectorSize(sectorSize);
            cylinders->setText(QString::number(request.geometry->cylinders));
            heads->setText(QString::number(request.geometry->heads));
            sectors->setText(QString::number(request.geometry->sectorsPerTrack));
            sectorSize->setCurrentIndex(sectorSize->findData(request.geometry->sectorSize));
        }
        request.capacity = request.geometry ? StorageRules::hardDiskCapacity(*request.geometry).value_or(0) : size;
        request.preallocate = preallocate->isEnabled() && preallocate->isChecked();
        request.preparationId = preparations->currentData().toString();
        auto extension = format.extension;
        if (!extension.startsWith(u'.')) extension.prepend(u'.');
        const bool valid = StorageRules::validHardDiskRequest(QString::fromLatin1(BlankImageName) + extension, format, request);
        auto message = label(valid ? "Emulation.Storage.Geometry.CalculatedCapacity" : "Emulation.Storage.Disk.InvalidSize");
        capacity->setText(message.replace(QLatin1String("{0}"), locale.formattedDataSize(request.capacity)));
        buttons->button(QDialogButtonBox::Ok)->setEnabled(valid);
    };

    const auto refreshFormat = [&] {
        const auto& format = selectedFormat();
        const QSignalBlocker blockCapacity(capacities), blockSectorSize(sectorSize), blockPreparation(preparations);
        capacities->clear();
        QList<qint64> choices;
        if (format.fixedGeometry) {
            choices.append(StorageRules::hardDiskCapacity(*format.fixedGeometry).value_or(0));
        } else if (format.allowedCapacities) {
            choices = *format.allowedCapacities;
        } else {
            for (const auto mib : SizePresets) choices.append(mib * Mebibyte);
            choices.append(format.defaultCapacity);
        }
        std::ranges::sort(choices);
        choices.erase(std::unique(choices.begin(), choices.end()), choices.end());
        for (const auto size : choices)
            if (StorageRules::supportsHardDiskCapacity(size, format))
                capacities->addItem(locale.formattedDataSize(size), size);
        if (!format.fixedGeometry && !format.allowedCapacities)
            capacities->addItem(label("Emulation.Storage.Geometry.CustomSize"));
        const int preferred = capacities->findData(format.defaultCapacity);
        if (preferred >= 0) capacities->setCurrentIndex(preferred);

        sectorSize->clear();
        auto sectorSizes = format.logicalSectorSizes;
        std::ranges::sort(sectorSizes);
        for (const int bytes : sectorSizes)
            if (bytes > 0) sectorSize->addItem(locale.toString(bytes), bytes);
        const int preferredSector = sectorSize->findData(DefaultHardDiskSectorSize);
        if (preferredSector >= 0) sectorSize->setCurrentIndex(preferredSector);

        preparations->clear();
        preparations->addItem(label("Emulation.Storage.Media.Blank"));
        for (const auto& preparation : format.preparations) {
            if (preparation.choice.id.isEmpty()) continue;
            preparations->addItem(preparation.choice.invariantDisplayValue.isEmpty()
                ? localize_(preparation.choice.displayResourceKey) : preparation.choice.invariantDisplayValue,
                preparation.choice.id);
        }
        form->setRowVisible(preparations, preparations->count() > 1);
        preallocate->setEnabled(format.supportsFixedAllocation);
        refreshSize();
    };
    connect(formats, &QComboBox::currentIndexChanged, &options, refreshFormat);
    for (auto* widget : {capacities, sectorSize, preparations})
        connect(widget, &QComboBox::currentIndexChanged, &options, refreshSize);
    for (auto* widget : {customSize, cylinders, heads, sectors})
        connect(widget, &QLineEdit::textChanged, &options, refreshSize);
    connect(automatic, &QCheckBox::toggled, &options, refreshSize);
    connect(preallocate, &QCheckBox::toggled, &options, refreshSize);
    connect(buttons, &QDialogButtonBox::accepted, &options, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &options, &QDialog::reject);
    refreshFormat();
    if (options.exec() != QDialog::Accepted) return {};

    const auto& format = selectedFormat();
    auto extension = format.extension;
    if (!extension.startsWith(u'.')) extension.prepend(u'.');
    QFileDialog destination(&parent, title);
    destination.setAcceptMode(QFileDialog::AcceptSave);
    destination.setFileMode(QFileDialog::AnyFile);
    destination.setOption(QFileDialog::DontConfirmOverwrite);
    destination.setDefaultSuffix(extension.mid(1));
    destination.setNameFilter(QString::fromLatin1(FileExtensionFilter).arg(extension.mid(1).toUpper(), extension));
    RememberedFileDialog::prepare(destination, workspace_, scope, directory);
    RememberedFileDialog::remember(destination, workspace_, scope);
    const auto name = device.defaultImageFileName.isEmpty() ? QString::fromLatin1(BlankImageName) : device.defaultImageFileName;
    const auto baseName = QFileInfo(name).completeBaseName();
    destination.selectFile(baseName + extension);
    if (destination.exec() != QDialog::Accepted) return {};
    const auto path = destination.selectedFiles().value(0);
    if (!StorageRules::validHardDiskRequest(path, format, request)) {
        QMessageBox::warning(&parent, title, label("Emulation.Storage.Disk.InvalidSize"));
        return {};
    }
    const StorageMount candidate{{device.slot, path}, true};
    const auto canAttach = StorageRules::validMount(candidate, settings_.availableDevices, settings_.configuredSlots);
    return confirmImageCreation(parent, translations_, title, path,
        [&](std::stop_token token) { workspace_.emulation->createHardDisk(path, format, request, token); }, canAttach)
        ? path : QString{};
}

bool EmulationStoragePanel::confirmImageCreation(QWidget& parent, const Translations::TranslationService& translations,
    const QString& title, const QString& path,
        const std::function<void(std::stop_token)>& write, bool canAttach) {
    const auto label = [&translations](const char* key) { return QString::fromStdString(translations.text(key)); };
    QDialog creation(&parent);
    creation.setWindowTitle(title);
    auto* layout = new QVBoxLayout(&creation);
    auto* file = new QLineEdit(path, &creation);
    file->setReadOnly(true);
    layout->addWidget(file);
    auto* status = new QLabel(&creation);
    status->setTextFormat(Qt::PlainText);
    status->setWordWrap(true);
    layout->addWidget(status);
    auto* actions = new QDialogButtonBox(QDialogButtonBox::Close, &creation);
    actions->button(QDialogButtonBox::Close)->setText(label("Preferences.Close"));
    auto* create = actions->addButton(label("Common.Create"), QDialogButtonBox::ActionRole);
    auto* mount = actions->addButton(label("Emulation.Storage.Media.CreateAndMount"), QDialogButtonBox::ActionRole);
    auto* progress = new QProgressBar(&creation);
    progress->setRange(0, 0);
    progress->hide();
    layout->addWidget(progress);
    bool created = false;
    std::jthread worker;
    const auto createImage = [&](bool attach) {
        if (created) {
            if (attach) creation.accept();
            return;
        }
        create->setEnabled(false);
        mount->setEnabled(false);
        progress->show();
        status->clear();
        worker = std::jthread([&, attach](std::stop_token token) {
            bool success = false;
            try {
                write(token);
                success = true;
            } catch (...) {
                // Report through the same localized dialog for every image writer.
            }
            if (token.stop_requested()) return;
            QMetaObject::invokeMethod(&creation, [&, attach, success] {
                created = success;
                progress->hide();
                create->setEnabled(!created);
                mount->setEnabled(canAttach);
                const QFileInfo target(path);
                const auto key = created ? "Emulation.Storage.Media.Created" :
                    (target.exists() || target.isSymLink() ? "Emulation.Storage.Media.AlreadyExists" :
                        "Emulation.Error.EmulatorRejected");
                auto message = label(key);
                status->setText(message.replace(QLatin1String("{0}"), path));
                if (created && attach) creation.accept();
            }, Qt::QueuedConnection);
        });
    };
    mount->setEnabled(canAttach);
    connect(create, &QPushButton::clicked, &creation, [&] { createImage(false); });
    connect(mount, &QPushButton::clicked, &creation, [&] { createImage(true); });
    connect(&creation, &QDialog::finished, &creation, [&] { worker.request_stop(); });
    connect(actions, &QDialogButtonBox::rejected, &creation, &QDialog::reject);
    layout->addWidget(actions);
    return creation.exec() == QDialog::Accepted;
}

void EmulationStoragePanel::addMemoryCardPathControls(QDialog& dialog, QFormLayout& form, QLineEdit& pathWidget,
    const StorageDevice& device, WorkspaceState& workspace, const Translations::TranslationService& translations,
    const QString& scope, const Localize& localize) {
    const auto label = [&translations](const char* key) { return QString::fromStdString(translations.text(key)); };
    const auto resource = [&](const QString& key, const char* fallback) { return key.isEmpty() ? label(fallback) : localize(key); };
    const auto presentation = device.dialogPresentation.value_or(StorageDialogPresentation{});
    auto directory = device.imageDirectory;
    if (directory.isEmpty()) {
        directory = resolveEmulationFolders(workspace.emulationFolders()).storage;
        if (!device.defaultImageSubdirectory.isEmpty()) directory = QDir(directory).filePath(device.defaultImageSubdirectory);
    }
    auto* pathRow = new QWidget(&dialog);
    auto* pathLayout = new QHBoxLayout(pathRow);
    pathLayout->setContentsMargins({});
    pathLayout->addWidget(&pathWidget, 1);
    auto* browse = new QPushButton(resource(presentation.useExistingResourceKey, "Emulation.Storage.MemoryCard.Use"), pathRow);
    auto* clear = new QPushButton(label("Emulation.Storage.Media.Remove"), pathRow);
    pathLayout->addWidget(browse);
    pathLayout->addWidget(clear);
    form.addRow(resource(presentation.associatedResourceKey, "Emulation.Storage.Media.Associated"), pathRow);
    connect(browse, &QPushButton::clicked, &dialog, [&, directory, scope] {
        QFileDialog picker(&dialog, dialog.windowTitle());
        picker.setFileMode(device.usesDirectory ? QFileDialog::Directory : QFileDialog::ExistingFile);
        if (device.usesDirectory) {
            picker.setOption(QFileDialog::ShowDirsOnly);
        } else {
            QStringList filters;
            for (auto extension : device.slot.acceptedExtensions) {
                if (!extension.startsWith(u'.')) extension.prepend(u'.');
                filters.append(QString::fromLatin1(FileExtensionFilter).arg(extension.mid(1).toUpper(), extension));
            }
            picker.setNameFilters(filters);
        }
        RememberedFileDialog::prepare(picker, workspace, scope, directory);
        RememberedFileDialog::remember(picker, workspace, scope);
        if (picker.exec() == QDialog::Accepted) pathWidget.setText(picker.selectedFiles().value(0));
    });
    connect(clear, &QPushButton::clicked, &pathWidget, &QLineEdit::clear);
    if (!device.usesDirectory && device.fixedImageSizeBytes.value_or(0) > 0 &&
        !device.slot.acceptedExtensions.isEmpty()) {
        const auto title = resource(presentation.createResourceKey, "Common.Create");
        auto* create = new QPushButton(title, pathRow);
        pathLayout->insertWidget(pathLayout->count() - 1, create);
        connect(create, &QPushButton::clicked, &dialog, [&, label, title, directory, scope] {
            QFileDialog destination(&dialog, title);
            destination.setAcceptMode(QFileDialog::AcceptSave);
            destination.setFileMode(QFileDialog::AnyFile);
            destination.setOption(QFileDialog::DontConfirmOverwrite);
            QStringList filters;
            for (auto extension : device.slot.acceptedExtensions) {
                if (!extension.startsWith(u'.')) extension.prepend(u'.');
                filters.append(QString::fromLatin1(FileExtensionFilter).arg(extension.mid(1).toUpper(), extension));
            }
            auto suffix = device.slot.acceptedExtensions.front();
            if (suffix.startsWith(u'.')) suffix.remove(0, 1);
            destination.setDefaultSuffix(suffix);
            destination.setNameFilters(filters);
            RememberedFileDialog::prepare(destination, workspace, scope, directory);
            RememberedFileDialog::remember(destination, workspace, scope);
            auto name = device.defaultImageFileName;
            if (name.isEmpty()) {
                name = device.slot.id;
                while (name.endsWith(u':')) name.chop(1);
            }
            destination.selectFile(name);
            if (destination.exec() != QDialog::Accepted) return;
            const auto path = destination.selectedFiles().value(0);
            if (!StorageRules::acceptsCardFile(device, path)) {
                QMessageBox::warning(&dialog, title, label("Emulation.Storage.MemoryCard.InvalidFormat"));
                return;
            }
            if (confirmImageCreation(dialog, translations, title, path,
                [&](std::stop_token token) { workspace.emulation->createBlankMemoryCard(path, device, token); }, true))
                pathWidget.setText(path);
        });
    }
}

std::optional<QString> EmulationStoragePanel::selectMemoryCard(QWidget& parent, const StorageDevice& device,
    QString title, const QString& current, WorkspaceState& workspace, const Translations::TranslationService& translations,
    const QString& scope, const Localize& localize) {
    QDialog dialog(&parent);
    dialog.setWindowTitle(std::move(title));
    auto* form = new QFormLayout(&dialog);
    auto* path = new QLineEdit(current, &dialog);
    path->setReadOnly(true);
    addMemoryCardPathControls(dialog, *form, *path, device, workspace, translations, scope, localize);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    buttons->button(QDialogButtonBox::Save)->setText(QString::fromStdString(translations.text("Common.Save")));
    buttons->button(QDialogButtonBox::Cancel)->setText(QString::fromStdString(translations.text("Common.Cancel")));
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, [&] {
        if (const auto* key = memoryCardError(StorageRules::validateMemoryCardPath(device, path->text()))) {
            QMessageBox::warning(&dialog, dialog.windowTitle(), QString::fromStdString(translations.text(key)));
            return;
        }
        dialog.accept();
    });
    if (dialog.exec() != QDialog::Accepted) return std::nullopt;
    return path->text();
}

void EmulationStoragePanel::configureDevice() {
    const auto* device = currentDevice();
    if (!device) return;
    QDialog dialog(this);
    dialog.setWindowTitle(deviceLabel(*device));
    auto* form = new QFormLayout(&dialog);
    auto found = std::ranges::find_if(settings_.deviceSettings,
        [&](const auto& entry) { return sameSlot(entry.slot, device->slot); });
    auto entry = found == settings_.deviceSettings.end() ? StorageDeviceSettings{device->slot} : *found;
    const auto mount = std::ranges::find_if(settings_.mountedMedia,
        [&](const auto& value) { return sameSlot(value.media.slot, device->slot); });
    auto path = associatedPath(*device);
    const auto previousPath = path;
    QString createdFloppy;
    const bool pathConfigurable = device->configurationKind == StorageConfigurationKind::HardDiskDrive ||
        device->configurationKind == StorageConfigurationKind::MemoryCard;
    auto* pathWidget = new QLineEdit(path, &dialog);
    pathWidget->setReadOnly(true);
    auto* readOnly = new QCheckBox(label("Emulation.Storage.ReadOnly"), &dialog);
    readOnly->setChecked(mount != settings_.mountedMedia.end() && mount->media.readOnly);
    if (device->slot.kind == MediaKind::MemoryCard) {
        addMemoryCardPathControls(dialog, *form, *pathWidget, *device, workspace_, translations_,
            scope_ + u'/' + device->slot.id, localize_);
    } else if (pathConfigurable) {
        auto* pathRow = new QWidget(&dialog);
        auto* pathLayout = new QHBoxLayout(pathRow);
        pathLayout->setContentsMargins(0, 0, 0, 0);
        pathLayout->addWidget(pathWidget, 1);
        const auto resource = [&](const QString& key, const char* fallback) {
            return key.isEmpty() ? label(fallback) : localize_(key);
        };
        const auto presentation = device->dialogPresentation.value_or(StorageDialogPresentation{});
        auto* browse = new QPushButton(resource(presentation.useExistingResourceKey,
            "Emulation.Storage.Disk.UseExisting"), pathRow);
        auto* clear = new QPushButton(label("Emulation.Storage.Media.Remove"), pathRow);
        pathLayout->addWidget(browse);
        pathLayout->addWidget(clear);
        form->addRow(resource(presentation.associatedResourceKey, "Emulation.Storage.Media.Associated"), pathRow);
        if (device->slot.kind == MediaKind::HardDisk) form->addRow(readOnly);
        auto directory = device->imageDirectory.isEmpty() ? resolveEmulationFolders(workspace_.emulationFolders()).storage : device->imageDirectory;
        if (!device->defaultImageSubdirectory.isEmpty() && device->imageDirectory.isEmpty())
            directory = QDir(directory).filePath(device->defaultImageSubdirectory);
        const auto scope = scope_ + u'/' + device->slot.id;
        connect(browse, &QPushButton::clicked, &dialog, [&, directory, scope] {
            const auto selected = RememberedFileDialog::choose(&dialog, workspace_, deviceLabel(*device),
                scope, directory, device->usesDirectory ? QFileDialog::Directory : QFileDialog::ExistingFile);
            if (!selected.isEmpty()) pathWidget->setText(selected);
        });
        if (device->slot.kind == MediaKind::HardDisk &&
            std::ranges::any_of(device->hardDiskFormats, [](const auto& format) { return format.canCreate; })) {
            auto* create = new QPushButton(resource(presentation.createResourceKey, "Emulation.Storage.HardDisk.Create"), pathRow);
            pathLayout->insertWidget(pathLayout->count() - 1, create);
            connect(create, &QPushButton::clicked, &dialog, [&, directory, scope] {
                const auto selected = createHardDisk(dialog, *device, directory, scope);
                if (!selected.isEmpty()) pathWidget->setText(selected);
            });
        }
        connect(clear, &QPushButton::clicked, pathWidget, &QLineEdit::clear);
    } else {
        pathWidget->hide();
        const auto hint = device->optionDescriptionResourceKey.isEmpty()
            ? label("Emulation.Storage.Media.RemovableRuntimeHint") : localize_(device->optionDescriptionResourceKey);
        auto* message = new QLabel(hint, &dialog);
        message->setTextFormat(Qt::PlainText);
        message->setWordWrap(true);
        form->addRow(message);
    }
    if (!pathConfigurable || device->slot.kind != MediaKind::HardDisk) readOnly->hide();
    auto* interfaceBox = new QComboBox(&dialog);
    for (const auto& option : device->interfaceChoices)
        interfaceBox->addItem(option.invariantDisplayValue.isEmpty() ? localize_(option.displayResourceKey) :
            option.invariantDisplayValue, option.id);
    interfaceBox->setCurrentIndex(interfaceBox->findData(entry.interfaceId));
    if (interfaceBox->count() > 0) form->addRow(label("Emulation.Storage.Device.Interface"), interfaceBox);
    else interfaceBox->hide();

    const auto addToggle = [&](const QString& caption, bool checked) {
        auto* control = new QCheckBox(caption, &dialog);
        control->setChecked(checked);
        form->addRow(control);
        return control;
    };
    const auto createChoice = [&](const QList<SettingsChoice>& choices, const QString& value, const QString& unit = {}) {
        auto* control = new QComboBox(&dialog);
        for (const auto& choice : choices) control->addItem(moduleChoiceDisplay(choice, unit, localize_), choice.id);
        const int index = control->findData(value);
        if (index >= 0) control->setCurrentIndex(index);
        return control;
    };
    const auto addChoice = [&](const QString& caption, const QList<SettingsChoice>& choices, const QString& value) {
        auto* control = createChoice(choices, value);
        form->addRow(caption, control);
        return control;
    };
    std::function<void()> readDevice;
    auto* options = &entry.options;
    if (device->configurationKind == StorageConfigurationKind::FloppyDrive && device->floppyOptions) {
        const auto& capabilities = *device->floppyOptions;
        if (!entry.floppy) entry.floppy = FloppyDriveSettings{{}, StorageRules::standardDriveSpeedChoices().constFirst().id};
        auto& settings = *entry.floppy;
        options = &settings.options;
        QList<SettingsChoice> models;
        for (const auto& model : capabilities.models) models.append(model.choice);
        auto* model = models.isEmpty() ? nullptr : addChoice(label("Emulation.Storage.Floppy.Drive"), models, settings.model);
        const auto& speeds = capabilities.speedChoices ? *capabilities.speedChoices : StorageRules::standardDriveSpeedChoices();
        auto* speed = capabilities.canConfigureBehavior && !speeds.isEmpty()
            ? new ModuleRadioChoices(speeds, settings.speed, {}, localize_, &dialog) : nullptr;
        if (speed) form->addRow(label("Emulation.Storage.Floppy.Speed"), speed);
        auto* writeProtected = capabilities.canConfigureBehavior && capabilities.canConfigureWriteProtection
            ? addToggle(label("Emulation.Storage.Floppy.WriteProtection"), settings.writeProtected) : nullptr;
        auto* redirect = capabilities.canConfigureBehavior && capabilities.canRedirectWrites
            ? addToggle(label("Emulation.Storage.Floppy.WriteRedirect"), settings.redirectWrites) : nullptr;
        if (capabilities.canCreateBlankMedia) {
            auto* create = new QPushButton(label("Emulation.Storage.Floppy.CreateBlank"), &dialog);
            form->addRow(create);
            connect(create, &QPushButton::clicked, &dialog, [&, model] {
                const auto selected = createBlankFloppy(dialog, *device, model ? model->currentData().toString() : settings.model);
                if (selected.isEmpty()) return;
                createdFloppy = selected;
                create->setToolTip(selected);
            });
        }
        readDevice = [&, model, speed, writeProtected, redirect] {
            if (model) settings.model = model->currentData().toString();
            if (speed) settings.speed = speed->value();
            if (writeProtected) settings.writeProtected = writeProtected->isChecked();
            if (redirect) settings.redirectWrites = redirect->isChecked();
        };
    } else if (device->configurationKind == StorageConfigurationKind::CompactDiscDrive) {
        if (!entry.compactDisc) entry.compactDisc = CompactDiscDriveSettings{};
        auto& settings = *entry.compactDisc;
        options = &settings.options;
        QList<SettingsChoice> speeds;
        for (const auto& speed : device->compactDiscSpeeds) speeds.append({speed, {}, speed});
        auto* speed = speeds.isEmpty() ? nullptr : addChoice(label("Emulation.Storage.Cd.Speed"), speeds, settings.speed);
        auto* cache = device->compactDiscCacheResourceKey.isEmpty() ? nullptr :
            addToggle(localize_(device->compactDiscCacheResourceKey), settings.cacheImage);
        auto* ignore = device->compactDiscIgnoreErrorsResourceKey.isEmpty() ? nullptr :
            addToggle(localize_(device->compactDiscIgnoreErrorsResourceKey), settings.ignoreErrors);
        readDevice = [&, speed, cache, ignore] {
            if (speed) settings.speed = speed->currentData().toString();
            if (cache) settings.cacheImage = cache->isChecked();
            if (ignore) settings.ignoreErrors = ignore->isChecked();
        };
    } else if (device->configurationKind == StorageConfigurationKind::CartridgeSlot) {
        if (!entry.cartridge) entry.cartridge = CartridgeSlotSettings{};
        auto& settings = *entry.cartridge;
        options = &settings.options;
        if (!device->optionResourceKey.isEmpty()) {
            auto* enabled = addToggle(localize_(device->optionResourceKey), settings.optionEnabled);
            readDevice = [&, enabled] { settings.optionEnabled = enabled->isChecked(); };
        }
    }

    QList<std::pair<QString, std::function<QString()>>> values;
    const auto addField = [&](const SettingsField& field) {
        if (!field.visible) return;
        const auto caption = localize_(field.labelResourceKey);
        const auto initial = options->value(field.id, field.value);
        const auto detailed = detailedHelpResourceKeys_.value(field.id);
        const auto presentation = choicePresentation_.value(field.id);
        const auto addRow = [&](QWidget* control) {
            addModuleSettingRow(*form, control, caption,
                field.explanationResourceKey.isEmpty() ? QString{} : localize_(field.explanationResourceKey),
                detailed.isEmpty() ? QString{} : localize_(detailed));
        };
        if (field.editor == SettingsEditor::Information) {
            auto current = field;
            current.value = initial;
            auto* control = new QLabel(moduleFieldDisplay(current, presentation.unitResourceKey, localize_), &dialog);
            control->setTextFormat(Qt::PlainText);
            control->setWordWrap(true);
            addRow(control);
        } else if (field.editor == SettingsEditor::Toggle) {
            auto* control = new QCheckBox(&dialog);
            control->setChecked(initial == field.enabledValue);
            control->setEnabled(field.enabled);
            addRow(control);
            if (field.enabled) values.append({field.id, [control, field] {
                return control->isChecked() ? field.enabledValue : field.disabledValue;
            }});
        } else if (!field.choices.isEmpty() && (presentation.useRadioButtons ||
            (field.editor == SettingsEditor::Choice && field.choices.size() == 2))) {
            auto* control = new ModuleRadioChoices(field.choices, initial, presentation.unitResourceKey, localize_, &dialog);
            control->setEnabled(field.enabled);
            addRow(control);
            if (field.enabled) values.append({field.id, [control] { return control->value(); }});
        } else if (!field.choices.isEmpty()) {
            auto* control = createChoice(field.choices, initial, presentation.unitResourceKey);
            control->setEnabled(field.enabled);
            addRow(control);
            if (field.enabled) values.append({field.id, [control] { return control->currentData().toString(); }});
        } else {
            auto* control = new QLineEdit(initial, &dialog);
            control->setReadOnly(!field.enabled);
            addRow(control);
            if (!control->isReadOnly()) values.append({field.id, [control] { return control->text(); }});
        }
    };
    const auto fields = device->describeCartridgeOptions ? device->describeCartridgeOptions(*options) : device->configurationFields;
    for (const auto& field : fields) addField(field);
    for (const auto& block : device->configurationBlocks)
        if (block.visible)
            for (const auto& field : block.fields) addField(field);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    buttons->button(QDialogButtonBox::Save)->setText(label("Common.Save"));
    buttons->button(QDialogButtonBox::Cancel)->setText(label("Common.Cancel"));
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    form->addRow(buttons);
    if (dialog.exec() != QDialog::Accepted) return;
    if (readDevice) readDevice();
    for (const auto& [id, value] : values) options->insert(id, value());
    if (interfaceBox->currentIndex() >= 0) entry.interfaceId = interfaceBox->currentData().toString();
    if (pathConfigurable) {
        path = pathWidget->text();
        if (device->slot.kind == MediaKind::MemoryCard) {
            if (const auto* error = memoryCardError(StorageRules::validateMemoryCardPath(*device, path))) {
                status_->setText(label(error));
                return;
            }
            entry.memoryCardPath = path;
        } else {
            StorageMount replacement{{device->slot, path, readOnly->isChecked()}, true};
            if (mount != settings_.mountedMedia.end()) {
                replacement.media.parameters = mount->media.parameters;
                if (path == previousPath) replacement.inserted = mount->inserted;
            }
            if (!path.isEmpty() && !StorageRules::validMount(replacement, settings_.availableDevices, settings_.configuredSlots)) {
                status_->setText(label("Emulation.Storage.Disk.ImageRequired"));
                return;
            }
            if (mount != settings_.mountedMedia.end()) settings_.mountedMedia.erase(mount);
            if (!path.isEmpty()) settings_.mountedMedia.append(std::move(replacement));
        }
    }
    if (!createdFloppy.isEmpty()) {
        if (mount != settings_.mountedMedia.end()) settings_.mountedMedia.erase(mount);
        settings_.mountedMedia.append(StorageMount{{device->slot, createdFloppy}, true});
    }
    if (found != settings_.deviceSettings.end()) *found = std::move(entry);
    else settings_.deviceSettings.append(std::move(entry));
    changed_ = true;
    rebuild(&device->slot);
    if (onChanged_) onChanged_();
}
}
