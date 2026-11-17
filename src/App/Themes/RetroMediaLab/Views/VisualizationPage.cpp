#include "VisualizationPage.h"
#include "VisualizationConstants.h"
#include "Shared/Controls/Card.h"
#include "../FlowLayout.h"
#include "Shared/Localization/MediaText.h"
#include <Errors.h>
#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLocale>
#include "Shared/Controls/HexGrid.h"
#include <QPushButton>
#include <QPlainTextEdit>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QSplitter>
#include <QTableWidget>
#include <QVBoxLayout>
#include <algorithm>
namespace RetroMediaLab {
using namespace VisualizationConstants;
namespace {
QString sequentialTime(std::int64_t ticks){
    const auto seconds=ticks/MediaEngine::SequentialTicksPerSecond;
    const auto milliseconds=(ticks%MediaEngine::SequentialTicksPerSecond)*MillisecondsPerSecond/MediaEngine::SequentialTicksPerSecond;
    return QString::fromLatin1(TimePattern).arg(seconds/SecondsPerHour,SequentialTimeDigits,DecimalBase,QLatin1Char(HexPadding)).arg(seconds/SecondsPerMinute%SecondsPerMinute,SequentialTimeDigits,DecimalBase,QLatin1Char(HexPadding)).arg(seconds%SecondsPerMinute,SequentialTimeDigits,DecimalBase,QLatin1Char(HexPadding)).arg(milliseconds,SequentialMillisecondDigits,DecimalBase,QLatin1Char(HexPadding));
}
}

VisualizationPage::VisualizationPage(ThemeService& themes, Translations::TranslationService& translations, WorkspaceState& state, QWidget* parent)
    : QWidget(parent), translations_(translations),themes_(themes) {
    if (!state.explorer) state.explorer = std::make_shared<Infrastructure::ExplorerSession>();
    if (!state.visualization) state.visualization = std::make_shared<Infrastructure::VisualizationSession>(state.explorer);
    session_ = state.visualization;
    auto* layout = new QVBoxLayout(this); layout->setContentsMargins(Padding,Padding,Padding,Padding); layout->setSpacing(Gap);
    auto* toolbar = new QHBoxLayout;
    open_ = new QPushButton(this); reset_ = new QPushButton(this);
    zoomOut_ = new QPushButton(QString::fromUtf8(ZoomOut),this); zoomIn_ = new QPushButton(QString::fromUtf8(ZoomIn),this); zoom_ = new QLabel(this);
    representation_=new QComboBox(this);representation_->addItems({QString{},QString{}});
    toolbar->addWidget(open_);toolbar->addWidget(representation_); toolbar->addStretch(); toolbar->addWidget(reset_); toolbar->addWidget(zoomOut_); toolbar->addWidget(zoom_); toolbar->addWidget(zoomIn_);
    layout->addLayout(toolbar);
    status_ = new QLabel(this); status_->setTextFormat(Qt::PlainText); status_->setWordWrap(true); layout->addWidget(status_);
    summary_=new QLabel(this);summary_->setWordWrap(true);summary_->setTextFormat(Qt::PlainText);layout->addWidget(summary_);
    auto* splitter = new QSplitter(Qt::Vertical,this);
    auto* graph = new MediaGraph(themes,session_,splitter);
    auto* inspector = new Card(themes.data(),splitter);
    auto* details = new QVBoxLayout(inspector); details->setContentsMargins(Padding,Padding,Padding,Padding);
    auto* coordinates = new QHBoxLayout;
    sideLabel_ = new QLabel(inspector); trackLabel_ = new QLabel(inspector); revolutionLabel_ = new QLabel(inspector);
    side_ = new QSpinBox(inspector); track_ = new QSpinBox(inspector); revolutions_ = new QComboBox(inspector);
    coordinates->addWidget(sideLabel_); coordinates->addWidget(side_); coordinates->addWidget(trackLabel_); coordinates->addWidget(track_);
    coordinates->addWidget(revolutionLabel_); coordinates->addWidget(revolutions_);decoderLabel_=new QLabel(inspector);decoder_=new QComboBox(inspector);decoder_->setMaximumWidth(FluxDecoderMaximumWidth);coordinates->addWidget(decoderLabel_);coordinates->addWidget(decoder_); coordinates->addStretch(); details->addLayout(coordinates);
    elementDetails_=new QLabel(inspector);elementDetails_->setWordWrap(true);elementDetails_->setTextFormat(Qt::PlainText);details->addWidget(elementDetails_);
    fluxDetails_=new QPlainTextEdit(inspector);fluxDetails_->setReadOnly(true);fluxDetails_->setMaximumHeight(FluxDetailsMaximumHeight);details->addWidget(fluxDetails_);
    auto* contents = new QSplitter(inspector);
    sectors_ = new QTableWidget(contents); sectors_->setColumnCount(static_cast<int>(Columns.size()));
    sectors_->setEditTriggers(QAbstractItemView::NoEditTriggers); sectors_->setSelectionBehavior(QAbstractItemView::SelectRows); sectors_->setSelectionMode(QAbstractItemView::SingleSelection);
    sectors_->verticalHeader()->hide(); sectors_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    hex_ = new HexGrid(contents);
    details->addWidget(contents,1);
    splitter->addWidget(graph); splitter->addWidget(inspector); splitter->setStretchFactor(0,GraphWeight); splitter->setStretchFactor(1,DetailsWeight);
    layout->addWidget(splitter,1);
    legend_=new QWidget(this);auto* legendLayout=new FlowLayout(legend_);legendLayout->setContentsMargins(0,0,0,0);legendLayout->setSpacing(Gap);
    for(std::size_t index=0;index<legendLabels_.size();++index){auto* item=new QWidget(legend_);auto* row=new QHBoxLayout(item);row->setContentsMargins(0,0,0,0);row->setSpacing(Gap);legendDots_[index]=new QLabel(QString::fromUtf8(LegendMark),item);legendLabels_[index]=new QLabel(item);row->addWidget(legendDots_[index]);row->addWidget(legendLabels_[index]);legendLayout->addWidget(item);}layout->addWidget(legend_);
    themeSubscription_=themes.subscribe([this]{refresh();});
    connect(open_,&QPushButton::clicked,this,[this,&state] {
        const auto path=QFileDialog::getOpenFileName(this,text(Open),state.defaultImageDirectory()); if(!path.isEmpty()) session_->opening()->open(path);
    });
    connect(representation_,&QComboBox::currentIndexChanged,this,[this](int index){if(index>=0)session_->setWaveform(index!=0);});
    connect(reset_,&QPushButton::clicked,this,[this]{session_->setZoom(InitialZoom);});
    connect(zoomIn_,&QPushButton::clicked,this,[this]{session_->setZoom(std::min(MaximumZoom,session_->zoom*ZoomStep));});
    connect(zoomOut_,&QPushButton::clicked,this,[this]{session_->setZoom(std::max(InitialZoom,session_->zoom/ZoomStep));});
    connect(side_,&QSpinBox::valueChanged,this,[this](int value){session_->select(value,session_->position);});
    connect(track_,&QSpinBox::valueChanged,this,[this](int value){session_->select(session_->surface,value);});
    connect(revolutions_,&QComboBox::currentIndexChanged,this,[this](int value){if(value<0)return;if(session_->snapshot() && session_->snapshot()->flux){session_->fluxSynthesis=value==0;session_->setRevolution(value==0?0:static_cast<std::size_t>(value-1));}else session_->setRevolution(static_cast<std::size_t>(value));});
    connect(decoder_,&QComboBox::currentIndexChanged,this,[this](int index){if(index<0)return;const auto id=decoder_->itemData(index).toString();session_->setFluxDecoder(id.isEmpty()?std::nullopt:std::optional<std::string>(id.toStdString()));});
    connect(sectors_,&QTableWidget::currentCellChanged,this,[this](int row){if(row>=0 && session_->snapshot() && (session_->snapshot()->linear || session_->snapshot()->sequential)){session_->inspect(static_cast<std::size_t>(row));return;}if(row>=0 && static_cast<std::size_t>(row)<rows_.size() && session_->snapshot() && session_->snapshot()->sectors)session_->selectSector(rows_[row]->logicalBlock);else showSector(row);});
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
    const QSignalBlocker blockedRepresentation(representation_);
    representation_->setAccessibleName(text(DisplayMode));representation_->setItemText(0,text(Segments));representation_->setItemText(1,text(Signal));
    representation_->setVisible(session_->snapshot() && session_->snapshot()->sequential.has_value());
    representation_->setCurrentIndex(session_->showWaveform?1:0);
    representation_->setEnabled(session_->snapshot() && session_->snapshot()->tape && !session_->snapshot()->tape->samples().empty());
    status_->setText(session_->opening()->busy() || session_->busy()?text(Loading):session_->opening()->path().isEmpty()?text(Empty):session_->opening()->path());
    const auto error=session_->error()?session_->error():session_->snapshot()?session_->snapshot()->warning:std::optional<MediaEngine::MessageInfo>{};
    if (error) {
        status_->setText(mediaText(translations_,*error));
    } else if(!session_->opening()->error().isEmpty()) status_->setText(QString::fromStdString(Errors::render(translations_.language(),session_->opening()->error().toStdString())));
    QStringList headers; for(const auto key:Columns)headers.push_back(text(key)); sectors_->setHorizontalHeaderLabels(headers);
    const QSignalBlocker blockedSectors(sectors_),blockedSide(side_),blockedTrack(track_),blockedRevolutions(revolutions_);
    sectors_->clearContents(); sectors_->setRowCount(0); rows_.clear();rowStates_.clear(); revolutions_->clear();
    summary_->clear();elementDetails_->clear();fluxDetails_->clear();
    const bool flux=session_->snapshot() && session_->snapshot()->flux.has_value();
    decoder_->setVisible(flux);decoderLabel_->setVisible(flux);decoderLabel_->setText(text(Decoder));fluxDetails_->setVisible(flux);
    const QSignalBlocker blockedDecoder(decoder_);decoder_->clear();
    for(auto* label:legendLabels_)label->parentWidget()->hide();
    legend_->setVisible(session_->snapshot() && (session_->snapshot()->sectors.has_value() || session_->snapshot()->linear.has_value() || session_->snapshot()->sequential.has_value() || flux));
    for(std::size_t index=0;index<SectorLabels.size();++index){legendLabels_[index]->setText(text(SectorLabels[index]));auto palette=legendDots_[index]->palette();palette.setColor(QPalette::WindowText,themes_.data()->color(SectorBrushes[index]));legendDots_[index]->setPalette(palette);}
    for(std::size_t index=0;index<SectorLabels.size();++index)legendLabels_[index]->parentWidget()->show();
    if(!session_->snapshot()) {hex_->setBytes({});return;}
    const auto& snapshot=*session_->snapshot();
    const auto& descriptor=snapshot.descriptor->fields();
    if(snapshot.flux){
        decoder_->addItem(text(Automatic),QString{});for(const auto& id:snapshot.flux->decoders)decoder_->addItem(text((std::string(DecoderPrefix)+id).c_str()),QString::fromStdString(id));decoder_->setCurrentIndex(session_->fluxDecoder?decoder_->findData(QString::fromStdString(*session_->fluxDecoder)):0);
        for(auto* control:{static_cast<QWidget*>(sideLabel_),static_cast<QWidget*>(side_),static_cast<QWidget*>(trackLabel_),static_cast<QWidget*>(track_),static_cast<QWidget*>(revolutionLabel_),static_cast<QWidget*>(revolutions_)})control->show();
        std::int32_t maximumHead=0,maximumTrack=0;for(const auto& track:snapshot.flux->tracks){maximumHead=std::max(maximumHead,track.head);maximumTrack=std::max(maximumTrack,track.cylinder);}side_->setRange(0,maximumHead);side_->setValue(session_->surface);track_->setRange(0,maximumTrack);track_->setValue(static_cast<int>(session_->position));
        const auto found=std::find_if(snapshot.flux->tracks.begin(),snapshot.flux->tracks.end(),[this](const auto& item){return item.head==session_->surface && item.cylinder==session_->position;});QStringList summary,details;
        revolutions_->addItem(text(Synthesis));
        if(found!=snapshot.flux->tracks.end()){if(found->sourceNumber)summary<<text(ScpEntry)+QString::fromUtf8(ValueSeparator)+QString::number(*found->sourceNumber);summary<<text(TrackCount)+QString::fromUtf8(ValueSeparator)+QString::number(snapshot.flux->tracks.size());
            for(std::size_t index=0;index<found->revolutions.size();++index){const auto& value=found->revolutions[index];revolutions_->addItem(QString::number(index+1));details<<QString::number(index+1)+QString::fromUtf8(ValueSeparator)+QLocale().toString(static_cast<qulonglong>(value.transitions))+QLatin1Char(HexSeparator)+text(Transitions)+QString::fromUtf8(InfoSeparator)+QLocale().toString(value.milliseconds,PercentNumberFormat,FluxNumericPrecision)+QString::fromLatin1(FluxTimeSuffix)+QString::fromUtf8(InfoSeparator)+QLocale().toString(value.rpm,PercentNumberFormat,FluxNumericPrecision)+QString::fromLatin1(FluxRpmSuffix);}
        }
        revolutions_->setCurrentIndex(session_->fluxSynthesis?0:static_cast<int>(session_->revolution+1));
        if(session_->fluxInspection){const auto& inspection=*session_->fluxInspection;if(!inspection.decoderId.empty()){summary<<text(FormatDetected)+QString::fromUtf8(ValueSeparator)+text((std::string(DecoderPrefix)+inspection.decoderId).c_str());summary<<text(Confidence)+QString::fromUtf8(ValueSeparator)+QLocale().toString(inspection.confidence*PercentFactor,PercentNumberFormat,0)+QString::fromLatin1(Percent);summary<<text(Cell)+QString::fromUtf8(ValueSeparator)+QLocale().toString(inspection.cellTicks,PercentNumberFormat,FluxNumericPrecision)+QLatin1Char(HexSeparator)+text(Ticks);summary<<text(Structures)+QString::fromUtf8(ValueSeparator)+QString::number(inspection.details.size());details<<text(AnalysedRevolution).replace(QString::fromLatin1(FirstArgument),QString::number(inspection.revolution+1));}
            for(std::size_t index=0;index<std::min<std::size_t>(inspection.details.size(),FluxDetailLimit);++index){const auto& item=inspection.details[index];details<<text(item.kindKey.c_str())+QString::fromLatin1(StructureOffsetSeparator)+text(BitOffset).replace(QString::fromLatin1(NumberArgument),QLocale().toString(static_cast<qulonglong>(item.bitOffset)));}for(const auto& sector:inspection.sectors)rows_.push_back(&sector);
        }
        summary_->setText(summary.join(QString::fromUtf8(InfoSeparator)));fluxDetails_->setPlainText(details.join(QLatin1Char(LineSeparator)));
        for(std::size_t index=0;index<FluxLabels.size();++index){legendLabels_[index]->parentWidget()->show();legendLabels_[index]->setText(text(FluxLabels[index]));auto palette=legendDots_[index]->palette();palette.setColor(QPalette::WindowText,themes_.data()->color(FluxLegendBrushes[index]));legendDots_[index]->setPalette(palette);}
        for(int column=0;column<sectors_->columnCount();++column)sectors_->setColumnHidden(column,column==StateColumn);sectors_->setRowCount(static_cast<int>(rows_.size()));
        int selected=0;for(std::size_t index=0;index<rows_.size();++index){const auto& sector=*rows_[index];const QStringList values{QString::number(sector.logicalBlock),QString::number(sector.head),QString::number(sector.cylinder),QString::number(sector.number),QLocale().formattedDataSize(static_cast<qint64>(sector.size)),text(sector.dataValid?(*sector.dataValid?ValidKey:InvalidKey):UnavailableKey)};for(int column=0;column<values.size();++column)sectors_->setItem(static_cast<int>(index),column,new QTableWidgetItem(values[column]));if(session_->selectedSector==sector.logicalBlock)selected=static_cast<int>(index);}
        if(!rows_.empty()){sectors_->setCurrentCell(selected,0);showSector(selected);}else hex_->setBytes({});return;
    }
    if(snapshot.sequential){
        const auto& model=*snapshot.sequential;
        for(auto* control:{static_cast<QWidget*>(sideLabel_),static_cast<QWidget*>(side_),static_cast<QWidget*>(trackLabel_),static_cast<QWidget*>(track_),static_cast<QWidget*>(revolutionLabel_),static_cast<QWidget*>(revolutions_)})control->hide();
        QStringList summary;
        if(model.durationTicks)summary<<text(Duration)+QString::fromUtf8(ValueSeparator)+sequentialTime(*model.durationTicks);
        if(model.logicalLength)summary<<text(Capacity)+QString::fromUtf8(ValueSeparator)+QLocale().formattedDataSize(*model.logicalLength);
        summary<<text(Segments)+QString::fromUtf8(ValueSeparator)+QString::number(model.segments.size());
        for(std::size_t index=0;index<SequentialSummaryKeys.size();++index){const auto found=model.metadata.find(SequentialSummaryKeys[index]);if(found!=model.metadata.end() && !found->second.empty())summary<<text(SequentialSummaryLabels[index])+QString::fromUtf8(ValueSeparator)+QString::fromStdString(found->second);}
        summary_->setText(summary.join(QString::fromUtf8(InfoSeparator)));
        QStringList sequentialHeaders;for(const auto key:SequentialColumns)sequentialHeaders<<text(key);sectors_->setHorizontalHeaderLabels(sequentialHeaders);
        for(int column=0;column<sectors_->columnCount();++column)sectors_->setColumnHidden(column,false);
        for(std::size_t index=0;index<SequentialLabels.size();++index){legendLabels_[index]->setText(text(SequentialLabels[index]));auto palette=legendDots_[index]->palette();palette.setColor(QPalette::WindowText,themes_.data()->color(SequentialBrushes[index]));legendDots_[index]->setPalette(palette);legendLabels_[index]->parentWidget()->setVisible(!session_->showWaveform);}
        sectors_->setRowCount(static_cast<int>(model.segments.size()));
        for(std::size_t index=0;index<model.segments.size();++index){const auto& item=model.segments[index];const auto kind=static_cast<std::size_t>(item.kind);
            const QStringList values{item.startTicks?sequentialTime(*item.startTicks):QString::number(item.position),item.durationTicks?sequentialTime(*item.durationTicks):QString{},text(SequentialLabels[kind]),item.face?QString::number(*item.face):QString{},item.track?QString::number(*item.track):QString{},item.channel?QString::number(*item.channel):QString{},item.storedLength?QLocale().formattedDataSize(*item.storedLength):QString{}};
            for(int column=0;column<values.size();++column)sectors_->setItem(static_cast<int>(index),column,new QTableWidgetItem(values[column]));sectors_->item(static_cast<int>(index),InspectorColumn)->setForeground(themes_.data()->color(SequentialBrushes[kind]));
        }
        QStringList details;
        if(session_->selectedElement && *session_->selectedElement<model.segments.size()){const auto selected=*session_->selectedElement;sectors_->setCurrentCell(static_cast<int>(selected),0);const auto& item=model.segments[selected];for(std::size_t index=0;index<SequentialMetadataKeys.size();++index){const auto found=item.metadata.find(SequentialMetadataKeys[index]);if(found!=item.metadata.end() && !found->second.empty())details<<text(SequentialMetadataLabels[index])+QString::fromUtf8(ValueSeparator)+QString::fromStdString(found->second);}}
        elementDetails_->setText(details.join(QString::fromUtf8(InfoSeparator)));showBytes(session_->preview);return;
    }
    if(snapshot.linear){
        const auto& model=*snapshot.linear;const bool optical=descriptor.representationKind==MediaEngine::RepresentationKind::OpticalTracks;
        for(auto* control:{static_cast<QWidget*>(sideLabel_),static_cast<QWidget*>(side_),static_cast<QWidget*>(trackLabel_),static_cast<QWidget*>(track_),static_cast<QWidget*>(revolutionLabel_),static_cast<QWidget*>(revolutions_)})control->hide();
        QStringList summary{text(Capacity)+QString::fromUtf8(ValueSeparator)+QLocale().toString(static_cast<qlonglong>(model.logicalLength))+QLatin1Char(HexSeparator)+text(optical?SectorsUnit:BlocksUnit)};
        if(model.geometry)summary<<QString::fromUtf8(Geometry)+QString::fromUtf8(ValueSeparator)+QString::number(model.geometry->cylinders())+QString::fromUtf8(GeometrySeparator)+QString::number(model.geometry->heads())+QString::fromUtf8(GeometrySeparator)+QString::number(model.geometry->sectorsPerTrack());
        if(model.faces)summary<<text(Faces)+QString::fromUtf8(ValueSeparator)+QString::number(*model.faces);if(model.layers)summary<<text(Layers)+QString::fromUtf8(ValueSeparator)+QString::number(*model.layers);
        if(optical)summary<<text(TrackCount)+QString::fromUtf8(ValueSeparator)+QString::number(model.items.size());summary_->setText(summary.join(QString::fromUtf8(InfoSeparator)));
        QStringList linearHeaders;for(const auto key:LinearColumns)linearHeaders<<text(key);sectors_->setHorizontalHeaderLabels(linearHeaders);
        for(int column=0;column<sectors_->columnCount();++column)sectors_->setColumnHidden(column,!optical && (column==TrackColumn || column==SectorColumn));
        for(std::size_t index=0;index<SectorLabels.size();++index){legendLabels_[index]->parentWidget()->setVisible(index<BlockLabels.size());if(index<BlockLabels.size()){legendLabels_[index]->setText(text(optical?OpticalLabels[index]:BlockLabels[index]));auto palette=legendDots_[index]->palette();palette.setColor(QPalette::WindowText,themes_.data()->color(optical?OpticalBrushes[index]:BlockBrushes[index]));legendDots_[index]->setPalette(palette);}}
        sectors_->setRowCount(static_cast<int>(model.items.size()));
        for(std::size_t index=0;index<model.items.size();++index){const auto& item=model.items[index];const auto& fields=descriptor.elements[index].fields();const auto stateIndex=optical?static_cast<std::size_t>(item.opticalKind):static_cast<std::size_t>(item.blockState);
            const QStringList values{QString::number(fields.position),QString::number(fields.length.value_or(1)),fields.sessionNumber?QString::number(*fields.sessionNumber):QString{},fields.trackNumber?QString::number(*fields.trackNumber):QString{},text(optical?OpticalLabels[stateIndex]:BlockLabels[stateIndex]),item.volume && item.volume->partitionTable?QString::fromStdString(*item.volume->partitionTable):QString{},item.volume && item.volume->fileSystemId?QString::fromStdString(*item.volume->fileSystemId):QString{}};
            for(int column=0;column<values.size();++column)sectors_->setItem(static_cast<int>(index),column,new QTableWidgetItem(values[column]));sectors_->item(static_cast<int>(index),LinearStateColumn)->setForeground(themes_.data()->color(optical?OpticalBrushes[stateIndex]:BlockBrushes[stateIndex]));
        }
        QStringList details;
        if(session_->selectedElement && *session_->selectedElement<model.items.size()){const auto selected=*session_->selectedElement;sectors_->setCurrentCell(static_cast<int>(selected),0);const auto& item=model.items[selected];if(item.volume){details<<text(Origin)+QString::fromUtf8(ValueSeparator)+QString::fromStdString(item.volume->origin);if(item.volume->partitionNumber)details<<text(PartitionNumber)+QString::fromUtf8(ValueSeparator)+QString::number(*item.volume->partitionNumber);}if(item.storedSectorSize)details<<text(Size)+QString::fromUtf8(ValueSeparator)+QLocale().formattedDataSize(*item.storedSectorSize);if(item.pregap)details<<text(Pregap)+QString::fromUtf8(ValueSeparator)+QString::number(*item.pregap);}
        if(!model.associatedFiles.empty()){details<<text(AssociatedFiles);for(const auto& file:model.associatedFiles)details<<QString::fromStdString(file);}elementDetails_->setText(details.join(QString::fromUtf8(InfoSeparator)));showBytes(session_->preview);return;
    }
    const bool logical=snapshot.sectors && snapshot.sectors->logical;
    const bool coordinatesVisible=!logical && (descriptor.representationKind==MediaEngine::RepresentationKind::Flux || descriptor.representationKind==MediaEngine::RepresentationKind::Sectors);
    for(int column:{SideColumn,TrackColumn,SectorColumn})sectors_->setColumnHidden(column,logical);sectors_->setColumnHidden(StateColumn,!snapshot.sectors);
    for(auto* control:{static_cast<QWidget*>(sideLabel_),static_cast<QWidget*>(side_),static_cast<QWidget*>(trackLabel_),static_cast<QWidget*>(track_),static_cast<QWidget*>(revolutionLabel_),static_cast<QWidget*>(revolutions_)})control->setVisible(coordinatesVisible);
    const auto maximumSurface=descriptor.surfaces.empty()?0:*std::max_element(descriptor.surfaces.begin(),descriptor.surfaces.end());
    side_->setRange(0,maximumSurface);side_->setValue(session_->surface);
    std::int32_t maximumCylinder=0;
    if(snapshot.source->diskExploration) {
        const auto& disk=snapshot.source->diskExploration->info();
        for(const auto& format:disk.formats) if(format.formatId==disk.primaryFormatId || disk.formats.size()==1) {
            if(snapshot.sectors){
                maximumCylinder=static_cast<std::int32_t>(snapshot.sectors->cylinders? snapshot.sectors->cylinders-1:0);
                for(const auto& item:snapshot.sectors->sectors)if(logical || (item.head==session_->surface && item.cylinder==session_->position)){if(item.sourceIndex<format.sectors.size()){rows_.push_back(&format.sectors[item.sourceIndex]);rowStates_.push_back(item.state);}}
            }else for(const auto& sector:format.sectors){maximumCylinder=std::max(maximumCylinder,static_cast<std::int32_t>(sector.cylinder));if(sector.head==session_->surface && sector.cylinder==session_->position)rows_.push_back(&sector);}
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
        const QStringList values{QString::number(sector.logicalBlock),QString::number(sector.head),QString::number(sector.cylinder),QString::number(sector.number),QLocale().formattedDataSize(static_cast<qint64>(sector.size)),text(integrity),index<rowStates_.size()?text(SectorLabels[static_cast<std::size_t>(rowStates_[index])]):QString{}};
        for(int column=0;column<values.size();++column)sectors_->setItem(static_cast<int>(index),column,new QTableWidgetItem(values[column]));
        if(index<rowStates_.size())sectors_->item(static_cast<int>(index),StateColumn)->setForeground(themes_.data()->color(SectorBrushes[static_cast<std::size_t>(rowStates_[index])]));
    }
    if(!rows_.empty()) {
        int selected=0;
        if(session_->selectedSector)for(std::size_t index=0;index<rows_.size();++index)if(rows_[index]->logicalBlock==*session_->selectedSector) {selected=static_cast<int>(index);break;}
        sectors_->setCurrentCell(selected,0);showSector(selected);
    } else hex_->setBytes({});
}
void VisualizationPage::showSector(int row) {
    if(row<0 || static_cast<std::size_t>(row)>=rows_.size() || !rows_[row]->data) {hex_->setBytes({});return;}
    session_->selectedSector=rows_[row]->logicalBlock;
    showBytes(*rows_[row]->data);
}
void VisualizationPage::showBytes(const std::vector<std::uint8_t>& bytes) {
    const auto count=std::min(bytes.size(),static_cast<std::size_t>(HexPreviewBytes));hex_->setBytes(std::vector<std::uint8_t>(bytes.begin(),bytes.begin()+count));
}
}
