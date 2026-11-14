#include "KryoFluxSettingsWidget.h"
#include <QApplication>
#include "Hardware/HostToolsService.h"
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
namespace RetroMediaLab {
using namespace HardwareConstants;
KryoFluxSettingsWidget::KryoFluxSettingsWidget(Translations::TranslationService& translations,WorkspaceState& state,QWidget* parent)
    :QWidget(parent),translations_(translations),state_(state){
    auto* layout=new QVBoxLayout(this);layout->setContentsMargins(0,0,0,0);layout->setSpacing(Gap);
    description_=new QLabel(this);description_->setWordWrap(true);layout->addWidget(description_);
    title_=new QLabel(this);layout->addWidget(title_);auto* row=new QHBoxLayout;
    path_=new QLineEdit(state.hardware().dtcPath,this);path_->setObjectName(DtcPathInput);title_->setBuddy(path_);
    browse_=new QPushButton(this);browse_->setAutoDefault(false);row->addWidget(path_,1);row->addWidget(browse_);layout->addLayout(row);
    find_=new QPushButton(this);find_->setObjectName(FindDtcButton);find_->setAutoDefault(false);layout->addWidget(find_,0,Qt::AlignLeft);
    status_=new QLabel(this);status_->setWordWrap(true);status_->setTextFormat(Qt::PlainText);layout->addWidget(status_);
    connect(path_,&QLineEdit::editingFinished,this,[this]{synchronizeState();});
    connect(find_,&QPushButton::clicked,this,[this]{findTool();});
    connect(browse_,&QPushButton::clicked,this,[this]{
        if(!picker_){picker_=std::make_unique<QFileDialog>(this);picker_->setFileMode(QFileDialog::ExistingFile);picker_->setNameFilter(DtcFilter);
            connect(picker_.get(),&QFileDialog::fileSelected,this,[this](const QString& path){path_->setText(path);notFound_=false;synchronizeState();refreshTexts();});}
        picker_->setWindowTitle(QString::fromStdString(translations_.text(DtcPathKey)));picker_->open();
    });
    subscription_=translations.subscribe([this]{refreshTexts();});refreshTexts();
}
KryoFluxSettingsWidget::~KryoFluxSettingsWidget()=default;
void KryoFluxSettingsWidget::synchronizeState(){auto settings=state_.hardware();settings.dtcPath=path_->text().trimmed();state_.setHardware(std::move(settings));}
void KryoFluxSettingsWidget::findTool(){
    const auto result=HostToolsService::findDtcExecutable(path_->text(),HostToolsService::managedDirectory(state_.logsDirectory,KryoFolder),QApplication::applicationDirPath());
    notFound_=result.isEmpty();if(!notFound_){path_->setText(result);synchronizeState();}refreshTexts();
}
void KryoFluxSettingsWidget::refreshTexts(){
    const auto text=[this](const char* key){return QString::fromStdString(translations_.text(key));};
    title_->setText(text(DtcPathKey));description_->setText(text(DtcConfigurationKey));browse_->setText(text(ChooseKey));find_->setText(text(FindDtcKey));status_->setText(notFound_?text(DtcNotFoundKey):QString());
}
}
