#include "HardwareSettingsPage.h"
#include "FloppySettingsPage.h"
#include "MediaDeviceSettingsPage.h"
#include <QTabBar>
#include <QTabWidget>
#include <QVBoxLayout>
namespace RetroMediaLab {
using namespace HardwareConstants;
HardwareSettingsPage::HardwareSettingsPage(ThemeService& themes,Translations::TranslationService& translations,WorkspaceState& state,QWidget* parent):QWidget(parent){
    setObjectName(Page);auto* layout=new QVBoxLayout(this);layout->setContentsMargins(Padding,Padding,Padding,Padding);
    categories_=new QTabWidget(this);categories_->setObjectName(Tab);categories_->tabBar()->setObjectName(CategoryTabBar);categories_->setUsesScrollButtons(true);
    floppy_=new FloppySettingsPage(themes,translations,state,categories_);
    for(const auto& category:hardwareCategories()){
        QWidget* page=category.id==QString(Floppy)?static_cast<QWidget*>(floppy_):new MediaDeviceSettingsPage(category,themes,translations,state,categories_);
        const auto index=categories_->addTab(page,QString::fromStdString(translations.text(category.key)));
        if(state.hardwareCategory==category.id)categories_->setCurrentIndex(index);
    }
    layout->addWidget(categories_);
    connect(categories_,&QTabWidget::currentChanged,this,[&state](int index){if(index>=0)state.hardwareCategory=hardwareCategories().at(index).id;});
    subscription_=translations.subscribe([this,&translations]{for(int index=0;index<categories_->count();++index)categories_->setTabText(index,QString::fromStdString(translations.text(hardwareCategories().at(index).key)));});
}
void HardwareSettingsPage::synchronizeState(){floppy_->synchronizeState();}
}
