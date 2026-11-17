#include "MigrationDialog.h"
#include "MigrationDialogConstants.h"
#include "../ThemeConstants.h"
#include "Shared/Controls/Card.h"
#include "Shared/Localization/MediaText.h"
#include <RetroMediaLab/Platform/WindowAppearance.h>
#include <algorithm>
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QScreen>
#include <QSignalBlocker>
#include <QTreeWidget>
#include <QVBoxLayout>
namespace RetroMediaLab {
namespace C=MigrationDialogConstants;
MigrationDialog::MigrationDialog(ThemeService& themes,Translations::TranslationService& translations,WorkspaceState& state,QWidget* parent)
 :QDialog(parent),themes_(themes),translations_(translations),state_(state),session_(state.migration){
    setObjectName(C::ObjectName);setWindowModality(Qt::WindowModal);setWindowIcon(parent->windowIcon());setSizeGripEnabled(true);
    auto* layout=new QVBoxLayout(this);layout->setContentsMargins(C::Padding,C::Padding,C::Padding,C::Padding);layout->setSpacing(C::Gap);
    auto* card=new Card(themes.data(),this);auto* grid=new QGridLayout(card);
    sourceLabel_=new QLabel(card);source_=new QLineEdit(card);source_->setReadOnly(true);browseSource_=new QPushButton(card);
    destinationLabel_=new QLabel(card);targets_=new QComboBox(card);outputLabel_=new QLabel(card);output_=new QLineEdit(card);output_->setReadOnly(true);browseOutput_=new QPushButton(card);
    summary_=new QLabel(card);summary_->setWordWrap(true);
    grid->addWidget(sourceLabel_,0,0);grid->addWidget(source_,1,0);grid->addWidget(browseSource_,1,1);grid->addWidget(summary_,C::SourceSummaryRow,0,1,C::FullSpan);
    grid->addWidget(destinationLabel_,C::DestinationLabelRow,0);grid->addWidget(targets_,C::DestinationInputRow,0,1,C::FullSpan);grid->addWidget(outputLabel_,C::OutputLabelRow,0);grid->addWidget(output_,C::OutputInputRow,0);grid->addWidget(browseOutput_,C::OutputInputRow,1);grid->setColumnStretch(0,1);layout->addWidget(card);
    status_=new QLabel(this);status_->setWordWrap(true);layout->addWidget(status_);
    losses_=new QTreeWidget(this);losses_->setRootIsDecorated(false);losses_->setUniformRowHeights(false);losses_->setColumnCount(C::ColumnCount);losses_->header()->setSectionResizeMode(QHeaderView::ResizeToContents);losses_->header()->setSectionResizeMode(C::DescriptionColumn,QHeaderView::Stretch);layout->addWidget(losses_,1);
    accept_=new QCheckBox(this);layout->addWidget(accept_);
    auto* footer=new QHBoxLayout;progress_=new QProgressBar(this);progress_->setRange(0,0);execute_=new QPushButton(this);close_=new QPushButton(this);footer->addWidget(progress_,1);footer->addStretch();footer->addWidget(execute_);footer->addWidget(close_);layout->addLayout(footer);
    connect(browseSource_,&QPushButton::clicked,this,[this]{auto path=QFileDialog::getOpenFileName(this,text(C::Source),session_->sourcePath());if(!path.isEmpty())session_->open(path);});
    connect(browseOutput_,&QPushButton::clicked,this,[this]{auto path=QFileDialog::getSaveFileName(this,text(C::Output),session_->outputPath());if(!path.isEmpty())session_->setOutput(path);});
    connect(targets_,&QComboBox::currentIndexChanged,this,[this](int index){if(index>=0)session_->selectTarget(targets_->itemData(index).toString().toStdString());});
    connect(accept_,&QCheckBox::toggled,this,[this](bool value){session_->acceptLosses(value);});
    connect(close_,&QPushButton::clicked,this,[this]{reject();});
    connect(execute_,&QPushButton::clicked,this,[this]{
        if(session_->busy()){session_->cancel();return;}
        const bool exists=QFileInfo::exists(session_->outputPath());
        if(exists){QMessageBox confirm(QMessageBox::Warning,text(C::Title),text(C::Overwrite).replace(QString::fromLatin1(C::Argument),session_->outputPath()),QMessageBox::Yes|QMessageBox::No,this);confirm.setDefaultButton(QMessageBox::No);if(confirm.exec()!=QMessageBox::Yes)return;}
        session_->start(exists);
    });
    subscription_=session_->subscribe([this]{refresh();});language_=translations.subscribe([this]{refresh();});theme_=themes.subscribe([this]{appearance();});
    const auto room=parent->screen()->availableGeometry().size()-QSize(C::ScreenInset,C::ScreenInset);setMinimumSize(qMin(C::MinimumWidth,room.width()),qMin(C::MinimumHeight,room.height()));resize(qMin(C::Width,room.width()),qMin(C::Height,room.height()));
    if(!state.migrationGeometry.isEmpty())restoreGeometry(state.migrationGeometry);
    refresh();appearance();
}
MigrationDialog::~MigrationDialog()=default;
QString MigrationDialog::text(const char* key)const{return QString::fromStdString(translations_.text(key));}
void MigrationDialog::refresh(){
    if(closing_&&!session_->busy()){closing_=false;done(QDialog::Rejected);return;}
    setWindowTitle(text(C::Title));sourceLabel_->setText(text(C::Source));destinationLabel_->setText(text(C::Destination));outputLabel_->setText(text(C::Output));browseSource_->setText(text(C::Browse));browseOutput_->setText(text(C::Browse));close_->setText(text(C::Close));accept_->setText(text(C::AcceptLosses));execute_->setText(text(session_->busy()?C::Stop:C::Execute));
    source_->setText(session_->sourcePath());output_->setText(session_->outputPath());
    {const QSignalBlocker blocked(targets_);targets_->clear();for(const auto& target:MediaEngine::FileSystemMigrationTargetCatalog::all()){const auto label=std::find_if(C::TargetLabels.begin(),C::TargetLabels.end(),[&](const auto& pair){return target.formatId==pair.first;});const auto id=QString::fromUtf8(target.formatId.data(),static_cast<int>(target.formatId.size()));targets_->addItem(label==C::TargetLabels.end()?id:text(label->second),id);}targets_->setCurrentIndex(targets_->findData(QString::fromStdString(session_->target())));}
    summary_->setText(session_->source()?mediaText(translations_,{C::SourceSummary,{session_->source()->name,session_->source()->fileSystemId,std::to_string(session_->source()->entries.size())},{}}):text(C::NoSource));
    losses_->setHeaderLabels({text(C::Severity),text(C::Path),text(C::Description)});losses_->clear();bool blocking=false,metadata=false;
    if(session_->report())for(const auto& loss:session_->report()->losses){blocking|=loss.isBlocking;metadata|=!loss.isBlocking;auto* item=new QTreeWidgetItem(losses_,{text(loss.isBlocking?C::Blocking:C::MetadataLoss),QString::fromStdString(loss.path),text(C::LossLabels.at(static_cast<std::size_t>(loss.kind)))});item->setToolTip(1,QString::fromStdString(loss.path));}
    status_->setText(session_->message()?mediaText(translations_,*session_->message()):text(!session_->report()?C::Waiting:blocking?C::Incompatible:metadata?C::CompatibleWithLosses:C::Compatible));
    {const QSignalBlocker blocked(accept_);accept_->setChecked(session_->lossesAccepted());}accept_->setVisible(metadata&&!blocking);
    const bool busy=session_->busy();browseSource_->setEnabled(!busy);browseOutput_->setEnabled(!busy&&bool(session_->source()));targets_->setEnabled(!busy);accept_->setEnabled(!busy);progress_->setVisible(busy);execute_->setEnabled(!closing_&&(busy||session_->canExecute()));close_->setEnabled(!closing_);
}
void MigrationDialog::appearance(){Platform::applyWindowAppearance(static_cast<std::uintptr_t>(winId()),themes_.data()->dark,themes_.data()->color(ThemeConstants::CaptionBrush).rgba(),themes_.data()->color(ThemeConstants::TextBrush).rgba(),themes_.data()->nativeSystemColors);}
void MigrationDialog::synchronizeState(){state_.migrationGeometry=saveGeometry();}
void MigrationDialog::showEvent(QShowEvent* event){QDialog::showEvent(event);appearance();}
void MigrationDialog::done(int result){if(session_->busy()){closing_=true;session_->cancel();refresh();return;}synchronizeState();state_.migrationOpen=false;QDialog::done(result);}
}
