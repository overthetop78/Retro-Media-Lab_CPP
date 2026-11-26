#include "Shared/Presentation/AssetPath.h"
#include "Shared/Controls/RememberedFileDialog.h"
#include "ExplorerPage.h"
#include "Media/VisualizationSession.h"
#include "MediaLoadingOverlay.h"
#include "LoadingConstants.h"
#include "ClassificationPanel.h"
#include "ClassificationConstants.h"
#include <QToolButton>
#include "Media/ExplorerReadSession.h"
#include "MediaPageConstants.h"
#include "../DisplayDensity.h"
#include "Shared/Controls/Card.h"
#include "Shared/Localization/MediaText.h"
#include <Errors.h>
#include <RetroMediaLab/Translations/TranslationConstants.h>
#include <QCheckBox>
#include <QCollator>
#include <QComboBox>
#include <QGridLayout>
#include <QFormLayout>
#include <QMenu>
#include <QWidgetAction>
#include "../ThemeIcons.h"
#include "../ThemeConstants.h"
#include <QCoreApplication>
#include <QMessageBox>
#include <QPainter>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSplitter>
#include <QResizeEvent>
#include <QStyledItemDelegate>
#include <QStyle>
#include <QScrollArea>
#include <QSet>
#include <QTableWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <algorithm>
#include <set>
#include <tuple>

namespace RetroMediaLab {
using namespace MediaPageConstants;
using MediaEngine::FileSystemEntryInfo;
using MediaEngine::FileSystemEntryKind;
namespace {
QIcon fileIcon(const QString& name) {
    const auto path = assetPath(QString::fromLatin1(ResourceIconPath));
    const auto resource = path.arg(name);
    return QFile::exists(resource) ? QIcon(resource) : QIcon(path.arg(QString::fromLatin1(FileIcon)));
}

QIcon entryIcon(const FileSystemEntryInfo& entry, const QColor& executableColor) {
    if (entry.kind == FileSystemEntryKind::Directory) return fileIcon(QString::fromLatin1(FolderIcon));
    if (!entry.analysis || entry.analysis->iconId.empty()) return fileIcon(QString::fromLatin1(FileIcon));
    auto icon = QString::fromStdString(entry.analysis->iconId);
    for (const auto& [id, asset] : FileIconAliases) {
        if (icon == QLatin1String(id)) return fileIcon(QString::fromLatin1(asset));
    }
    auto result = fileIcon(icon);
    if (entry.analysis->categoryId == ExecutableCategory) {
        auto canvas = result.pixmap(DetailIconSize);
        QPainter painter(&canvas);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(canvas.rect(), executableColor);
        painter.end();
        result = QIcon(canvas);
    }
    return result;
}

class ExplorerTable final : public QTableWidget {
public:
    using QTableWidget::QTableWidget;

protected:
    bool viewportEvent(QEvent* event) override {
        const bool handled = QTableWidget::viewportEvent(event);
        if (event->type() == QEvent::Resize && columnCount() == ColumnCount) {
            const auto width = static_cast<QResizeEvent*>(event)->size().width();
            setColumnWidth(ModifiedColumn, qRound(width * ModifiedColumnShare));
            setColumnWidth(TypeColumn, qRound(width * TypeColumnShare));
            setColumnWidth(SizeColumn, qRound(width * SizeColumnShare));
        }
        return handled;
    }
};

class ExplorerCellDelegate final : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        const auto* widget = option.widget;
        const auto margin = qMin(qRound(CellHorizontalInset * DisplayDensity::factor(widget)),
                                 qMax(0, option.rect.width() / 2 - 1));
        if (option.state & QStyle::State_Selected) {
            auto background = option;
            initStyleOption(&background, index);
            widget->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &background, painter, widget);
        }
        auto padded = option;
        padded.rect.adjust(margin, 0, -margin, 0);
        QStyledItemDelegate::paint(painter, padded, index);
    }
};

bool isDirectory(const FileSystemEntryInfo& entry) {
    return entry.kind == FileSystemEntryKind::Directory;
}

QString extension(const FileSystemEntryInfo& entry) {
    const std::string_view name = entry.name;
    const auto separator = name.rfind(ExtensionSeparator);
    return separator == std::string_view::npos || separator == 0
        ? QString{} : QString::fromUtf8(name.substr(separator + 1));
}

