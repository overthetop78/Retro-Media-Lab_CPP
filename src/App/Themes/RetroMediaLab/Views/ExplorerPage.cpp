#include "ExplorerPage.h"
#include "Media/ExplorerReadSession.h"
#include "MediaPageConstants.h"
#include "Shared/Controls/Card.h"
#include "Shared/Localization/MediaText.h"
#include <Errors.h>
#include <QComboBox>
#include <QCoreApplication>
#include <QMessageBox>
#include <QDateTime>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSplitter>
#include <QScrollArea>
#include <QSet>
#include <QTableWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <algorithm>
#include <set>

namespace RetroMediaLab {
using namespace MediaPageConstants;
using MediaEngine::FileSystemEntryInfo;
using MediaEngine::FileSystemEntryKind;
ExplorerPage::ExplorerPage(ThemeService& themes, Translations::TranslationService& translations, WorkspaceState& state, QWidget* parent)
    : QWidget(parent), translations_(translations) {
    state.ensureMediaOperations(translations);
    reading_=state.explorerReading;
    if (!state.explorer) state.explorer = std::make_shared<Infrastructure::ExplorerSession>();
    session_ = state.explorer;
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(Padding, Padding, Padding, Padding);
    layout->setSpacing(Gap);
    auto* toolbar = new QHBoxLayout;
    read_=new QPushButton(this);
    open_ = new QPushButton(this);
    cancel_ = new QPushButton(this);
    volumes_ = new QComboBox(this);
    sessions_ = new QComboBox(this);
    formats_ = new QComboBox(this);
    volumes_->setMinimumWidth(0);
    toolbar->addWidget(read_);toolbar->addWidget(open_);
    toolbar->addWidget(formats_);
    toolbar->addWidget(sessions_);
    toolbar->addWidget(volumes_, 1);
    toolbar->addWidget(cancel_);
    layout->addLayout(toolbar);
    status_ = new QLabel(this);
    status_->setTextFormat(Qt::PlainText);
    status_->setWordWrap(true);
    layout->addWidget(status_);
    readStatus_=new QLabel(this);readStatus_->setWordWrap(true);readStatus_->setTextFormat(Qt::PlainText);layout->addWidget(readStatus_);
    auto* splitter = new QSplitter(this);
    folders_ = new QTreeWidget(splitter);
    folders_->setHeaderHidden(true);
    contents_ = new QTableWidget(splitter);
    contents_->setColumnCount(ColumnCount);
    contents_->setSelectionBehavior(QAbstractItemView::SelectRows);
    contents_->setSelectionMode(QAbstractItemView::SingleSelection);
    contents_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    contents_->verticalHeader()->hide();
    contents_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    contents_->horizontalHeader()->setSectionResizeMode(NameColumn, QHeaderView::Stretch);
    auto* detailScroll=new QScrollArea(splitter);detailScroll->setWidgetResizable(true);detailScroll->setFrameShape(QFrame::NoFrame);
    auto* card = new Card(themes.data(), detailScroll);detailScroll->setWidget(card);
    auto* detailLayout = new QVBoxLayout(card);
    details_ = new QLabel(card);
    details_->setTextFormat(Qt::PlainText);
    details_->setWordWrap(true);
    details_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    detailLayout->addWidget(details_);
    detailLayout->addStretch();
    for (auto* child : {static_cast<QWidget*>(folders_), static_cast<QWidget*>(contents_), static_cast<QWidget*>(detailScroll)}) child->setMinimumWidth(0);
    splitter->setStretchFactor(0, TreeStretch);
    splitter->setStretchFactor(1, ContentsStretch);
    splitter->setStretchFactor(DetailsPaneIndex, DetailsStretch);
    layout->addWidget(splitter, 1);
    connect(read_,&QPushButton::clicked,this,[this,&state]{
        if(reading_->busy()){if(QMessageBox::question(this,text(StopTitle),text(StopConfirm),QMessageBox::Yes|QMessageBox::No,QMessageBox::No)==QMessageBox::Yes)reading_->stop();return;}
        const auto id=state.selectedHardwareDeviceId.isEmpty()?state.reading->draft.deviceId:state.selectedHardwareDeviceId;
        const auto found=std::find_if(state.hardware().devices.begin(),state.hardware().devices.end(),[&](const auto& device){return device.id==id;});const auto label=found==state.hardware().devices.end()?text(Unknown):found->name;
        if(QMessageBox::question(this,text(ReadConfirmTitle),text(ReadConfirm).replace(QString::fromLatin1(Argument),label),QMessageBox::Yes|QMessageBox::No,QMessageBox::No)!=QMessageBox::Yes)return;
        try{reading_->start(state.hardware(),state.engines(),QCoreApplication::applicationDirPath(),id);}catch(const std::exception& error){readStatus_->setText(mediaErrorText(translations_,error));readStatus_->show();}
    });
    connect(open_, &QPushButton::clicked, this, [this, &state] {
        const auto path = QFileDialog::getOpenFileName(this, text(Open), state.defaultImageDirectory());
        if (!path.isEmpty()) session_->open(path);
    });
    connect(cancel_, &QPushButton::clicked, this, [this] { session_->cancel(); });
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
        session_->open(session_->path(), format.isEmpty() ? std::optional<std::string>{} : format.toStdString());
    });
    connect(folders_, &QTreeWidget::currentItemChanged, this, [this](QTreeWidgetItem* current) {
        const auto found = paths_.find(current);
        if (found != paths_.end()) showFolder(found->second);
    });
    connect(contents_, &QTableWidget::itemSelectionChanged, this, [this] {
        const auto row = contents_->currentRow();
        const auto* entry=row>=0 && static_cast<std::size_t>(row)<rows_.size()?rows_[row]:nullptr;
        if(entry){const auto current=volume();if(current){const auto* entries=&current->entries;for(const auto index:session_->folderPath)entries=&entries->at(index).children;session_->selectedEntry=static_cast<std::size_t>(entry-entries->data());}}else session_->selectedEntry.reset();
        showDetails(entry);
    });
    connect(contents_, &QTableWidget::cellDoubleClicked, this, [this](int row) {
        if (row < 0 || static_cast<std::size_t>(row) >= rows_.size() || rows_[row]->kind != FileSystemEntryKind::Directory) return;
        const auto selected = rows_[row];
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
    readSubscription_=reading_->subscribe([this]{refresh();});
    operationSubscription_ = session_->subscribe([this] { refresh(); });
    languageSubscription_ = translations.subscribe([this] { displayed_.reset(); refresh(); });
    refresh();
}
QString ExplorerPage::text(const char* key) const { return QString::fromStdString(translations_.text(key)); }
std::shared_ptr<const MediaEngine::FileSystemVolumeInfo> ExplorerPage::volume() const {
    if (!displayed_) return {};
    if (displayed_->mediaExploration) {
        const auto& volumes = displayed_->mediaExploration->volumes;
        if (session_->selectedVolume < volumes.size()) return volumes[session_->selectedVolume].fileSystem;
    }
    return displayed_->diskExploration ? displayed_->diskExploration->info().volume : nullptr;
}
void ExplorerPage::refresh() {
    open_->setText(text(Open)); cancel_->setText(text(Cancel));read_->setText(text(reading_->busy()?Stop:ReadDisk));
    read_->setEnabled(reading_->busy() || reading_->canStart());
    const auto& execution=reading_->capture().execution();auto readSummary=QString::fromStdString(translations_.text(execution.summaryKey.toStdString()));for(int index=0;index<execution.summaryArguments.size();++index)readSummary.replace(QString::fromLatin1(ArgumentPattern).arg(index),execution.summaryArguments[index]);readStatus_->setText(readSummary);readStatus_->setVisible(reading_->busy() || !execution.lastCommand.isEmpty());
    open_->setEnabled(!session_->busy() && !reading_->busy()); cancel_->setVisible(session_->busy() && !reading_->busy());
    formats_->setEnabled(!session_->busy() && !reading_->busy());
    formats_->setAccessibleName(text(Format));
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
    else status_->setText(session_->path().isEmpty()?text(NoImage):session_->path());
    if (displayed_ != session_->result()) {
        displayed_ = session_->result();
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
    formats_->addItem(text(Automatic), QString{});
    if (displayed_ && displayed_->diskExploration) {
        const auto& disk = displayed_->diskExploration->info();
        for (const auto& format : disk.detectedFormatIds) formats_->addItem(QString::fromStdString(format), QString::fromStdString(format));
    }
    formats_->setVisible(formats_->count() > 1);
    if (session_->selectedFormat) formats_->setCurrentIndex(formats_->findData(QString::fromStdString(*session_->selectedFormat)));
    sessions_->hide();
    if (!displayed_) return;
    if (displayed_->mediaExploration) {
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
    volumes_->setCurrentIndex(selected == visibleVolumes_.end() ? -1 : static_cast<int>(selected - visibleVolumes_.begin()));
}
void ExplorerPage::rebuildTree() {
    const auto saved = session_->folderPath;
    const QSignalBlocker blocked(folders_);
    paths_.clear(); folders_->clear();
    const auto currentVolume = volume();
    if (!currentVolume) { rows_.clear(); contents_->setRowCount(0); showDetails(nullptr); return; }
    auto* root = new QTreeWidgetItem(folders_, QStringList{volumeName(*currentVolume)});
    paths_[root] = {};
    QTreeWidgetItem* selection = root;
    const auto append = [&](auto&& self, QTreeWidgetItem* parent, const std::vector<FileSystemEntryInfo>& entries, std::vector<std::size_t> path) -> void {
        for (std::size_t index = 0; index < entries.size(); ++index) {
            if (entries[index].kind != FileSystemEntryKind::Directory) continue;
            auto next = path; next.push_back(index);
            auto* item = new QTreeWidgetItem(parent, QStringList{QString::fromStdString(entries[index].name)});
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
    std::stable_sort(rows_.begin(), rows_.end(), [](const auto* left, const auto* right) {
        if ((left->kind == FileSystemEntryKind::Directory) != (right->kind == FileSystemEntryKind::Directory)) return left->kind == FileSystemEntryKind::Directory;
        return QString::localeAwareCompare(QString::fromStdString(left->name), QString::fromStdString(right->name)) < 0;
    });
    contents_->setRowCount(static_cast<int>(rows_.size()));
    for (std::size_t row = 0; row < rows_.size(); ++row) {
        const auto& entry = *rows_[row];
        const auto type = entry.kind == FileSystemEntryKind::Directory ? Directory : entry.kind == FileSystemEntryKind::File ? File : entry.kind == FileSystemEntryKind::Link ? Link : Unknown;
        contents_->setItem(static_cast<int>(row), NameColumn, new QTableWidgetItem(QString::fromStdString(entry.name)));
        const auto typeName = entry.analysis && !entry.analysis->typeResourceKey.empty()
            ? QString::fromStdString(translations_.text(entry.analysis->typeResourceKey, entry.analysis->typeTranslationDomain)) : text(type);
        contents_->setItem(static_cast<int>(row), TypeColumn, new QTableWidgetItem(typeName));
        contents_->setItem(static_cast<int>(row), SizeColumn, new QTableWidgetItem(QLocale().formattedDataSize(static_cast<qint64>(entry.size))));
        if (entry.modified) { const auto& t = *entry.modified; contents_->setItem(static_cast<int>(row), ModifiedColumn, new QTableWidgetItem(QLocale().toString(QDateTime(QDate(t.year,t.month,t.day), QTime(t.hour,t.minute,t.second)), QLocale::ShortFormat))); }
    }
    const FileSystemEntryInfo* selected=nullptr;
    if(session_->selectedEntry && *session_->selectedEntry<entries->size())for(std::size_t row=0;row<rows_.size();++row)if(rows_[row]==&entries->at(*session_->selectedEntry)){contents_->setCurrentCell(static_cast<int>(row),NameColumn);selected=rows_[row];break;}
    showDetails(selected);
}
QString ExplorerPage::volumeName(const MediaEngine::FileSystemVolumeInfo& value) const {return value.nameRecipe?mediaText(translations_,*value.nameRecipe):value.name.empty()?text(Unnamed):QString::fromStdString(value.name);}
void ExplorerPage::showDetails(const FileSystemEntryInfo* entry) {
    QStringList lines;
    const auto add = [&](const char* key, const QString& value) { lines.push_back(text(key) + QString::fromLatin1(DetailSeparator) + value); };
    if (entry) {
        lines.push_back(QString::fromStdString(entry->name));
        add(Type,entry->analysis && !entry->analysis->typeResourceKey.empty()?QString::fromStdString(translations_.text(entry->analysis->typeResourceKey,entry->analysis->typeTranslationDomain)):text(entry->kind==FileSystemEntryKind::Directory?Directory:entry->kind==FileSystemEntryKind::File?File:entry->kind==FileSystemEntryKind::Link?Link:Unknown));
        add(HeaderKeys[SizeColumn], QLocale().formattedDataSize(static_cast<qint64>(entry->size)));
        if(entry->modified){const auto& t=*entry->modified;add(HeaderKeys[ModifiedColumn],QLocale().toString(QDateTime(QDate(t.year,t.month,t.day),QTime(t.hour,t.minute,t.second)),QLocale::ShortFormat));}
        if(entry->commentResourceKey)add(Comment,mediaText(translations_,{*entry->commentResourceKey,entry->commentArguments,entry->commentTranslationDomain.value_or(std::string{})}));else if(!entry->comment.empty())add(Comment,QString::fromStdString(entry->comment));
        if(entry->linkTarget)add(Link,QString::fromStdString(*entry->linkTarget));
        if(entry->analysis){if(!entry->analysis->contentFormatId.empty())add(ContentFormat,QString::fromStdString(entry->analysis->contentFormatId));if(!entry->analysis->textEncodingId.empty())add(TextEncoding,QString::fromStdString(entry->analysis->textEncodingId));}
    } else if (const auto value = volume()) {
        add(Volume,volumeName(*value));
        add(FileSystem,value->fileSystemDisplayResourceKey?mediaText(translations_,{*value->fileSystemDisplayResourceKey,value->fileSystemDisplayArguments,value->fileSystemDisplayTranslationDomain.value_or(std::string{})}):QString::fromStdString(value->fileSystemDisplayName));
        add(Capacity, QLocale().formattedDataSize(static_cast<qint64>(value->capacity)));
        if (value->freeSpaceKnown) add(Free, QLocale().formattedDataSize(static_cast<qint64>(value->freeBytes)));
        const auto count=[](auto&& self,const std::vector<FileSystemEntryInfo>& entries)->std::uint64_t{std::uint64_t total=entries.size();for(const auto& item:entries)total+=self(self,item.children);return total;};add(Entries,QLocale().toString(static_cast<qulonglong>(count(count,value->entries))));
    } else if(displayed_){add(Volume,text(Unnamed));add(FileSystem,displayed_->diskExploration?text(NoFileSystem):text(Unknown));}else lines.push_back(text(SelectEntry));
    if(!entry && displayed_ && displayed_->mediaExploration && session_->selectedVolume<displayed_->mediaExploration->volumes.size()){
        const auto& descriptor=displayed_->mediaExploration->volumes[session_->selectedVolume].descriptor;
        if(descriptor.sessionNumber)add(Session,QString::number(*descriptor.sessionNumber));if(descriptor.trackNumber)add(Track,QString::number(*descriptor.trackNumber));if(descriptor.partitionTable)add(PartitionTable,QString::fromStdString(*descriptor.partitionTable).toUpper());if(descriptor.partitionNumber)add(PartitionNumber,QString::number(*descriptor.partitionNumber));if(descriptor.partitionType)add(PartitionType,QString::fromStdString(*descriptor.partitionType));if(descriptor.partitionId)add(PartitionId,QString::fromStdString(*descriptor.partitionId));if(!volume())add(Capacity,QLocale().formattedDataSize(descriptor.length));
    }
    QStringList diagnostics;QSet<QString> seen;
    const auto append=[&](auto&& self,const MediaEngine::DiagnosticInfo& diagnostic)->void{const auto rendered=mediaText(translations_,diagnostic.message);if(!seen.contains(rendered)){seen.insert(rendered);diagnostics.push_back(rendered);}for(const auto& cause:diagnostic.causes)self(self,cause);};
    if(entry){for(const auto& diagnostic:entry->diagnostics)append(append,diagnostic);}
    else if(displayed_){
        if(const auto current=volume()){for(const auto& diagnostic:current->warnings)append(append,diagnostic);for(const auto& message:current->sourceMessages){const auto rendered=mediaText(translations_,message);if(!seen.contains(rendered)){seen.insert(rendered);diagnostics.push_back(rendered);}}}
        if(displayed_->mediaExploration){for(const auto& diagnostic:displayed_->mediaExploration->explorationDiagnostics)append(append,diagnostic);if(session_->selectedVolume<displayed_->mediaExploration->volumes.size())for(const auto& diagnostic:displayed_->mediaExploration->volumes[session_->selectedVolume].diagnostics)append(append,diagnostic);for(const auto& message:displayed_->mediaExploration->messages){const auto rendered=mediaText(translations_,message);if(!seen.contains(rendered)){seen.insert(rendered);diagnostics.push_back(rendered);}}}
        if(displayed_->diskExploration)for(const auto& diagnostic:displayed_->diskExploration->info().diagnostics)append(append,diagnostic);
    }
    add(Warnings,QLocale().toString(diagnostics.size()));lines.append(diagnostics);
    details_->setText(lines.join(LineSeparator));
}
}
