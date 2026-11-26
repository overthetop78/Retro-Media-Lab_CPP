#include "ClassificationPanel.h"
#include "ClassificationConstants.h"
#include <QComboBox>
#include <QCheckBox>
#include <QPainter>
#include <QPixmap>
#include <QGridLayout>
#include <QLabel>
#include <QSignalBlocker>
#include <QVBoxLayout>
#include <algorithm>
namespace RetroMediaLab {
namespace C=ClassificationConstants;

QIcon recognizedFormatIcon(){
    QPixmap image(C::RecognitionIconExtent,C::RecognitionIconExtent);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor::fromRgb(C::RecognitionDotRgb));
    painter.drawEllipse(image.rect().adjusted(C::RecognitionDotInset,C::RecognitionDotInset,
        -C::RecognitionDotInset,-C::RecognitionDotInset));
    painter.end();
    return QIcon(image);
}

ClassificationPanel::ClassificationPanel(Infrastructure::ClassificationSelection& selection,Translations::TranslationService& translations,bool showAutomatic,std::function<void(bool)> changed,QWidget* parent)
 :QWidget(parent),selection_(selection),translations_(translations),changed_(std::move(changed)),showAutomatic_(showAutomatic){
    auto* layout=new QVBoxLayout(this);layout->setContentsMargins(C::Padding,C::Padding,C::Padding,C::Padding);layout->setSpacing(C::Spacing);
    automatic_=new QCheckBox(this);automatic_->setObjectName(C::AutomaticObject);layout->addWidget(automatic_);auto* grid=new QGridLayout;layout->addLayout(grid);
    machine_=new QComboBox(this);machine_->setObjectName(C::MachineObject);format_=new QComboBox(this);format_->setObjectName(C::FormatObject);protection_=new QComboBox(this);protection_->setObjectName(C::ProtectionObject);
    machine_->setMinimumWidth(0);format_->setMinimumWidth(0);protection_->setMinimumWidth(0);machine_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);format_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);protection_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    machineLabel_=new QLabel(this);formatLabel_=new QLabel(this);protectionLabel_=new QLabel(this);grid->addWidget(machineLabel_,0,0);grid->addWidget(formatLabel_,0,1);grid->addWidget(protectionLabel_,0,C::ProtectionColumn);grid->addWidget(machine_,C::SelectionRow,0);grid->addWidget(format_,C::SelectionRow,1);grid->addWidget(protection_,C::SelectionRow,C::ProtectionColumn);grid->setColumnStretch(0,1);grid->setColumnStretch(1,1);grid->setColumnStretch(C::ProtectionColumn,1);
    connect(automatic_,&QCheckBox::toggled,this,[this](bool checked){selection_.setAutomatic(checked);refresh();changed_(true);});
    connect(machine_,&QComboBox::currentIndexChanged,this,[this](int index){if(index<0)return;selection_.selectMachine(machine_->itemData(index).toString().toStdString());refresh();changed_(false);});
    connect(format_,&QComboBox::currentIndexChanged,this,[this](int index){if(index<0)return;selection_.selectFormat(format_->itemData(index).toString().toStdString());refresh();changed_(false);});
    connect(protection_,&QComboBox::currentIndexChanged,this,[this](int index){if(index<0)return;auto id=protection_->itemData(index).toString();selection_.selectProtection(id.isEmpty()?std::nullopt:std::optional<std::string>(id.toStdString()));refresh();changed_(false);});refresh();
}
void ClassificationPanel::refresh(){
    const auto text=[this](const char* key){return QString::fromStdString(translations_.text(key));};const auto resolve=[this](std::string_view key){return translations_.text(std::string(key),C::Domain);};
    const QSignalBlocker machineBlocked(machine_),formatBlocked(format_),protectionBlocked(protection_),automaticBlocked(automatic_);
    automatic_->setText(text(C::Automatic));automatic_->setVisible(showAutomatic_);automatic_->setChecked(selection_.automatic());machineLabel_->setText(text(C::Machine));formatLabel_->setText(text(C::Format));protectionLabel_->setText(text(C::Protection));
    const auto& catalog=selection_.catalog();machine_->clear();for(const auto& machine:catalog.machines()){const auto formats=catalog.formatsFor(machine);machine_->addItem(formats.empty()?QString::fromStdString(machine):QString::fromStdString(MediaEngine::imageFormatFamilyName(formats.front(),resolve)),QString::fromStdString(machine));if(std::any_of(selection_.detectedFormats().begin(),selection_.detectedFormats().end(),[&](const auto& id){const auto* detected=catalog.resolveFormat(id);return detected&&detected->family==machine;})){machine_->setItemIcon(machine_->count()-1,recognizedFormatIcon());}}
    machine_->setCurrentIndex(selection_.machine()?machine_->findData(QString::fromStdString(*selection_.machine())):-1);
    format_->clear();for(const auto& format:catalog.formatsFor(selection_.machine())){format_->addItem(QString::fromStdString(MediaEngine::imageFormatName(format,resolve)),QString::fromStdString(format.id));if(std::find(selection_.detectedFormats().begin(),selection_.detectedFormats().end(),format.id)!=selection_.detectedFormats().end()){format_->setItemIcon(format_->count()-1,recognizedFormatIcon());}}
    format_->setCurrentIndex(selection_.format()?format_->findData(QString::fromStdString(*selection_.format())):-1);
    protection_->clear();const auto protections=catalog.protectionsFor(selection_.machine(),selection_.format());protection_->addItem(QString::fromUtf8(C::NoProtection),QString{});for(const auto& protection:protections)protection_->addItem(QString::fromStdString(protection.displayName),QString::fromStdString(protection.id));protection_->setCurrentIndex(selection_.protection()?protection_->findData(QString::fromStdString(*selection_.protection())):0);protection_->setVisible(!protections.empty());protectionLabel_->setVisible(!protections.empty());
}
}
