#include "Shared/Presentation/AssetPath.h"
#include <RetroMediaLab/Translations/TranslationConstants.h>
#include "MediaPageConstants.h"
#include "Shared/Controls/RememberedFileDialog.h"
#include "VisualizationPage.h"
#include "MediaLoadingOverlay.h"
#include "LoadingConstants.h"
#include "../DisplayDensity.h"
#include <Constants/DisplayConstants.h>
#include "ClassificationPanel.h"
#include "ClassificationConstants.h"
#include <QToolButton>
#include "VisualizationConstants.h"
#include "Shared/Controls/Card.h"
#include <QGridLayout>
#include "Shared/Localization/MediaText.h"
#include <Errors.h>
#include <QComboBox>
#include <QCheckBox>
#include <set>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLocale>
#include "Shared/Controls/HexGrid.h"
#include <QPushButton>
#include "CaptureProgressView.h"
#include <QFileInfo>
#include <QFile>
#include <QEvent>
#include <tuple>
#include "../ThemeIcons.h"
#include "../ThemeConstants.h"
#include <QSignalBlocker>
#include <QSpinBox>
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
    auto* header = new Card(themes.data(),this);
    auto* headerLayout = new QGridLayout(header);
    headerLayout->setColumnStretch(HeaderTitleColumn, 1);
    auto* actions = new QHBoxLayout;
    auto* viewControls = new QHBoxLayout;
    mediaIcon_ = new QLabel(header);
    mediaIcon_->setFixedSize(MediaPageConstants::IdentityIconSize, MediaPageConstants::IdentityIconSize);
    auto iconPolicy = mediaIcon_->sizePolicy();
    iconPolicy.setRetainSizeWhenHidden(true);
    mediaIcon_->setSizePolicy(iconPolicy);
    headerLayout->addWidget(mediaIcon_, HeaderTitleRow, HeaderIconColumn, HeaderSummaryRow + 1, 1, Qt::AlignTop);
    status_ = new QLabel(header);
    status_->setTextFormat(Qt::PlainText);
    auto titleFont = status_->font();
    titleFont.setBold(true);
    status_->setFont(titleFont);
    status_->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);
    summary_ = new QLabel(header);
    summary_->setTextFormat(Qt::PlainText);
    summary_->setWordWrap(true);
    summary_->setProperty(DisplayConstants::ContentHeightProperty, true);
    summary_->installEventFilter(this);
    summary_->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    headerLayout->addWidget(status_, HeaderTitleRow, HeaderTitleColumn);
    headerLayout->addWidget(summary_, HeaderSummaryRow, HeaderTitleColumn, 1, HeaderContentColumns);
    headerLayout->addLayout(actions, HeaderTitleRow, HeaderActionsColumn);
    headerLayout->addLayout(viewControls, HeaderOptionsRow, HeaderIconColumn, 1, HeaderColumnCount);
    revolutionLabel_ = new QLabel(header);
    revolutions_ = new QComboBox(header);
    open_ = new QPushButton(this); reset_ = new QPushButton(this);
    zoomOut_ = new QPushButton(QString::fromUtf8(ZoomOut),this); zoomIn_ = new QPushButton(QString::fromUtf8(ZoomIn),this); zoom_ = new QLabel(this);
    representation_=new QComboBox(this);representation_->addItems({QString{},QString{}});
    linkZoom_=new QCheckBox(this);
    classificationToggle_=new QToolButton(this);classificationToggle_->setObjectName(ClassificationConstants::ToggleObject);classificationToggle_->setCheckable(true);
    viewControls->addWidget(representation_);
    diskFormat_ = new QComboBox(header);
    viewControls->addWidget(diskFormat_);
    connect(diskFormat_, &QComboBox::activated, this, [this](int index) {
        const auto id=diskFormat_->itemData(index).toString();
        session_->setDiskFormat(id.isEmpty()?std::nullopt:std::optional{id.toStdString()});
    });
    viewControls->addWidget(revolutionLabel_);
    viewControls->addWidget(revolutions_);
    viewControls->addWidget(linkZoom_);
    actions->addWidget(reset_);
    actions->addWidget(open_);
    actions->addWidget(classificationToggle_);
    viewControls->addStretch();
    layout->addWidget(header);
    classification_=new ClassificationPanel(session_->classification,translations,false,[this](bool){session_->applyClassification();},this);layout->addWidget(classification_);
    connect(classificationToggle_,&QToolButton::toggled,this,[this](bool visible){session_->classification.expanded=visible;classification_->setVisible(visible);});
    auto* graphCard = new Card(themes.data(),this);
    auto* graphLayout = new QVBoxLayout(graphCard);
    auto* graphControls = new QHBoxLayout;
    graphControls->addStretch();
    graphControls->addWidget(zoomOut_);
    graphControls->addWidget(zoom_);
    graphControls->addWidget(zoomIn_);
    graphLayout->addLayout(graphControls);
    graph_ = new MediaGraph(themes,session_,graphCard);
    graphLayout->addWidget(graph_,1);
    layout->addWidget(graphCard,1);
    auto* inspector = new Card(themes.data(),this);
    auto* details = new QVBoxLayout(inspector); details->setContentsMargins(Padding,Padding,Padding,Padding);
    auto* coordinates = new QHBoxLayout;
    sideLabel_ = new QLabel(inspector); trackLabel_ = new QLabel(inspector);
    side_ = new QSpinBox(inspector); track_ = new QSpinBox(inspector);
    coordinates->addWidget(sideLabel_); coordinates->addWidget(side_); coordinates->addWidget(trackLabel_); coordinates->addWidget(track_);
    decoderLabel_=new QLabel(inspector);decoder_=new QComboBox(inspector);decoder_->setMaximumWidth(FluxDecoderMaximumWidth);coordinates->addWidget(decoderLabel_);coordinates->addWidget(decoder_); coordinates->addStretch(); details->addLayout(coordinates);
    elementDetails_=new QLabel(inspector);elementDetails_->setWordWrap(true);elementDetails_->setTextFormat(Qt::PlainText);details->addWidget(elementDetails_);
    auto* inspectorToggle = new QToolButton(inspector);
    inspectorToggle->setArrowType(Qt::UpArrow);
    inspectorToggle->setCheckable(true);
    inspectorToggle->setChecked(true);
    inspectorToggle->setAccessibleName(text(Sector));
    coordinates->addWidget(inspectorToggle);

    auto* contents = new QWidget(inspector);
    contents->setFixedHeight(InspectorDataHeight);
    auto* dataLayout = new QHBoxLayout(contents);
    dataLayout->setContentsMargins(0,0,0,0);
    dataLayout->setSpacing(Gap);
    connect(inspectorToggle, &QToolButton::toggled, this, [contents, inspectorToggle](bool expanded) {
        contents->setVisible(expanded);
        inspectorToggle->setArrowType(expanded ? Qt::UpArrow : Qt::DownArrow);
    });
    sectors_ = new QTableWidget(contents); sectors_->setColumnCount(static_cast<int>(Columns.size()));
    sectors_->setEditTriggers(QAbstractItemView::NoEditTriggers); sectors_->setSelectionBehavior(QAbstractItemView::SelectRows); sectors_->setSelectionMode(QAbstractItemView::SingleSelection);
    sectors_->verticalHeader()->hide(); sectors_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    faceInspector_ = new QWidget(contents);
    auto* faces = new QVBoxLayout(faceInspector_);
    faces->setContentsMargins(0,0,0,0);
    auto* faceCards = new QHBoxLayout;
    for (int face = 0; face < InspectorFaces; ++face) {
        auto* card = new QWidget(faceInspector_);
        auto* column = new QHBoxLayout(card);
        column->setContentsMargins(0,0,0,0);
        column->setSpacing(Gap);
        faceTitles_[face] = new QLabel(card);
        faceDetails_[face] = new QLabel(card);
        faceDetails_[face]->setTextFormat(Qt::PlainText);
        column->addWidget(faceTitles_[face]);
        column->addWidget(faceDetails_[face],1);
        faceCards->addWidget(card,1);
    }
    faces->addLayout(faceCards);
    faces->addWidget(sectors_,1);
    hex_ = new HexGrid(contents);
    faceInspector_->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);
    hex_->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);
    dataLayout->addWidget(faceInspector_,1);
    dataLayout->addWidget(hex_,1);
    details->addWidget(contents);
    layout->addWidget(inspector);
    overviewBar_=new Card(themes.data(),this);
    auto* overviewLayout=new QHBoxLayout(overviewBar_);
    legend_=new QWidget(overviewBar_);auto* legendLayout=new QGridLayout(legend_);legendLayout->setContentsMargins(0,0,0,0);legendLayout->setHorizontalSpacing(Gap);legendLayout->setVerticalSpacing(0);
    for(std::size_t index=0;index<legendLabels_.size();++index){auto* item=new QWidget(legend_);auto* row=new QHBoxLayout(item);row->setContentsMargins(0,0,0,0);row->setSpacing(Gap);legendDots_[index]=new QLabel(QString::fromUtf8(LegendMark),item);legendLabels_[index]=new QLabel(item);auto compactFont=legendLabels_[index]->font();compactFont.setPixelSize(LoadingConstants::StripFont);legendDots_[index]->setFont(compactFont);legendLabels_[index]->setFont(compactFont);row->addWidget(legendDots_[index]);row->addWidget(legendLabels_[index]);legendLayout->addWidget(item,static_cast<int>(index)/LegendColumns,static_cast<int>(index)%LegendColumns);}
    overviewLayout->addWidget(legend_);
    overview_=new CaptureProgressView(themes,overviewBar_,true);
    overviewLayout->addWidget(overview_,1);
    auto overviewPolicy = overviewBar_->sizePolicy();
    overviewPolicy.setRetainSizeWhenHidden(true);
    overviewBar_->setSizePolicy(overviewPolicy);
    overviewBar_->setProperty(DisplayConstants::ContentHeightProperty, true);
    layout->addWidget(overviewBar_);
    themeSubscription_=themes.subscribe([this]{refresh();});
    connect(open_,&QPushButton::clicked,this,[this,&state] {
        const auto path=RememberedFileDialog::choose(this,state,text(Open),FileDialogScope::Visualization,state.defaultImageDirectory()); if(!path.isEmpty()) session_->opening()->open(path);
    });
    connect(representation_,&QComboBox::currentIndexChanged,this,[this](int index){if(index>=0)session_->setWaveform(index!=0);});
    connect(linkZoom_,&QCheckBox::toggled,this,[this](bool value){session_->setLinkedZoom(value);});
    connect(reset_,&QPushButton::clicked,this,[this]{session_->setZoom(InitialZoom);});
    connect(zoomIn_,&QPushButton::clicked,this,[this]{session_->setZoom(std::min(MaximumZoom,session_->zoom*ZoomStep));});
    connect(zoomOut_,&QPushButton::clicked,this,[this]{session_->setZoom(std::max(InitialZoom,session_->zoom/ZoomStep));});
    connect(side_,&QSpinBox::valueChanged,this,[this](int value){session_->select(value,session_->position);});
    connect(track_,&QSpinBox::valueChanged,this,[this](int value){session_->select(session_->surface,value);});
    connect(revolutions_,&QComboBox::currentIndexChanged,this,[this](int value){if(value<0)return;if(session_->displayedFlux()){session_->setRevolution(value==0?0:static_cast<std::size_t>(value-1),value==0);}else session_->setRevolution(static_cast<std::size_t>(value));});
    connect(decoder_,&QComboBox::currentIndexChanged,this,[this](int index){if(index<0)return;const auto id=decoder_->itemData(index).toString();session_->setFluxDecoder(id.isEmpty()?std::nullopt:std::optional<std::string>(id.toStdString()));});
    connect(sectors_,&QTableWidget::currentCellChanged,this,[this](int row){if(row>=0 && session_->snapshot() && (session_->snapshot()->linear || session_->snapshot()->sequential)){session_->inspect(static_cast<std::size_t>(row));return;}if(row>=0 && static_cast<std::size_t>(row)<rows_.size() && session_->displayedSectors())session_->selectSector(rows_[row]->logicalBlock);else { showSector(row); if (graph_) graph_->update(); }});
    loading_=new MediaLoadingOverlay(themes,translations,[this]{session_->cancel();},graphCard,true,true);
    subscription_=session_->subscribe([this]{if(session_->presentingTracks()||session_->presentingLinear())refreshLoading();else refresh();});
    openingSubscription_=session_->opening()->subscribe([this]{if(session_->opening()->busy())refreshLoading();else refresh();});
    languageSubscription_=translations.subscribe([this]{refresh();});
    refresh();
}
void VisualizationPage::setSummary(const QString& value) {
    if (summary_->text() != value) summary_->setText(value);
    resizeSummary();
}
void VisualizationPage::resizeSummary() {
    const auto available = summary_->contentsRect();
    const auto metrics = summary_->fontMetrics();
    const auto required = metrics.boundingRect(available, Qt::TextWordWrap, summary_->text()).height();
    const auto height = std::max(metrics.height(), required);
    if (summary_->minimumHeight() != height) summary_->setMinimumHeight(height);
}
bool VisualizationPage::eventFilter(QObject* object, QEvent* event) {
    if (object == summary_ && (event->type() == QEvent::Resize || event->type() == QEvent::FontChange))
        resizeSummary();
    return QWidget::eventFilter(object, event);
}
QString VisualizationPage::text(const char* key) const {return QString::fromStdString(translations_.text(key));}
bool VisualizationPage::updateScpIdentity(const Infrastructure::LoadingProgress& progress) {
    const MediaEngine::DiskFormat* format = nullptr;
    if (const auto& snapshot = session_->snapshot(); !session_->opening()->busy() && snapshot && snapshot->source->diskExploration) {
        const auto& disk = snapshot->source->diskExploration->info();
        format = session_->classification.catalog().resolveFormat(session_->diskFormat().value_or(disk.primaryFormatId));
    }
    if (!progress.capture && !format) {
        mediaIcon_->hide();
        return false;
    }
    const auto iconId = MediaEngine::selectMediaIcon(MediaEngine::MediaKind::Floppy, {}, format);
    const auto pattern = assetPath(QString::fromLatin1(MediaPageConstants::ResourceIconPath));
    const auto resource = pattern.arg(QString::fromLatin1(MediaPageConstants::MediaIdentityPrefix)
        + QString::fromLatin1(iconId.data(), iconId.size()));
    const auto pixmap = QIcon(QFile::exists(resource) ? resource :
        pattern.arg(QString::fromLatin1(MediaPageConstants::FloppyIcon))).pixmap(mediaIcon_->size());
    mediaIcon_->setPixmap(pixmap);
    mediaIcon_->show();
    if (!progress.capture) return false;
    const auto& capture = *progress.capture;
    const auto& header = capture.header;
    setSummary(mediaText(translations_,{ScpSummary,{
        header.versionText(),std::to_string(capture.tracks.size()),std::to_string(header.revolutions),
        std::to_string(header.resolutionNanoseconds()),text(capture.checksumValid?ScpChecksumValid:ScpChecksumInvalid).toStdString()}, {}}));
    return true;
}
void VisualizationPage::refreshScpLegend() {
    legend_->show();
    for (std::size_t index = 0; index < FluxQualityLabels.size(); ++index) {
        legendLabels_[index]->parentWidget()->show();
        legendLabels_[index]->setText(text(FluxQualityLabels[index]));
        auto palette = legendDots_[index]->palette();
        palette.setColor(QPalette::WindowText,themes_.data()->color(FluxQualityBrushes[index]));
        legendDots_[index]->setPalette(palette);
    }
}
void VisualizationPage::refreshLoading(){
    constexpr auto legendRows = (LegendCapacity + LegendColumns - 1) / LegendColumns;
    const auto& overviewLayout = *overviewBar_->layout();
    const auto margins = overviewLayout.contentsMargins();
    const auto legendHeight = legendRows * legendLabels_.front()->fontMetrics().height();
    const auto stripHeight = qRound(InspectorFaces * LoadingConstants::StripRowHeight * DisplayDensity::factor(this));
    const auto overviewHeight = margins.top() + margins.bottom() + std::max(legendHeight, stripHeight);
    if (overviewBar_->height() != overviewHeight || overviewBar_->minimumHeight() != overviewHeight)
        overviewBar_->setFixedHeight(overviewHeight);
    const auto& tracks = session_->trackProgress();
    if (session_->presentingTracks() && session_->displayedFlux() && (!session_->snapshot() || session_->opening()->busy())) {
        sideLabel_->hide();
        side_->hide();
        trackLabel_->hide();
        track_->hide();
        revolutionLabel_->hide();
        revolutions_->hide();
        sectors_->hide();
        faceInspector_->show();
        for (int face = 0; face < InspectorFaces; ++face) {
            faceTitles_[face]->setText(mediaText(translations_,{LoadingConstants::Face,{std::to_string(face)}, {}}));
            faceTitles_[face]->parentWidget()->setVisible(tracks.cylindersPerHead.contains(face));
            faceDetails_[face]->clear();
        }
    }
    QStringList labels;
    for (const auto head : tracks.heads)
        labels << mediaText(translations_,{LoadingConstants::Face,{std::to_string(head)},{}});
    overview_->setProgress(tracks,labels,session_->presentingTracks());
    if (session_->snapshot() && session_->snapshot()->sequential)
        overview_->setSequential(*session_->snapshot()->sequential,session_->presentedSegments(),text(LoadingConstants::Tape));
    if (!session_->linearProgress().empty()) {
        const auto optical = session_->snapshot()->descriptor->fields().representationKind == MediaEngine::RepresentationKind::OpticalTracks;
        overview_->setLinear(session_->linearProgress(), text(optical ? LoadingConstants::Optical : LoadingConstants::Blocks));
    }
    overviewBar_->setVisible(session_->displayedFlux() || session_->snapshot());
auto progress=session_->opening()->progress();
    if (updateScpIdentity(progress)) {
        if(session_->displayedFlux()) refreshScpLegend();
        overviewBar_->show();
    } else if (session_->opening()->busy()) {
        setSummary({});
    }
    if(!session_->opening()->busy()&&session_->preparing()){progress=session_->presentationProgress();}const bool busy=session_->opening()->busy()||session_->preparing();loading_->display(busy,progress);if(busy){status_->setText(QFileInfo(session_->opening()->busy()?session_->opening()->loadingPath():session_->opening()->path()).fileName());open_->setEnabled(false);classification_->setEnabled(false);classificationToggle_->setEnabled(false);}}
