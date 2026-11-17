#include "VisualizationPage.h"
#include "VisualizationConstants.h"
#include "Shared/Controls/Card.h"
#include "Shared/Localization/MediaText.h"
#include <Errors.h>
#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLocale>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QSplitter>
#include <QTableWidget>
#include <QVBoxLayout>
#include <algorithm>
namespace RetroMediaLab {
using namespace VisualizationConstants;
VisualizationPage::VisualizationPage(ThemeService& themes, Translations::TranslationService& translations, WorkspaceState& state, QWidget* parent)
    : QWidget(parent), translations_(translations) {
    if (!state.explorer) state.explorer = std::make_shared<Infrastructure::ExplorerSession>();
    if (!state.visualization) state.visualization = std::make_shared<Infrastructure::VisualizationSession>(state.explorer);
    session_ = state.visualization;
    auto* layout = new QVBoxLayout(this); layout->setContentsMargins(Padding,Padding,Padding,Padding); layout->setSpacing(Gap);
    auto* toolbar = new QHBoxLayout;
    open_ = new QPushButton(this); reset_ = new QPushButton(this);
    zoomOut_ = new QPushButton(QString::fromUtf8(ZoomOut),this); zoomIn_ = new QPushButton(QString::fromUtf8(ZoomIn),this); zoom_ = new QLabel(this);
    toolbar->addWidget(open_); toolbar->addStretch(); toolbar->addWidget(reset_); toolbar->addWidget(zoomOut_); toolbar->addWidget(zoom_); toolbar->addWidget(zoomIn_);
    layout->addLayout(toolbar);
    status_ = new QLabel(this); status_->setTextFormat(Qt::PlainText); status_->setWordWrap(true); layout->addWidget(status_);
    auto* splitter = new QSplitter(Qt::Vertical,this);
    auto* graph = new MediaGraph(themes,session_,splitter);
    auto* inspector = new Card(themes.data(),splitter);
    auto* details = new QVBoxLayout(inspector); details->setContentsMargins(Padding,Padding,Padding,Padding);
    auto* coordinates = new QHBoxLayout;
    sideLabel_ = new QLabel(inspector); trackLabel_ = new QLabel(inspector); revolutionLabel_ = new QLabel(inspector);
    side_ = new QSpinBox(inspector); track_ = new QSpinBox(inspector); revolutions_ = new QComboBox(inspector);
    coordinates->addWidget(sideLabel_); coordinates->addWidget(side_); coordinates->addWidget(trackLabel_); coordinates->addWidget(track_);
    coordinates->addWidget(revolutionLabel_); coordinates->addWidget(revolutions_); coordinates->addStretch(); details->addLayout(coordinates);
    auto* contents = new QSplitter(inspector);
    sectors_ = new QTableWidget(contents); sectors_->setColumnCount(static_cast<int>(Columns.size()));
    sectors_->setEditTriggers(QAbstractItemView::NoEditTriggers); sectors_->setSelectionBehavior(QAbstractItemView::SelectRows); sectors_->setSelectionMode(QAbstractItemView::SingleSelection);
    sectors_->verticalHeader()->hide(); sectors_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    hex_ = new QPlainTextEdit(contents); hex_->setReadOnly(true); hex_->setLineWrapMode(QPlainTextEdit::NoWrap);
    details->addWidget(contents,1);
    splitter->addWidget(graph); splitter->addWidget(inspector); splitter->setStretchFactor(0,GraphWeight); splitter->setStretchFactor(1,DetailsWeight);
    layout->addWidget(splitter,1);
    connect(open_,&QPushButton::clicked,this,[this,&state] {
        const auto path=QFileDialog::getOpenFileName(this,text(Open),state.defaultImageDirectory()); if(!path.isEmpty()) session_->opening()->open(path);
    });
    connect(reset_,&QPushButton::clicked,this,[this]{session_->setZoom(InitialZoom);});
    connect(zoomIn_,&QPushButton::clicked,this,[this]{session_->setZoom(std::min(MaximumZoom,session_->zoom*ZoomStep));});
    connect(zoomOut_,&QPushButton::clicked,this,[this]{session_->setZoom(std::max(InitialZoom,session_->zoom/ZoomStep));});
    connect(side_,&QSpinBox::valueChanged,this,[this](int value){session_->select(value,session_->position);});
    connect(track_,&QSpinBox::valueChanged,this,[this](int value){session_->select(session_->surface,value);});
    connect(revolutions_,&QComboBox::currentIndexChanged,this,[this](int value){if(value>=0)session_->setRevolution(static_cast<std::size_t>(value));});
    connect(sectors_,&QTableWidget::currentCellChanged,this,[this](int row){showSector(row);});
    subscription_=session_->subscribe([this]{refresh();});
    openingSubscription_=session_->opening()->subscribe([this]{refresh();});
    languageSubscription_=translations.subscribe([this]{refresh();});
    refresh();
}
QString VisualizationPage::text(const char* key) const {return QString::fromStdString(translations_.text(key));}
void VisualizationPage::refresh() {
    open_->setText(text(Open)); reset_->setText(text(Reset)); sideLabel_->setText(text(Side)); trackLabel_->setText(text(Track)); revolutionLabel_->setText(text(Revolution));
    zoom_->setText(QLocale().toString(session_->zoom*PercentFactor,PercentNumberFormat,0)+QString::fromLatin1(Percent));
    open_->setEnabled(!session_->opening()->busy());
    status_->setText(session_->opening()->busy() || session_->busy()?text(Loading):session_->opening()->path().isEmpty()?text(Empty):session_->opening()->path());
    const auto error=session_->error()?session_->error():session_->snapshot()?session_->snapshot()->warning:std::optional<MediaEngine::MessageInfo>{};
    if (error) {
        status_->setText(mediaText(translations_,*error));
    } else if(!session_->opening()->error().isEmpty()) status_->setText(QString::fromStdString(Errors::render(translations_.language(),session_->opening()->error().toStdString())));
    QStringList headers; for(const auto key:Columns)headers.push_back(text(key)); sectors_->setHorizontalHeaderLabels(headers);
    const QSignalBlocker blockedSectors(sectors_),blockedSide(side_),blockedTrack(track_),blockedRevolutions(revolutions_);
    sectors_->clearContents(); sectors_->setRowCount(0); rows_.clear(); revolutions_->clear();
    if(!session_->snapshot()) {hex_->clear();return;}
    const auto& snapshot=*session_->snapshot();
    const auto& descriptor=snapshot.descriptor->fields();
    const bool coordinatesVisible=descriptor.representationKind==MediaEngine::RepresentationKind::Flux || descriptor.representationKind==MediaEngine::RepresentationKind::Sectors;
    for(auto* control:{static_cast<QWidget*>(sideLabel_),static_cast<QWidget*>(side_),static_cast<QWidget*>(trackLabel_),static_cast<QWidget*>(track_),static_cast<QWidget*>(revolutionLabel_),static_cast<QWidget*>(revolutions_)})control->setVisible(coordinatesVisible);
    const auto maximumSurface=descriptor.surfaces.empty()?0:*std::max_element(descriptor.surfaces.begin(),descriptor.surfaces.end());
    side_->setRange(0,maximumSurface);side_->setValue(session_->surface);
    std::int32_t maximumCylinder=0;
    if(snapshot.source->diskExploration) {
        const auto& disk=snapshot.source->diskExploration->info();
        for(const auto& format:disk.formats) if(format.formatId==disk.primaryFormatId || disk.formats.size()==1) {
            for(const auto& sector:format.sectors) {
                maximumCylinder=std::max(maximumCylinder,static_cast<std::int32_t>(sector.cylinder));
                if(sector.head==session_->surface && sector.cylinder==session_->position)rows_.push_back(&sector);
            }
            break;
        }
        for(const auto& track:disk.tracks) {
            maximumCylinder=std::max(maximumCylinder,track.cylinder);
            if(track.head==session_->surface && track.cylinder==session_->position)
                for(const auto& revolution:track.revolutions)revolutions_->addItem(QString::number(revolution.number));
        }
    }
    track_->setRange(0,maximumCylinder);track_->setValue(static_cast<int>(session_->position));
    revolutions_->setCurrentIndex(static_cast<int>(session_->revolution));
    sectors_->setRowCount(static_cast<int>(rows_.size()));
    for(std::size_t index=0;index<rows_.size();++index) {
        const auto& sector=*rows_[index];
        const auto integrity=sector.dataValid?(*sector.dataValid?ValidKey:InvalidKey):UnavailableKey;
        const QStringList values{QString::number(sector.head),QString::number(sector.cylinder),QString::number(sector.number),QLocale().formattedDataSize(static_cast<qint64>(sector.size)),text(integrity)};
        for(int column=0;column<values.size();++column)sectors_->setItem(static_cast<int>(index),column,new QTableWidgetItem(values[column]));
    }
    if(!rows_.empty()) {
        int selected=0;
        if(session_->selectedSector)for(std::size_t index=0;index<rows_.size();++index)if(rows_[index]->logicalBlock==*session_->selectedSector) {selected=static_cast<int>(index);break;}
        sectors_->setCurrentCell(selected,0);showSector(selected);
    } else hex_->clear();
}
void VisualizationPage::showSector(int row) {
    if(row<0 || static_cast<std::size_t>(row)>=rows_.size() || !rows_[row]->data) {hex_->clear();return;}
    session_->selectedSector=rows_[row]->logicalBlock;
    const auto& bytes=*rows_[row]->data;
    QStringList lines;
    for(std::size_t offset=0;offset<std::min(bytes.size(),static_cast<std::size_t>(HexPreviewBytes));offset+=HexBytesPerLine) {
        QStringList line;
        for(std::size_t index=offset;index<std::min(bytes.size(),offset+HexBytesPerLine);++index)
            line.push_back(QString::number(bytes[index],HexBase).rightJustified(HexDigits,QLatin1Char(HexPadding)).toUpper());
        lines.push_back(line.join(QLatin1Char(HexSeparator)));
    }
    hex_->setPlainText(lines.join(QLatin1Char(LineSeparator)));
}
}