auto dateKey(const std::optional<MediaEngine::FileTimestamp>& date) {
    const auto value = date.value_or(MediaEngine::FileTimestamp{});
    return std::tuple(date.has_value(), value.year, value.month, value.day,
                      value.hour, value.minute, value.second, value.nanosecond);
}

}
ExplorerPage::ExplorerPage(ThemeService& themes, Translations::TranslationService& translations, WorkspaceState& state, QWidget* parent)
    : QWidget(parent), themes_(themes), translations_(translations) {
    state.ensureMediaOperations(translations);
    reading_=state.explorerReading;visualization_=state.visualization;
    if (!state.explorer) state.explorer = std::make_shared<Infrastructure::ExplorerSession>();
    session_ = state.explorer;
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(Padding, Padding, Padding, Padding);
    layout->setSpacing(Gap);
    auto* header = new Card(themes.data(), this);
    auto* headerGrid = new QGridLayout(header);
    auto* identity = new QHBoxLayout;
    mediaIcon_ = new QLabel(header);
    identity->addWidget(mediaIcon_);
    auto* identityText = new QVBoxLayout;
    status_ = new QLabel(header);
    status_->setTextFormat(Qt::PlainText);
    status_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    auto titleFont = status_->font();
    titleFont.setBold(true);
    status_->setFont(titleFont);
    mediaSummary_ = new QLabel(header);
    mediaSummary_->setTextFormat(Qt::PlainText);
    auto summaryPolicy = QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    summaryPolicy.setRetainSizeWhenHidden(true);
    mediaSummary_->setSizePolicy(summaryPolicy);
    identityText->addWidget(status_);
    identityText->addWidget(mediaSummary_);
    identity->addLayout(identityText, 1);
    headerGrid->addLayout(identity, 0, 0);
    headerGrid->setColumnStretch(0, 1);

    auto* actions = new QHBoxLayout;
    read_ = new QPushButton(header);
    open_ = new QPushButton(header);
        classificationToggle_ = new QToolButton(header);
    classificationToggle_->setObjectName(ClassificationConstants::ToggleObject);
    classificationToggle_->setPopupMode(QToolButton::InstantPopup);
    actions->addWidget(read_);
    actions->addWidget(open_);
    actions->addWidget(classificationToggle_);
    headerGrid->addLayout(actions, 0, 1, Qt::AlignRight);

    auto* choices = new QHBoxLayout;
    formats_ = new QComboBox(header);
    sessions_ = new QComboBox(header);
    volumes_ = new QComboBox(header);
    volumes_->setMinimumWidth(0);
    presentationLabel_ = new QLabel(header);
    choices->addWidget(presentationLabel_);
    choices->addWidget(formats_);
    choices->addWidget(sessions_);
    choices->addWidget(volumes_);
    choices->addStretch();
    headerGrid->addLayout(choices, 1, 0);
    automatic_ = new QCheckBox(header);
    headerGrid->addWidget(automatic_, 1, 1, Qt::AlignRight);
    connect(automatic_, &QCheckBox::toggled, this, [this](bool automatic) {
        session_->classification.setAutomatic(automatic);
        session_->reclassify(true);
    });
    layout->addWidget(header);

    auto* classificationMenu = new QMenu(classificationToggle_);
    auto* classificationAction = new QWidgetAction(classificationMenu);
    classification_ = new ClassificationPanel(session_->classification, translations, false,
        [this](bool automaticChanged) { session_->reclassify(automaticChanged); }, classificationMenu);
    classificationAction->setDefaultWidget(classification_);
    classificationMenu->addAction(classificationAction);
    classificationToggle_->setMenu(classificationMenu);
    connect(classificationMenu, &QMenu::aboutToShow, classification_, [this] { classification_->refresh(); });
    auto* summary = new Card(themes.data(), this);
    auto* summaryGrid = new QGridLayout(summary);
    for (std::size_t i = 0; i < summaryValues_.size(); ++i) {
        summaryLabels_[i] = new QLabel(summary);
        summaryValues_[i] = new QLabel(summary);
        summaryValues_[i]->setTextFormat(Qt::PlainText);
        summaryValues_[i]->setWordWrap(true);
        summaryGrid->addWidget(summaryLabels_[i], 0, static_cast<int>(i));
        summaryGrid->addWidget(summaryValues_[i], 1, static_cast<int>(i));
        summaryGrid->setColumnStretch(static_cast<int>(i), 1);
    }
    layout->addWidget(summary);
    readStatus_=new QLabel(this);readStatus_->setWordWrap(true);readStatus_->setTextFormat(Qt::PlainText);layout->addWidget(readStatus_);
    auto* panes = new QWidget(this);
    auto* panesLayout = new QVBoxLayout(panes);
    panesLayout->setContentsMargins(0, 0, 0, 0);
    auto* splitter = new QSplitter(panes);
    splitter->setChildrenCollapsible(false);
    auto* folderPane = new Card(themes.data(), splitter);
    auto* folderLayout = new QVBoxLayout(folderPane);
    folderLayout->setContentsMargins(Gap, Gap, Gap, Gap);
    folders_ = new QTreeWidget(folderPane);
    folders_->setFrameShape(QFrame::NoFrame);
    folderLayout->addWidget(folders_);
    folders_->setHeaderHidden(true);
    auto* contentsPane = new Card(themes.data(), splitter);
    auto* contentsLayout = new QVBoxLayout(contentsPane);
    contentsLayout->setContentsMargins(Gap, Gap, Gap, Gap);
    contents_ = new ExplorerTable(contentsPane);
    contents_->setItemDelegate(new ExplorerCellDelegate(contents_));
    contents_->setFrameShape(QFrame::NoFrame);
    contentsLayout->addWidget(contents_);
    contents_->setColumnCount(ColumnCount);
    contents_->setSelectionBehavior(QAbstractItemView::SelectRows);
    contents_->setSelectionMode(QAbstractItemView::SingleSelection);
    contents_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    contents_->verticalHeader()->hide();
    contents_->setShowGrid(false);
    auto* fileHeader = contents_->horizontalHeader();
    fileHeader->setSortIndicator(sortColumn_, sortOrder_);
    fileHeader->setSortIndicatorShown(true);
    fileHeader->setSectionsClickable(true);
    fileHeader->setSectionResizeMode(QHeaderView::Fixed);
    fileHeader->setSectionResizeMode(NameColumn, QHeaderView::Stretch);
    connect(fileHeader, &QHeaderView::sectionClicked, this, [this, fileHeader](int column) {
        sortOrder_ = column == sortColumn_ && sortOrder_ == Qt::AscendingOrder
            ? Qt::DescendingOrder : Qt::AscendingOrder;
        sortColumn_ = column;
        fileHeader->setSortIndicator(sortColumn_, sortOrder_);
        showFolder(session_->folderPath);
    });
    auto* detailScroll=new QScrollArea(splitter);detailScroll->setWidgetResizable(true);detailScroll->setFrameShape(QFrame::NoFrame);
    auto* card = new Card(themes.data(), detailScroll);detailScroll->setWidget(card);
    auto* detailLayout = new QVBoxLayout(card);
    auto* detailHeading = new QHBoxLayout;
    detailIcon_ = new QLabel(card);
    detailTitle_ = new QLabel(card);
    detailTitle_->setTextFormat(Qt::PlainText);
    detailTitle_->setWordWrap(true);
    detailHeading->addWidget(detailIcon_);
    detailHeading->addWidget(detailTitle_, 1);
    detailLayout->addLayout(detailHeading);
    auto* detailDivider = new QFrame(card);
    detailDivider->setFrameShape(QFrame::HLine);
    detailLayout->addWidget(detailDivider);
    detailRows_ = new QFormLayout;
    detailRows_->setRowWrapPolicy(QFormLayout::WrapLongRows);
    detailLayout->addLayout(detailRows_);
    detailWarnings_ = new QLabel(card);
    detailWarnings_->setTextFormat(Qt::PlainText);
    detailWarnings_->setWordWrap(true);
    detailWarnings_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    detailLayout->addWidget(detailWarnings_);
    detailScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    detailLayout->addStretch();
    for (auto* child : {static_cast<QWidget*>(folderPane), static_cast<QWidget*>(contentsPane), static_cast<QWidget*>(detailScroll)}) child->setMinimumWidth(0);
    splitter->setStretchFactor(0, PaneStretch);
    splitter->setStretchFactor(1, PaneStretch);
    splitter->setStretchFactor(DetailsPaneIndex, PaneStretch);
    splitter->setSizes({TreeInitialWidth, ContentsInitialWidth, DetailsInitialWidth});
    panesLayout->addWidget(splitter);
    layout->addWidget(panes, 1);
    connect(read_,&QPushButton::clicked,this,[this,&state]{
        if(reading_->busy()){if(QMessageBox::question(this,text(StopTitle),text(StopConfirm),QMessageBox::Yes|QMessageBox::No,QMessageBox::No)==QMessageBox::Yes)reading_->stop();return;}
        const auto id=state.selectedHardwareDeviceId.isEmpty()?state.reading->draft.deviceId:state.selectedHardwareDeviceId;
        const auto found=std::find_if(state.hardware().devices.begin(),state.hardware().devices.end(),[&](const auto& device){return device.id==id;});const auto label=found==state.hardware().devices.end()?text(Unknown):found->name;
        if(QMessageBox::question(this,text(ReadConfirmTitle),text(ReadConfirm).replace(QString::fromLatin1(Argument),label),QMessageBox::Yes|QMessageBox::No,QMessageBox::No)!=QMessageBox::Yes)return;
        try{reading_->start(state.hardware(),state.engines(),QCoreApplication::applicationDirPath(),id);}catch(const std::exception& error){readStatus_->setText(mediaErrorText(translations_,error));readStatus_->show();}
    });
    connect(open_, &QPushButton::clicked, this, [this, &state] {
        const auto path = RememberedFileDialog::choose(this,state,text(Open),FileDialogScope::Explorer,state.defaultImageDirectory());
        if (!path.isEmpty()) session_->open(path);
    });
    connect(volumes_, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index < 0) return;
        if (static_cast<std::size_t>(index) >= visibleVolumes_.size()) return;
        session_->selectedVolume = visibleVolumes_[index];
        session_->folderPath.clear();session_->selectedEntry.reset();
        rebuildTree();
    });
    connect(sessions_, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index < 0) return;
        session_->selectedSession = sessions_->itemData(index).toInt();
        session_->folderPath.clear();session_->selectedEntry.reset();
        rebuildVolumes(); rebuildTree();
    });
    connect(formats_, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index < 0 || session_->busy()) return;
        const auto format = formats_->itemData(index).toString();
        session_->classification.selectFormat(format.toStdString());
        session_->reclassify();
    });
    connect(folders_, &QTreeWidget::currentItemChanged, this, [this](QTreeWidgetItem* current) {
        const auto found = paths_.find(current);
        if (found != paths_.end()) showFolder(found->second);
    });
    connect(contents_, &QTableWidget::itemSelectionChanged, this, [this] {
        const auto row = contents_->currentRow();
        const auto* entry = entryAt(row);
        if(entry){const auto current=volume();if(current){const auto* entries=&current->entries;for(const auto index:session_->folderPath)entries=&entries->at(index).children;session_->selectedEntry=static_cast<std::size_t>(entry-entries->data());}}else session_->selectedEntry.reset();
        showDetails(entry);
    });
    connect(contents_, &QTableWidget::cellDoubleClicked, this, [this](int row) {
        const auto* selected = entryAt(row);
        if (!selected || selected->kind != FileSystemEntryKind::Directory) return;
        const auto currentVolume = volume();
        if (!currentVolume) return;
        const auto* entries = &currentVolume->entries;
        for (const auto index : session_->folderPath) entries = &entries->at(index).children;
        auto path = session_->folderPath;
        path.push_back(static_cast<std::size_t>(selected - entries->data()));
        for (const auto& [item, candidate] : paths_) if (candidate == path) { folders_->setCurrentItem(item); folders_->scrollToItem(item); break; }
    });
    rebuildVolumes();
    rebuildTree();
    loading_=new MediaLoadingOverlay(themes,translations,[this]{if(reading_->busy())reading_->stop();else visualization_->cancel();},panes,false,true);
    visualizationSubscription_=visualization_->subscribe([this]{const bool busy=visualization_->preparing();if(busy||visualLoading_){visualLoading_=busy;if(busy)refreshLoading();else refresh();}});
    readSubscription_=reading_->subscribe([this]{refresh();});
    operationSubscription_ = session_->subscribe([this] { if(session_->busy())refreshLoading();else refresh(); });
    languageSubscription_ = translations.subscribe([this] { displayed_.reset(); refresh(); });
    refresh();
}
QString ExplorerPage::text(const char* key) const { return QString::fromStdString(translations_.text(key)); }