void VisualizationPage::refresh() {
    refreshLoading();
    classificationToggle_->setIcon(themeIcon(VisualizationSettingsIcon,themes_.data()->color(ThemeConstants::TextBrush)));classificationToggle_->setToolTip(text(ClassificationConstants::Advanced));classificationToggle_->setAccessibleName(text(ClassificationConstants::Advanced));const bool classificationAvailable=session_->snapshot()&&session_->snapshot()->source->diskExploration;classificationToggle_->setEnabled(classificationAvailable&&!session_->busy());{const QSignalBlocker blocked(classificationToggle_);classificationToggle_->setChecked(session_->classification.expanded);}classification_->refresh();classification_->setVisible(classificationAvailable&&session_->classification.expanded);classification_->setEnabled(!session_->busy());
    linkZoom_->setText(text(LinkZoom));{const QSignalBlocker blocked(linkZoom_);linkZoom_->setChecked(session_->linkedZoom());}
    std::set<std::int32_t> heads;if(session_->snapshot()){if(session_->snapshot()->flux)for(const auto& track:session_->snapshot()->flux->tracks)heads.insert(track.head);else if(session_->snapshot()->sectors&&!session_->snapshot()->sectors->logical)for(const auto& sector:session_->snapshot()->sectors->sectors)heads.insert(static_cast<std::int32_t>(sector.head));}linkZoom_->setVisible(heads.size()>1);
    const auto actionColor = themes_.data()->color(ThemeConstants::TextBrush);
    open_->setIcon(themeIcon(MediaPageConstants::OpenIcon, actionColor));
    reset_->setIcon(themeIcon(MediaPageConstants::RefreshIcon, actionColor));
    open_->setText(text(Open)); reset_->setText(text(Reset)); sideLabel_->setText(text(Side)); trackLabel_->setText(text(Track)); revolutionLabel_->setText(text(Revolution));
    zoom_->setText(QLocale().toString(session_->zoom*PercentFactor,PercentNumberFormat,0)+QString::fromLatin1(Percent));
    open_->setEnabled(!session_->opening()->busy());
    const QSignalBlocker blockedRepresentation(representation_);
    representation_->setAccessibleName(text(DisplayMode));representation_->setItemText(0,text(Segments));representation_->setItemText(1,text(Signal));
    representation_->setVisible(session_->snapshot() && session_->snapshot()->sequential.has_value());
    representation_->setCurrentIndex(session_->showWaveform?1:0);
    representation_->setEnabled(session_->snapshot() && session_->snapshot()->tape && !session_->snapshot()->tape->samples().empty());
    status_->setText(session_->opening()->busy() || session_->preparing()?text(Loading):session_->opening()->path().isEmpty()?text(Empty):QFileInfo(session_->opening()->path()).fileName());
    const auto error=session_->error()?session_->error():session_->snapshot()?session_->snapshot()->warning:std::optional<MediaEngine::MessageInfo>{};
    if (error) {
        status_->setText(mediaText(translations_,*error));
    } else if(session_->opening()->errorMessage()) status_->setText(mediaText(translations_,*session_->opening()->errorMessage()));
    else if(!session_->opening()->error().isEmpty()) status_->setText(QString::fromStdString(Errors::render(translations_.language(),session_->opening()->error().toStdString())));
    QStringList headers; for(const auto key:Columns)headers.push_back(text(key)); sectors_->setHorizontalHeaderLabels(headers);
    const QSignalBlocker blockedSectors(sectors_),blockedSide(side_),blockedTrack(track_),blockedRevolutions(revolutions_);
    sectors_->clearContents(); sectors_->setRowCount(0); rows_.clear();rowStates_.clear(); revolutions_->clear();
    const bool scpIdentity = updateScpIdentity(session_->opening()->progress());
    if (!scpIdentity) setSummary({});
    elementDetails_->clear(); elementDetails_->hide();
    const bool flux=session_->snapshot() && session_->displayedFlux();
    const auto* sectorModel=session_->displayedSectors();
    decoder_->setVisible(flux);decoderLabel_->setVisible(flux);decoderLabel_->setText(text(Decoder));
    faceInspector_->setVisible(true);
    for (auto* title : faceTitles_) title->parentWidget()->setVisible(flux);
    sectors_->setVisible(true);
    for(int column=0;column<sectors_->columnCount();++column) sectors_->setColumnHidden(column,false);
    const QSignalBlocker blockedDecoder(decoder_);decoder_->clear();
    for(auto* label:legendLabels_)label->parentWidget()->hide();
    legend_->setVisible(session_->snapshot() && (session_->snapshot()->sectors.has_value() || session_->snapshot()->linear.has_value() || session_->snapshot()->sequential.has_value() || flux));
    for(std::size_t index=0;index<SectorLabels.size();++index){legendLabels_[index]->setText(text(SectorLabels[index]));auto palette=legendDots_[index]->palette();palette.setColor(QPalette::WindowText,themes_.data()->color(SectorBrushes[index]));legendDots_[index]->setPalette(palette);}
    for(std::size_t index=0;index<SectorLabels.size();++index)legendLabels_[index]->parentWidget()->show();
    if(flux)refreshScpLegend();
    if(!session_->snapshot()) {diskFormat_->hide();hex_->setBytes({});return;}
    const auto& snapshot=*session_->snapshot();
    const auto& descriptor=snapshot.descriptor->fields();
    const QSignalBlocker blockedFormat(diskFormat_);
    diskFormat_->clear();
    diskFormat_->setVisible(snapshot.flux.has_value() && snapshot.source->diskExploration);
    diskFormat_->setEnabled(!session_->preparing() && !session_->opening()->busy());
    if (snapshot.flux && snapshot.source->diskExploration) {
        diskFormat_->addItem(text(RawFlux),QString{});
        QStringList formats;
        for (const auto& id : snapshot.source->diskExploration->info().detectedFormatIds) {
            const auto* format = session_->classification.catalog().resolveFormat(id);
            if (!format) continue;
            const auto name = QString::fromStdString(MediaEngine::imageFormatName(*format, [this](std::string_view key) {
                return translations_.text(std::string(key), TranslationConstants::MediaEngine);
            }));
            formats << name;
            diskFormat_->addItem(name,QString::fromStdString(id));
        }
        diskFormat_->setCurrentIndex(session_->diskFormat()?diskFormat_->findData(QString::fromStdString(*session_->diskFormat())):0);
        if (!formats.isEmpty()) {
            const auto detected = mediaText(translations_, {MediaPageConstants::DetectedFormats,
                {formats.join(QString::fromUtf8(MediaPageConstants::FormatsSeparator)).toStdString()}, {}});
            const auto identity = summary_->text();
            setSummary(identity.isEmpty() ? detected : identity + QLatin1Char(LoadingConstants::LineBreak) + detected);
        }
    }
    if(flux){
        decoder_->addItem(text(Automatic),QString{});for(const auto& id:snapshot.flux->decoders)decoder_->addItem(text((std::string(DecoderPrefix)+id).c_str()),QString::fromStdString(id));decoder_->setCurrentIndex(session_->fluxDecoder?decoder_->findData(QString::fromStdString(*session_->fluxDecoder)):0);
        for (auto* control : {static_cast<QWidget*>(sideLabel_),static_cast<QWidget*>(side_),
                static_cast<QWidget*>(trackLabel_),static_cast<QWidget*>(track_),static_cast<QWidget*>(decoderLabel_),
                static_cast<QWidget*>(decoder_)})
            control->hide();
        revolutionLabel_->show();
        revolutions_->show();
        revolutions_->addItem(text(Synthesis));
        std::size_t revolutionCount = 0;
        for (const auto& track : snapshot.flux->tracks)
            revolutionCount = std::max(revolutionCount, track.revolutions.size());
        for (std::size_t index = 0; index < revolutionCount; ++index)
            revolutions_->addItem(QString::number(index + 1));
        revolutions_->setCurrentIndex(session_->fluxSynthesis ? 0 : static_cast<int>(session_->revolution + 1));
        if (session_->fluxInspection) {
            const auto& inspection = *session_->fluxInspection;
            if (!scpIdentity && summary_->text().isEmpty() && !inspection.decoderId.empty())
                setSummary(text(FormatDetected) + QString::fromUtf8(ValueSeparator) +
                    text((std::string(DecoderPrefix) + inspection.decoderId).c_str()));
            for (const auto& sector : inspection.sectors) rows_.push_back(&sector);
        }
        for (int face = 0; face < InspectorFaces; ++face) {
            auto* title = faceTitles_[face];
            title->setText(mediaText(translations_,{LoadingConstants::Face,{std::to_string(face)}, {}}));
            QStringList values;
            const auto found = std::find_if(snapshot.flux->tracks.begin(), snapshot.flux->tracks.end(), [this,face](const auto& track) {
                return track.head == face && track.cylinder == session_->position;
            });
            if (found != snapshot.flux->tracks.end()) {
                if (!found->revolutions.empty()) {
                    const auto index = std::min(session_->revolution, found->revolutions.size() - 1);
                    const auto& revolution = found->revolutions[index];
                    values << QLocale().toString(revolution.milliseconds,PercentNumberFormat,FluxNumericPrecision) +
                        QString::fromLatin1(FluxTimeSuffix);
                    values << QLocale().toString(revolution.rpm,PercentNumberFormat,FluxNumericPrecision) +
                        QString::fromLatin1(FluxRpmSuffix);
                }
            }
            title->parentWidget()->setVisible(session_->trackProgress().cylindersPerHead.contains(face));
            faceDetails_[face]->setText(values.join(QString::fromUtf8(InfoSeparator)));
        }
        std::stable_sort(rows_.begin(),rows_.end(),[](const auto* left,const auto* right){
            return std::tie(left->number,left->logicalBlock)<std::tie(right->number,right->logicalBlock);
        });
        sectors_->setColumnHidden(StateColumn,true);
        side_->setRange(0,heads.empty()?0:*heads.rbegin()); side_->setValue(session_->surface);
        int lastCylinder=0;
        for(const auto& track:snapshot.flux->tracks) if(track.head==session_->surface) lastCylinder=std::max(lastCylinder,track.cylinder);
        track_->setRange(0,lastCylinder); track_->setValue(static_cast<int>(session_->position));
        sideLabel_->show(); side_->show(); trackLabel_->show(); track_->show();
        fillSectorTable();
        return;
    }
    if(snapshot.sequential){
        const auto& model=*snapshot.sequential;
        for(auto* control:{static_cast<QWidget*>(sideLabel_),static_cast<QWidget*>(side_),static_cast<QWidget*>(trackLabel_),static_cast<QWidget*>(track_),static_cast<QWidget*>(revolutionLabel_),static_cast<QWidget*>(revolutions_)})control->hide();
        QStringList summary;
        if(model.durationTicks)summary<<text(Duration)+QString::fromUtf8(ValueSeparator)+sequentialTime(*model.durationTicks);
        if(model.logicalLength)summary<<text(Capacity)+QString::fromUtf8(ValueSeparator)+QLocale().formattedDataSize(*model.logicalLength);
        summary<<text(Segments)+QString::fromUtf8(ValueSeparator)+QString::number(model.segments.size());
        for(std::size_t index=0;index<SequentialSummaryKeys.size();++index){const auto found=model.metadata.find(SequentialSummaryKeys[index]);if(found!=model.metadata.end() && !found->second.empty())summary<<text(SequentialSummaryLabels[index])+QString::fromUtf8(ValueSeparator)+QString::fromStdString(found->second);}
        setSummary(summary.join(QString::fromUtf8(InfoSeparator)));
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
        elementDetails_->setText(details.join(QString::fromUtf8(InfoSeparator)));elementDetails_->setVisible(!details.isEmpty());showBytes(session_->preview);return;
    }
    if(snapshot.linear){
        const auto& model=*snapshot.linear;const bool optical=descriptor.representationKind==MediaEngine::RepresentationKind::OpticalTracks;
        for(auto* control:{static_cast<QWidget*>(sideLabel_),static_cast<QWidget*>(side_),static_cast<QWidget*>(trackLabel_),static_cast<QWidget*>(track_),static_cast<QWidget*>(revolutionLabel_),static_cast<QWidget*>(revolutions_)})control->hide();
        QStringList summary{text(Capacity)+QString::fromUtf8(ValueSeparator)+QLocale().toString(static_cast<qlonglong>(model.logicalLength))+QLatin1Char(HexSeparator)+text(optical?SectorsUnit:BlocksUnit)};
        if(model.geometry)summary<<QString::fromUtf8(Geometry)+QString::fromUtf8(ValueSeparator)+QString::number(model.geometry->cylinders())+QString::fromUtf8(GeometrySeparator)+QString::number(model.geometry->heads())+QString::fromUtf8(GeometrySeparator)+QString::number(model.geometry->sectorsPerTrack());
        if(model.faces)summary<<text(Faces)+QString::fromUtf8(ValueSeparator)+QString::number(*model.faces);if(model.layers)summary<<text(Layers)+QString::fromUtf8(ValueSeparator)+QString::number(*model.layers);
        if(optical)summary<<text(TrackCount)+QString::fromUtf8(ValueSeparator)+QString::number(model.items.size());setSummary(summary.join(QString::fromUtf8(InfoSeparator)));
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
        if(!model.associatedFiles.empty()){details<<text(AssociatedFiles);for(const auto& file:model.associatedFiles)details<<QString::fromStdString(file);}elementDetails_->setText(details.join(QString::fromUtf8(InfoSeparator)));elementDetails_->setVisible(!details.isEmpty());showBytes(session_->preview);return;
    }
    const bool logical=sectorModel && sectorModel->logical;
    const bool coordinatesVisible=!logical && (descriptor.representationKind==MediaEngine::RepresentationKind::Flux || descriptor.representationKind==MediaEngine::RepresentationKind::Sectors);
    for(int column:{SideColumn,TrackColumn,SectorColumn})sectors_->setColumnHidden(column,logical);sectors_->setColumnHidden(StateColumn,!sectorModel);
    for(auto* control:{static_cast<QWidget*>(sideLabel_),static_cast<QWidget*>(side_),static_cast<QWidget*>(trackLabel_),static_cast<QWidget*>(track_)})control->setVisible(coordinatesVisible);
    revolutionLabel_->hide(); revolutions_->hide();
    const auto maximumSurface=descriptor.surfaces.empty()?0:*std::max_element(descriptor.surfaces.begin(),descriptor.surfaces.end());
    side_->setRange(0,maximumSurface);side_->setValue(session_->surface);
    std::int32_t maximumCylinder=0;
    if(snapshot.source->diskExploration) {
        const auto& disk=snapshot.source->diskExploration->info();
        for(const auto& format:disk.formats) if(format.formatId==session_->diskFormat().value_or(disk.primaryFormatId) || disk.formats.size()==1) {
            if(sectorModel){
                maximumCylinder=static_cast<std::int32_t>(sectorModel->cylinders? sectorModel->cylinders-1:0);
                for(const auto& item:sectorModel->sectors)if(logical || (item.head==session_->surface && item.cylinder==session_->position)){if(item.sourceIndex<format.sectors.size()){rows_.push_back(&format.sectors[item.sourceIndex]);rowStates_.push_back(item.state);}}
            }else for(const auto& sector:format.sectors){maximumCylinder=std::max(maximumCylinder,static_cast<std::int32_t>(sector.cylinder));if(sector.head==session_->surface && sector.cylinder==session_->position)rows_.push_back(&sector);}
            break;
        }
        if (!sectorModel) for(const auto& track:disk.tracks) {
            maximumCylinder=std::max(maximumCylinder,track.cylinder);
            if(track.head==session_->surface && track.cylinder==session_->position)
                for(const auto& revolution:track.revolutions)revolutions_->addItem(QString::number(revolution.number));
        }
    }
    track_->setRange(0,maximumCylinder);track_->setValue(static_cast<int>(session_->position));
    revolutions_->setCurrentIndex(static_cast<int>(session_->revolution));
    fillSectorTable();
}
void VisualizationPage::fillSectorTable() {
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
        sectors_->setCurrentCell(selected,0);
        const auto alreadySelected=session_->selectedSector;
        showSector(selected);
        session_->selectedSector=alreadySelected;
    } else hex_->setBytes({});
}
void VisualizationPage::showSector(int row) {
    if(row<0 || static_cast<std::size_t>(row)>=rows_.size() || !rows_[row]->data) {hex_->setBytes({});return;}
    if (session_->displayedFlux()) session_->selectedSector=rows_[row]->logicalBlock;
    showBytes(*rows_[row]->data);
}
void VisualizationPage::showBytes(const std::vector<std::uint8_t>& bytes) {
    const auto count=std::min(bytes.size(),static_cast<std::size_t>(HexPreviewBytes));hex_->setBytes(std::vector<std::uint8_t>(bytes.begin(),bytes.begin()+count));
}
}
