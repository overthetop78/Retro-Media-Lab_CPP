#include "EngineSettingsPage.h"
#include "../ThemeConstants.h"
#include "Shared/Controls/Card.h"
#include <QTabWidget>
#include <QLabel>
#include <QComboBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QSignalBlocker>
namespace RetroMediaLab {
EngineSettingsPage::EngineSettingsPage(ThemeService& themes,Translations::TranslationService& translations,WorkspaceState& state,QWidget* parent):QWidget(parent),translations_(translations),state_(state){
    using namespace ThemeConstants::EnginesPage;
    setObjectName(Page);auto* outer=new QVBoxLayout(this);outer->setContentsMargins(Padding,Padding,Padding,Padding);
    backends_=new QTabWidget(this);backends_->setObjectName(Tabs);outer->addWidget(backends_);
    auto* greaseweazle=new QWidget(backends_);greaseweazle->setObjectName(GreaseweazlePage);
    auto* layout=new QVBoxLayout(greaseweazle);layout->setContentsMargins(Padding,Padding,Padding,Padding);layout->setSpacing(Gap);
    description_=new QLabel(greaseweazle);description_->setWordWrap(true);layout->addWidget(description_);
    auto* card=new Card(themes.data(),greaseweazle);auto* grid=new QGridLayout(card);grid->setContentsMargins(CardPadding,CardPadding,CardPadding,CardPadding);grid->setVerticalSpacing(Gap);grid->setHorizontalSpacing(Gap);grid->setColumnStretch(0,1);grid->setColumnStretch(1,1);
    int index=0;for(const auto& operation:engineOperations()){
        auto* label=new QLabel(card);auto* combo=new QComboBox(card);combo->setObjectName(QString::fromLatin1(ChoicePrefix)+QString::fromLatin1(operation.id));combo->setMinimumWidth(ComboMinimumWidth);label->setBuddy(combo);grid->addWidget(label,index,0);grid->addWidget(combo,index,1);++index;
        rows_.push_back({QString::fromLatin1(operation.id),operation.label,label,combo});
        connect(combo,&QComboBox::currentIndexChanged,this,[this,combo,id=QString::fromLatin1(operation.id)](int item){
            if(item<0)return;auto settings=state_.engines();settings.select(EngineConstants::Greaseweazle,id,combo->itemData(item).toString());state_.setEngines(std::move(settings));
        });
    }
    layout->addWidget(card);strict_=new QLabel(greaseweazle);strict_->setWordWrap(true);layout->addWidget(strict_);layout->addStretch();
    backends_->addTab(greaseweazle,QString());
    auto* kryoflux=new QWidget(backends_);kryoflux->setObjectName(KryoFluxPage);backends_->addTab(kryoflux,QString());
    backends_->setCurrentIndex(state.engineCategory==EngineConstants::KryoFlux?1:0);
    connect(backends_,&QTabWidget::currentChanged,this,[this](int index){state_.engineCategory=index==0?EngineConstants::Greaseweazle:EngineConstants::KryoFlux;});
    languageSubscription_=translations.subscribe([this]{refreshTexts();});settingsSubscription_=state.subscribeToEngines([this]{refreshTexts();});refreshTexts();
}
void EngineSettingsPage::refreshTexts(){
    using namespace EngineConstants;
    const auto text=[this](const char* key){return QString::fromStdString(translations_.text(key));};
    backends_->setTabText(0,text(GreaseweazleKey));backends_->setTabText(1,text(KryoFluxKey));description_->setText(text(DescriptionKey));strict_->setText(text(StrictChoiceKey));
    for(const auto& row:rows_){
        row.label->setText(text(row.labelKey));const QSignalBlocker blocked(row.choice);row.choice->clear();row.choice->addItem(text(InternalKey),Internal);row.choice->addItem(text(HostToolsKey),HostTools);
        const auto selected=state_.engines().engineFor(Greaseweazle,row.operation);int index=row.choice->findData(selected);
        if(index<0){row.choice->addItem(text(MissingProviderKey).replace(Argument,selected),selected);index=row.choice->count()-1;row.choice->setItemData(index,text(MissingProviderKey).replace(Argument,selected),Qt::ToolTipRole);}
        row.choice->setCurrentIndex(index);row.choice->setAccessibleName(row.label->text());
    }
}
}
