#include "HostToolsSettingsWidget.h"
#include "Hardware/HardwareConstants.h"
#include "Logging/DiagnosticLog.h"
#include <QApplication>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QAbstractButton>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
namespace RetroMediaLab {
using namespace HardwareConstants;
HostToolsSettingsWidget::HostToolsSettingsWidget(Translations::TranslationService& translations,WorkspaceState& state,QWidget* parent)
    :QWidget(parent),translations_(translations),state_(state),service_(std::make_unique<HostToolsService>()){
    auto* layout=new QVBoxLayout(this);layout->setContentsMargins(0,0,0,0);layout->setSpacing(Gap);
    title_=new QLabel(this);layout->addWidget(title_);auto* row=new QHBoxLayout;
    path_=new QLineEdit(state.hardware().gwPath,this);path_->setObjectName(GwPathInput);
    browse_=new QPushButton(this);browse_->setObjectName(ToolBrowse);browse_->setAutoDefault(false);
    row->addWidget(path_,1);row->addWidget(browse_);layout->addLayout(row);auto* actions=new QHBoxLayout;
    const auto button=[&](const char* name){auto* result=new QPushButton(this);result->setObjectName(name);result->setAutoDefault(false);actions->addWidget(result);return result;};
    detect_=button(ToolFind);check_=button(ToolCheck);download_=button(ToolDownload);rollback_=button(ToolRollback);
    actions->addStretch();layout->addLayout(actions);
    progress_=new QProgressBar(this);progress_->setObjectName(ToolProgress);progress_->setRange(0,ProgressMaximum);progress_->hide();layout->addWidget(progress_);
    status_=new QLabel(this);status_->setObjectName(ToolStatus);status_->setWordWrap(true);status_->setTextFormat(Qt::PlainText);layout->addWidget(status_);
    connect(path_,&QLineEdit::editingFinished,this,[this]{synchronizeState();});
    connect(browse_,&QPushButton::clicked,this,[this]{
        if(!picker_){picker_=std::make_unique<QFileDialog>(this);picker_->setFileMode(QFileDialog::ExistingFile);picker_->setNameFilter(FileFilter);
            connect(picker_.get(),&QFileDialog::fileSelected,this,[this](const QString& value){selectPath(value);});}
        picker_->setWindowTitle(QString::fromStdString(translations_.text(GwPathLabelKey)));picker_->open();
    });
    connect(detect_,&QPushButton::clicked,this,[this]{
        const auto found=HostToolsService::findExecutable(path_->text(),managedRoot(),QApplication::applicationDirPath());
        if(found.isEmpty()){setStatus(HostNoneKey);return;}selectPath(found);
    });
    connect(rollback_,&QPushButton::clicked,this,[this]{
        const auto previous=state_.hardware().previousGwPath;
        if(!HostToolsService::validExecutable(previous)){failure(HostNoPreviousKey);return;}selectPath(previous);
    });
    connect(check_,&QPushButton::clicked,this,[this]{check(false);});connect(download_,&QPushButton::clicked,this,[this]{check(true);});
    subscription_=translations.subscribe([this]{refreshTexts();});refreshTexts();
}
HostToolsSettingsWidget::~HostToolsSettingsWidget()=default;
QString HostToolsSettingsWidget::managedRoot()const{return HostToolsService::managedDirectory(state_.logsDirectory,ToolFolder);}
void HostToolsSettingsWidget::synchronizeState(){auto settings=state_.hardware();settings.gwPath=path_->text().trimmed();state_.setHardware(std::move(settings));}
void HostToolsSettingsWidget::selectPath(const QString& path){
    auto settings=state_.hardware();
    settings.selectGwPath(path);state_.setHardware(std::move(settings));path_->setText(path);
    setStatus(HostDetectedKey,path);
}
void HostToolsSettingsWidget::setStatus(const char* key,const QString& argument){statusKey_=key;statusArgument_=argument;status_->setText(QString::fromStdString(translations_.text(key)).replace(Argument,argument));}
void HostToolsSettingsWidget::busy(bool value){for(auto* button:{detect_,check_,download_,rollback_,browse_})button->setEnabled(!value);path_->setEnabled(!value);if(!value)progress_->hide();}
void HostToolsSettingsWidget::failure(const char* key){
    busy(false);const auto message=QString::fromStdString(translations_.text(key));setStatus(key);
    writeDiagnostic(state_.logsDirectory,DiagnosticSeverity::Error,message,DiagnosticContext,QString::fromStdString(translations_.language()),DiagnosticProcess);
}
void HostToolsSettingsWidget::check(bool install){
#ifndef Q_OS_WIN
    failure(ToolHostErrorKey);return;
#endif
    synchronizeState();busy(true);
    service_->checkLatest([this,install](HostToolsRelease release){
        if(!install){setStatus(HostLatestKey,release.version);busy(false);return;}
        confirmation_=std::make_unique<QMessageBox>(QMessageBox::Question,QString::fromStdString(translations_.text(HostTitleKey)),
            QString::fromStdString(translations_.text(HostConfirmKey)).replace(Argument,release.version),QMessageBox::Yes|QMessageBox::No,this);
        confirmation_->setDefaultButton(QMessageBox::No);confirmation_->button(QMessageBox::Yes)->setText(QString::fromStdString(translations_.text(YesKey)));confirmation_->button(QMessageBox::No)->setText(QString::fromStdString(translations_.text(NoKey)));confirmation_->setTextFormat(Qt::PlainText);
        connect(confirmation_.get(),&QMessageBox::finished,this,[this,release](int answer){
            if(answer!=QMessageBox::Yes){busy(false);return;}
            progress_->setValue(0);progress_->show();
            service_->download(release,managedRoot(),[this,version=release.version](QString path){selectPath(path);
                setStatus(HostInstalledKey,version);busy(false);},
                [this](int value){progress_->setValue(value);},[this](const char* key){failure(key);});
        });confirmation_->open();
    },[this](const char* key){failure(key);});
}
void HostToolsSettingsWidget::refreshTexts(){
    const auto text=[this](const char* key){return QString::fromStdString(translations_.text(key));};
    title_->setText(text(GwPathLabelKey));title_->setBuddy(path_);browse_->setText(text(ChooseKey));
    detect_->setText(text(HostDetectKey));check_->setText(text(HostCheckKey));download_->setText(text(HostDownloadKey));rollback_->setText(text(HostRollbackKey));
    if(status_->text().isEmpty()&&HostToolsService::validExecutable(path_->text())){statusKey_=HostDetectedKey;statusArgument_=path_->text();}
    setStatus(statusKey_,statusArgument_);
}
}