QString ExplorerPage::diskFormatName(const std::string& id) const {
    if (const auto* format = session_->classification.catalog().resolveFormat(id)) {
        return QString::fromStdString(MediaEngine::imageFormatName(*format, [this](std::string_view key) {
            return translations_.text(std::string(key), TranslationConstants::MediaEngine);
        }));
    }
    return QString::fromStdString(id);
}
std::shared_ptr<const MediaEngine::FileSystemVolumeInfo> ExplorerPage::volume() const {
    if (!displayed_) return {};
    if (displayed_->diskExploration) {
        const auto& disk = displayed_->diskExploration->info();
        const auto chosen = std::find_if(disk.formats.begin(), disk.formats.end(), [&](const auto& format) {
            return format.formatId == session_->selectedFormat.value_or(disk.primaryFormatId);
        });
        if (chosen != disk.formats.end() && (disk.container.revolutionCount.has_value() || session_->selectedFormat)) return chosen->volume;
    }
    if (session_->selectedFormat) return {};
    if (displayed_->mediaExploration) {
        const auto& volumes = displayed_->mediaExploration->volumes;
        if (session_->selectedVolume < volumes.size()) return volumes[session_->selectedVolume].fileSystem;
    }
    return displayed_->diskExploration ? displayed_->diskExploration->info().volume : nullptr;
}
void ExplorerPage::refreshLoading() {
    const bool preparing = visualization_->preparing();
    const bool capturing = reading_->capture().loading();
    const auto progress = session_->busy() ? session_->progress() :
        capturing ? reading_->capture().loadingProgress() : visualization_->presentationProgress();
    loading_->display(session_->busy() || capturing || preparing, progress);
    if (session_->busy()) {
        status_->setText(QFileInfo(session_->loadingPath()).fileName());
        status_->setToolTip(session_->loadingPath());
        const auto icon = progress.capture ? FloppyIcon : FileIcon;
        mediaIcon_->setPixmap(fileIcon(QString::fromLatin1(icon)).pixmap(IdentityIconSize));
        mediaSummary_->hide();
        open_->setEnabled(false);
        read_->setEnabled(false);
        formats_->setEnabled(false);
        automatic_->setEnabled(false);
        classification_->setEnabled(false);
        classificationToggle_->setEnabled(false);
    }
}

void ExplorerPage::refresh() {
    refreshLoading();
    // Keep the header and panes in place until the loading card is dismissed.
    if (session_->busy() || visualization_->preparing()) return;
    const bool classificationAvailable=session_->result()&&session_->result()->diskExploration;classificationToggle_->setEnabled(classificationAvailable&&!session_->busy()&&!reading_->busy());classification_->setEnabled(!session_->busy()&&!reading_->busy());
    const auto color = themes_.data()->color(ThemeConstants::TextBrush);
    open_->setIcon(themeIcon(OpenIcon, color));
    read_->setIcon(themeIcon(ReadIcon, color));
    classificationToggle_->setIcon(themeIcon(SettingsIcon, color));
    classificationToggle_->setToolTip(text(ClassificationConstants::Advanced));
    classificationToggle_->setAccessibleName(text(ClassificationConstants::Advanced));
    automatic_->setText(text(AutomaticDetection));
    {
        const QSignalBlocker blocked(automatic_);
        automatic_->setChecked(session_->classification.automatic());
    }
    automatic_->setEnabled(!session_->busy() && !reading_->busy());
    for (std::size_t i = 0; i < summaryLabels_.size(); ++i) summaryLabels_[i]->setText(text(SummaryKeys[i]));
    open_->setText(text(Open));read_->setText(text(reading_->busy()?Stop:ReadDisk));
    read_->setEnabled(reading_->busy() || reading_->canStart());
    const auto& execution=reading_->capture().execution();auto readSummary=QString::fromStdString(translations_.text(execution.summaryKey.toStdString()));for(int index=0;index<execution.summaryArguments.size();++index)readSummary.replace(QString::fromLatin1(ArgumentPattern).arg(index),execution.summaryArguments[index]);readStatus_->setText(readSummary);readStatus_->setVisible(reading_->busy() || !execution.lastCommand.isEmpty());
    open_->setEnabled(!session_->busy() && !reading_->busy());
    formats_->setEnabled(!session_->busy() && !reading_->busy());
    formats_->setAccessibleName(text(Format));
    presentationLabel_->setText(text(Format));
    sessions_->setAccessibleName(text(Session));
    volumes_->setAccessibleName(text(Volume));
    QStringList headers;
    for (const auto key : HeaderKeys) headers.push_back(text(key));
    contents_->setHorizontalHeaderLabels(headers);
    if (session_->busy()) status_->setText(text(Loading));
    else if (!session_->error().isEmpty()) {
        auto detail = QString::fromStdString(Errors::render(translations_.language(), session_->error().toStdString()));
        if (const auto& message = session_->errorMessage()) {
            detail = mediaText(translations_,*message);
        }
        status_->setText(text(LoadFailed).replace(QString::fromLatin1(Argument), detail));
    }
    else status_->setText(session_->path().isEmpty() ? text(NoImage) : QFileInfo(session_->path()).fileName());
    status_->setToolTip(session_->path());
    const auto source = session_->busy() ? nullptr : session_->result();
    QStringList detected;
    auto mediaIcon = QString::fromLatin1(FileIcon);
    if (source && source->diskExploration) {
        const auto& disk = source->diskExploration->info();
        const auto* format = session_->classification.catalog().resolveFormat(session_->selectedFormat.value_or(disk.primaryFormatId));
        const auto iconId = MediaEngine::selectMediaIcon(MediaEngine::MediaKind::Floppy, {}, format);
        mediaIcon = QString::fromLatin1(MediaIdentityPrefix) + QString::fromLatin1(iconId.data(), iconId.size());
        for (const auto& id : source->diskExploration->info().detectedFormatIds) detected << diskFormatName(id);
    } else if (source && source->mediaExploration) {
        const auto& media = *source->mediaExploration;
        if (!media.formatId.empty()) detected << diskFormatName(media.formatId);
        if (media.mediaKind == MediaEngine::MediaKind::Tape) mediaIcon = QString::fromLatin1(TapeIcon);
        if (media.mediaKind == MediaEngine::MediaKind::Optical) mediaIcon = QString::fromLatin1(OpticalIcon);
    }
    mediaIcon_->setPixmap(fileIcon(mediaIcon).pixmap(IdentityIconSize));
    mediaSummary_->setText(detected.isEmpty() ? QString{} : mediaText(translations_,
        {DetectedFormats, {detected.join(QString::fromUtf8(FormatsSeparator)).toStdString()}, {}}));
    mediaSummary_->setToolTip(mediaSummary_->text());
    mediaSummary_->setVisible(!detected.isEmpty());
    if (displayed_ != session_->result() || displayedFormat_ != session_->selectedFormat) {
        displayed_ = session_->result();
        displayedFormat_ = session_->selectedFormat;
        rebuildVolumes();
        rebuildTree();
    }
}
void ExplorerPage::rebuildVolumes() {
    const QSignalBlocker blocked(volumes_);
    const QSignalBlocker blockedSessions(sessions_);
    const QSignalBlocker blockedFormats(formats_);
    volumes_->clear();
    sessions_->clear(); formats_->clear(); visibleVolumes_.clear();
    if (displayed_ && displayed_->diskExploration) {
        const auto& disk = displayed_->diskExploration->info();
        for (const auto& id : disk.detectedFormatIds) {
            auto choice = diskFormatName(id);
            if (const auto* format = session_->classification.catalog().resolveFormat(id)) {
                const auto family = MediaEngine::imageFormatFamilyName(*format, [this](std::string_view key) {
                    return translations_.text(std::string(key), TranslationConstants::MediaEngine);
                });
                choice.prepend(QString::fromStdString(family) + QString::fromUtf8(FormatsSeparator));
            }
            const auto recognized = std::any_of(disk.formats.begin(), disk.formats.end(), [&](const auto& format) {
                return format.formatId == id && format.volume && !format.volume->fileSystemId.empty();
            });
            formats_->addItem(recognized ? recognizedFormatIcon() : QIcon{}, choice, QString::fromStdString(id));
        }
        const auto& format = session_->selectedFormat ? *session_->selectedFormat : disk.primaryFormatId;
        const auto index = formats_->findData(QString::fromStdString(format));
        formats_->setPlaceholderText(index < 0 && session_->selectedFormat ? diskFormatName(format) : QString{});
        formats_->setCurrentIndex(index);
    }
    const auto multipleFormats = formats_->count() > 1 || session_->selectedFormat.has_value();
    formats_->setVisible(multipleFormats);
    presentationLabel_->setVisible(multipleFormats);
    sessions_->hide();
    volumes_->hide();
    if (!displayed_) return;
    if (displayed_->mediaExploration && !session_->selectedFormat) {
        const auto& values = displayed_->mediaExploration->volumes;
        std::set<std::int32_t> sessionNumbers;
        for (const auto& value : values) if (value.descriptor.sessionNumber) sessionNumbers.insert(*value.descriptor.sessionNumber);
        for (const auto number : sessionNumbers) sessions_->addItem(text(Session) + QString::fromLatin1(DetailSeparator) + QString::number(number), number);
        sessions_->setVisible(sessions_->count() > 1);
        if (session_->selectedSession) sessions_->setCurrentIndex(sessions_->findData(*session_->selectedSession));
        for (std::size_t index = 0; index < values.size(); ++index) {
            const auto& value = values[index];
            if (session_->selectedSession && value.descriptor.sessionNumber && value.descriptor.sessionNumber != session_->selectedSession) continue;
            const auto name=value.fileSystem?volumeName(*value.fileSystem):value.descriptor.nameRecipe?mediaText(translations_,*value.descriptor.nameRecipe):QString::fromStdString(value.descriptor.name.value_or(std::string{}));
            volumes_->addItem(name.isEmpty()?text(Unnamed):name);
            visibleVolumes_.push_back(index);
        }
    } else if (const auto value = volume()) { volumes_->addItem(volumeName(*value)); visibleVolumes_.push_back(0); }
    auto selected = std::find(visibleVolumes_.begin(), visibleVolumes_.end(), session_->selectedVolume);
    if (selected == visibleVolumes_.end() && !visibleVolumes_.empty()) { session_->selectedVolume = visibleVolumes_.front(); selected = visibleVolumes_.begin(); }
    volumes_->setVisible(volumes_->count() > 1);
    volumes_->setCurrentIndex(selected == visibleVolumes_.end() ? -1 : static_cast<int>(selected - visibleVolumes_.begin()));
}
void ExplorerPage::rebuildTree() {
    const auto saved = session_->folderPath;
    const QSignalBlocker blocked(folders_);
    paths_.clear(); folders_->clear();
    const auto currentVolume = volume();
    if (!currentVolume) { rows_.clear(); contents_->setRowCount(0); showDetails(nullptr); return; }
    auto* root = new QTreeWidgetItem(folders_, QStringList{volumeName(*currentVolume)});
    root->setIcon(NameColumn, fileIcon(QString::fromLatin1(FolderIcon)));
    paths_[root] = {};
    QTreeWidgetItem* selection = root;
    const auto append = [&](auto&& self, QTreeWidgetItem* parent, const std::vector<FileSystemEntryInfo>& entries, std::vector<std::size_t> path) -> void {
        for (std::size_t index = 0; index < entries.size(); ++index) {
            if (entries[index].kind != FileSystemEntryKind::Directory) continue;
            auto next = path; next.push_back(index);
            auto* item = new QTreeWidgetItem(parent, QStringList{QString::fromStdString(entries[index].name)});
            item->setIcon(NameColumn, fileIcon(QString::fromLatin1(FolderIcon)));
            paths_[item] = next;
            if (next == saved) selection = item;
            self(self, item, entries[index].children, next);
        }
    };
    append(append, root, currentVolume->entries, {});
    root->setExpanded(true);
    folders_->setCurrentItem(selection);
    showFolder(paths_.at(selection));
}
void ExplorerPage::showFolder(const std::vector<std::size_t>& path) {
    const auto currentVolume = volume(); if (!currentVolume) return;
    const auto* entries = &currentVolume->entries;
    for (const auto index : path) { if (index >= entries->size()) return; entries = &entries->at(index).children; }
    if(session_->folderPath!=path)session_->selectedEntry.reset();
    session_->folderPath = path;
    const QSignalBlocker blocked(contents_);
    contents_->clearContents(); rows_.clear();
    for (const auto& entry : *entries) rows_.push_back(&entry);
    QCollator collator;
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    const bool ascending = sortOrder_ == Qt::AscendingOrder;
    const bool directoriesFirst = sortColumn_ != NameColumn || ascending;
    std::stable_sort(rows_.begin(), rows_.end(), [&](const auto* left, const auto* right) {
        const bool directory = isDirectory(*left);
        if (directory != isDirectory(*right)) return directory == directoriesFirst;

        int order = 0;
        if (sortColumn_ == ModifiedColumn) {
            const auto a = dateKey(left->modified), b = dateKey(right->modified);
            order = (a > b) - (a < b);
        } else if (sortColumn_ == TypeColumn && !directory) {
            order = collator.compare(extension(*left), extension(*right));
        } else if (sortColumn_ == SizeColumn && !directory) {
            order = (left->size > right->size) - (left->size < right->size);
        }
        if (order == 0)
            order = collator.compare(QString::fromStdString(left->name), QString::fromStdString(right->name));
        return ascending ? order < 0 : order > 0;
    });

    contents_->setRowCount(static_cast<int>(rows_.size()));
    for (std::size_t row = 0; row < rows_.size(); ++row) {
        const auto& entry = *rows_[row];
        const auto index = static_cast<int>(row);
        auto* name = new QTableWidgetItem(QString::fromStdString(entry.name));
        const auto executableColor = themes_.data()->color(SuccessBrush);
        name->setIcon(entryIcon(entry, executableColor));
        if (entry.analysis && entry.analysis->categoryId == ExecutableCategory) {
            name->setForeground(executableColor);
            auto font = name->font();
            font.setWeight(QFont::DemiBold);
            name->setFont(font);
        }
        contents_->setItem(index, NameColumn, name);
        contents_->setItem(index, TypeColumn, new QTableWidgetItem(entryType(entry)));

        auto* size = new QTableWidgetItem(entry.kind == FileSystemEntryKind::Directory ? QString{} :
            QLocale().formattedDataSize(static_cast<qint64>(entry.size)));
        size->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        contents_->setItem(index, SizeColumn, size);

        auto* modified = new QTableWidgetItem;
        if (entry.modified) {
            const auto& t = *entry.modified;
            const QDateTime date(QDate(t.year, t.month, t.day), QTime(t.hour, t.minute, t.second));
            modified->setText(QLocale().toString(date, QLocale::ShortFormat));
        }
        contents_->setItem(index, ModifiedColumn, modified);
    }
    const FileSystemEntryInfo* selected = nullptr;
    if (session_->selectedEntry && *session_->selectedEntry < entries->size()) {
        selected = &entries->at(*session_->selectedEntry);
        for (int row = 0; row < contents_->rowCount(); ++row) {
            if (entryAt(row) == selected) {
                contents_->setCurrentCell(row, NameColumn);
                break;
            }
        }
    }
    showDetails(selected);
}
const FileSystemEntryInfo* ExplorerPage::entryAt(int row) const {
    return row >= 0 && static_cast<std::size_t>(row) < rows_.size() ? rows_[row] : nullptr;
}

QString ExplorerPage::entryType(const FileSystemEntryInfo& entry) const {
    if (entry.analysis && !entry.analysis->typeResourceKey.empty()) {
        const auto& analysis = *entry.analysis;
        auto extension = QString::fromStdString(analysis.extension);
        if (extension.startsWith(QLatin1Char(ExtensionSeparator))) extension.remove(0, 1);
        return mediaText(translations_, {analysis.typeResourceKey, {extension.toUpper().toStdString()}, analysis.typeTranslationDomain});
    }
    return text(entry.kind == FileSystemEntryKind::Directory ? Directory :
        entry.kind == FileSystemEntryKind::File ? File :
        entry.kind == FileSystemEntryKind::Link ? Link : Unknown);
}

QString ExplorerPage::volumeName(const MediaEngine::FileSystemVolumeInfo& value) const {return value.nameRecipe?mediaText(translations_,*value.nameRecipe):value.name.empty()?text(Unnamed):QString::fromStdString(value.name);}
void ExplorerPage::showDetails(const FileSystemEntryInfo* entry) {
    const auto currentVolume = volume();
    const auto fileSystemName = [this](const MediaEngine::FileSystemVolumeInfo& value) {
        if (value.fileSystemDisplayResourceKey)
            return mediaText(translations_, {*value.fileSystemDisplayResourceKey,
                value.fileSystemDisplayArguments, value.fileSystemDisplayTranslationDomain.value_or(std::string{})});
        return value.fileSystemDisplayName.empty() ? text(NoFileSystem) : QString::fromStdString(value.fileSystemDisplayName);
    };
    std::array<QString, SummaryKeys.size()> summary;
    summary.fill(QString::fromUtf8(EmptyValue));
    if (displayed_ && displayed_->diskExploration) {
        const auto& disk = displayed_->diskExploration->info();
        const auto& presentation = session_->selectedFormat ? *session_->selectedFormat : disk.primaryFormatId;
        const auto selected = std::find_if(disk.formats.begin(), disk.formats.end(), [&](const auto& format) {
            return format.formatId == presentation;
        });
        if (const auto* format = session_->classification.catalog().resolveFormat(presentation)) {
            summary[1] = QString::fromStdString(MediaEngine::imageFormatFamilyName(*format, [this](std::string_view key) {
                return translations_.text(std::string(key), TranslationConstants::MediaEngine);
            }));
        } else if (selected != disk.formats.end() && !selected->machineId.empty()) {
            summary[1] = QString::fromStdString(selected->machineId);
        }
        if (selected != disk.formats.end()) {
            if (selected->protectionId) {
                const auto protections = session_->classification.catalog().protections();
                const auto protection = std::find_if(protections.begin(), protections.end(), [&](const auto& value) {
                    return value.id == *selected->protectionId;
                });
                summary[2] = QString::fromStdString(protection == protections.end() ? *selected->protectionId : protection->displayName);
            }
        }
        if (session_->selectedFormat && !currentVolume) summary[3] = text(NoFileSystem);
    }
    if (currentVolume) {
        summary[0] = volumeName(*currentVolume);
        summary[3] = fileSystemName(*currentVolume);
        summary[4] = QLocale().formattedDataSize(static_cast<qint64>(currentVolume->capacity));
        if (currentVolume->freeSpaceKnown) summary[5] = QLocale().formattedDataSize(static_cast<qint64>(currentVolume->freeBytes));
        const auto count = [](auto&& self, const std::vector<FileSystemEntryInfo>& entries) -> qulonglong {
            qulonglong total = entries.size();
            for (const auto& item : entries) total += self(self, item.children);
            return total;
        };
        summary[6] = QLocale().toString(count(count, currentVolume->entries));
    }
    for (std::size_t i = 0; i < summary.size(); ++i) summaryValues_[i]->setText(summary[i]);
    while (detailRows_->rowCount()) detailRows_->removeRow(0);
    detailTitle_->setText(entry ? QString::fromStdString(entry->name) : summary.front());
    const auto icon = entry ? entryIcon(*entry, themes_.data()->color(SuccessBrush))
                            : fileIcon(QString::fromLatin1(FileIcon));
    detailIcon_->setPixmap(icon.pixmap(DetailIconSize));
    const auto add = [&](const char* key, const QString& value) {
        auto* cell = new QLabel(value);
        cell->setTextFormat(Qt::PlainText);
        cell->setWordWrap(true);
        cell->setTextInteractionFlags(Qt::TextSelectableByMouse);
        detailRows_->addRow(text(key), cell);
    };
    if (entry) {
        add(Type, entryType(*entry));
        add(HeaderKeys[SizeColumn], QLocale().formattedDataSize(static_cast<qint64>(entry->size)));
        if(entry->modified){const auto& t=*entry->modified;add(HeaderKeys[ModifiedColumn],QLocale().toString(QDateTime(QDate(t.year,t.month,t.day),QTime(t.hour,t.minute,t.second)),QLocale::ShortFormat));}
        if(entry->commentResourceKey)add(Comment,mediaText(translations_,{*entry->commentResourceKey,entry->commentArguments,entry->commentTranslationDomain.value_or(std::string{})}));else if(!entry->comment.empty())add(Comment,QString::fromStdString(entry->comment));
        if(entry->linkTarget)add(Link,QString::fromStdString(*entry->linkTarget));
        if(entry->analysis && entry->kind==FileSystemEntryKind::File){
            const auto& analysis=*entry->analysis;
            const auto localized=[this](const char* prefix,const std::string& id,const char* fallback){const auto key=std::string(prefix)+id;const auto value=translations_.text(key);return value==TranslationConstants::MissingKeyOpen+key+TranslationConstants::MissingKeyClose?text(fallback):QString::fromStdString(value);};
            if(!analysis.categoryId.empty())add(Category,localized(CategoryPrefix,analysis.categoryId,File));
            if(!analysis.contentFormatId.empty())add(ContentFormat,localized(ContentFormatPrefix,analysis.contentFormatId,ContentUnknown));
            if(!analysis.textEncodingId.empty() && analysis.textEncodingId!=NotApplicable){const auto encoding=std::find(TextEncodingIds.begin(),TextEncodingIds.end(),analysis.textEncodingId);add(TextEncoding,encoding==TextEncodingIds.end()?text(Unknown):QString::fromLatin1(TextEncodingNames[static_cast<std::size_t>(encoding-TextEncodingIds.begin())]));}
            if(!analysis.previewKindId.empty())add(Preview,localized(PreviewPrefix,analysis.previewKindId,PreviewNone));
            if(std::find(ExecutionIds.begin(),ExecutionIds.end(),analysis.executionKindId)!=ExecutionIds.end())add(Execution,localized(ExecutionPrefix,analysis.executionKindId,Unknown));
        }
        for(std::size_t index=0;index<RecordMetadataKeys.size();++index){const auto found=entry->metadata.find(RecordMetadataKeys[index]);if(found!=entry->metadata.end() && !found->second.empty())add(RecordMetadataLabels[index],QString::fromStdString(found->second));}
        if(const auto found=entry->metadata.find(EndRecordKey);found!=entry->metadata.end())add(EndRecordLabel,text(QString::fromStdString(found->second).compare(QLatin1String(TrueValue),Qt::CaseInsensitive)==0?Yes:No));
    } else if (const auto value = volume()) {
        add(Volume,volumeName(*value));
        add(SummaryKeys[1], summary[1]);
        add(SummaryKeys[2], summary[2]);
        add(FileSystem,fileSystemName(*value));
        if (displayed_->diskExploration && displayed_->diskExploration->info().container.revolutionCount.has_value())
            add(PartitionTable, text(NoPartitionTable));
        add(Capacity, QLocale().formattedDataSize(static_cast<qint64>(value->capacity)));
        if (value->freeSpaceKnown) add(Free, QLocale().formattedDataSize(static_cast<qint64>(value->freeBytes)));
        add(Entries, summary[6]);
    } else if(displayed_){add(Volume,text(Unnamed));add(FileSystem,displayed_->diskExploration?text(NoFileSystem):text(Unknown));}
    const bool fluxCatalogue = displayed_ && displayed_->diskExploration && displayed_->diskExploration->info().container.revolutionCount.has_value();
    if(!entry && displayed_ && !fluxCatalogue && displayed_->mediaExploration && session_->selectedVolume<displayed_->mediaExploration->volumes.size()){
        const auto& descriptor=displayed_->mediaExploration->volumes[session_->selectedVolume].descriptor;
        if(descriptor.sessionNumber)add(Session,QString::number(*descriptor.sessionNumber));if(descriptor.trackNumber)add(Track,QString::number(*descriptor.trackNumber));if(descriptor.partitionTable)add(PartitionTable,QString::fromStdString(*descriptor.partitionTable).toUpper());if(descriptor.partitionNumber)add(PartitionNumber,QString::number(*descriptor.partitionNumber));if(descriptor.partitionType)add(PartitionType,QString::fromStdString(*descriptor.partitionType));if(descriptor.partitionId)add(PartitionId,QString::fromStdString(*descriptor.partitionId));if(!volume())add(Capacity,QLocale().formattedDataSize(descriptor.length));
    }
    QStringList diagnostics;QSet<QString> seen;
    const auto append=[&](auto&& self,const MediaEngine::DiagnosticInfo& diagnostic)->void{const auto rendered=mediaText(translations_,diagnostic.message);if(!seen.contains(rendered)){seen.insert(rendered);diagnostics.push_back(rendered);}for(const auto& cause:diagnostic.causes)self(self,cause);};
    if(entry){for(const auto& diagnostic:entry->diagnostics)append(append,diagnostic);}
    else if(displayed_){
        if(const auto current=volume()){for(const auto& diagnostic:current->warnings)append(append,diagnostic);for(const auto& message:current->sourceMessages){const auto rendered=mediaText(translations_,message);if(!seen.contains(rendered)){seen.insert(rendered);diagnostics.push_back(rendered);}}}
        if(!fluxCatalogue && displayed_->mediaExploration){for(const auto& diagnostic:displayed_->mediaExploration->explorationDiagnostics)append(append,diagnostic);if(session_->selectedVolume<displayed_->mediaExploration->volumes.size())for(const auto& diagnostic:displayed_->mediaExploration->volumes[session_->selectedVolume].diagnostics)append(append,diagnostic);for(const auto& message:displayed_->mediaExploration->messages){const auto rendered=mediaText(translations_,message);if(!seen.contains(rendered)){seen.insert(rendered);diagnostics.push_back(rendered);}}}
        if(!fluxCatalogue && displayed_->diskExploration)for(const auto& diagnostic:displayed_->diskExploration->info().diagnostics)append(append,diagnostic);
    }
    if (entry || displayed_) add(Warnings,QLocale().toString(diagnostics.size()));
    detailWarnings_->setVisible(!diagnostics.isEmpty());
    detailWarnings_->setText(diagnostics.join(LineSeparator));
}
}
