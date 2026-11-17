#include "ConversionDialogs.h"
#include "ConversionDialogConstants.h"
#include "Shared/Localization/MediaText.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QFormLayout>
#include <QHeaderView>
#include <QIntValidator>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
namespace RetroMediaLab {
namespace C=ConversionDialogConstants;
std::optional<MediaEngine::TapePcmProfile> requestPcmProfile(Translations::TranslationService& translations,QWidget* parent,bool input,std::optional<MediaEngine::TapePcmProfile> initial){
    const auto text=[&](const char* key){return QString::fromStdString(translations.text(key));};
    const auto value=initial.value_or(MediaEngine::readTapePcmProfile());std::optional<MediaEngine::TapePcmProfile> result;
    QDialog dialog(parent);dialog.setWindowTitle(text(input?C::Input:C::Output));auto* layout=new QVBoxLayout(&dialog);auto* form=new QFormLayout;
    auto* rate=new QComboBox(&dialog);rate->setEditable(true);rate->setValidator(new QIntValidator(C::MinimumRate,C::MaximumRate,rate));for(int item:C::SampleRates)rate->addItem(QString::number(item));rate->setCurrentText(QString::number(value.sampleRate));
    auto* channels=new QComboBox(&dialog);for(int item:C::Channels)channels->addItem(QString::number(item),item);channels->setCurrentIndex(channels->findData(value.channels));
    auto* bits=new QComboBox(&dialog);for(int item:C::Bits)bits->addItem(QString::number(item),item);bits->setCurrentIndex(bits->findData(value.bitsPerSample));
    auto* signedSamples=new QCheckBox(text(C::Signed),&dialog);signedSamples->setChecked(value.signedSamples);auto* endian=new QCheckBox(text(C::Endian),&dialog);endian->setChecked(value.littleEndian);
    form->addRow(text(C::Rate),rate);form->addRow(text(C::ChannelLabel),channels);form->addRow(text(C::BitLabel),bits);form->addRow(signedSamples);form->addRow(endian);layout->addLayout(form);
    auto* error=new QLabel(&dialog);error->setWordWrap(true);layout->addWidget(error);auto* buttons=new QDialogButtonBox(&dialog);buttons->addButton(text(C::Cancel),QDialogButtonBox::RejectRole);buttons->addButton(text(C::Continue),QDialogButtonBox::AcceptRole);layout->addWidget(buttons);
    QObject::connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    QObject::connect(buttons,&QDialogButtonBox::accepted,&dialog,[&]{try{MediaEngine::TapePcmProfile profile{rate->currentText().toInt(),channels->currentData().toInt(),bits->currentData().toInt(),signedSamples->isChecked(),endian->isChecked()};MediaEngine::tapePcmParameters(profile);result=profile;dialog.accept();}catch(const std::exception& failure){error->setText(mediaErrorText(translations,failure));}});
    if(dialog.exec()!=QDialog::Accepted)return {};return result;
}
std::optional<Infrastructure::ConversionConflictDecisions> requestConversionConflicts(Translations::TranslationService& translations,QWidget* parent,std::span<const MediaEngine::ConversionOutput> outputs){
    using Infrastructure::ConversionConflict;
    const auto text=[&](const char* key){return QString::fromStdString(translations.text(key));};
    QStringList paths;for(const auto& output:outputs){const auto path=QString::fromStdU16String(output.outputPath.u16string());if(QFileInfo::exists(path))paths.push_back(path);}
    if(paths.isEmpty())return Infrastructure::ConversionConflictDecisions{};
    QDialog dialog(parent);dialog.setWindowTitle(text(C::Conflict));dialog.resize(C::Width,C::Height);auto* layout=new QVBoxLayout(&dialog);layout->setContentsMargins(C::Padding,C::Padding,C::Padding,C::Padding);layout->setSpacing(C::Gap);auto* heading=new QLabel(text(C::Help),&dialog);heading->setWordWrap(true);layout->addWidget(heading);
    auto* table=new QTableWidget(static_cast<int>(paths.size()),C::ColumnCount,&dialog);table->setHorizontalHeaderLabels({text(C::File),text(C::Action)});table->horizontalHeader()->setSectionResizeMode(C::FileColumn,QHeaderView::Stretch);table->horizontalHeader()->setSectionResizeMode(C::ActionColumn,QHeaderView::ResizeToContents);table->setEditTriggers(QAbstractItemView::NoEditTriggers);table->setSelectionBehavior(QAbstractItemView::SelectRows);table->verticalHeader()->hide();
    std::vector<QComboBox*> choices;for(int row=0;row<paths.size();++row){auto* file=new QTableWidgetItem(paths[row]);file->setToolTip(paths[row]);table->setItem(row,C::FileColumn,file);auto* choice=new QComboBox(table);choice->addItem(text(C::Overwrite),static_cast<int>(ConversionConflict::Overwrite));choice->addItem(text(C::Skip),static_cast<int>(ConversionConflict::Skip));choice->addItem(text(C::Number),static_cast<int>(ConversionConflict::Number));table->setCellWidget(row,C::ActionColumn,choice);choices.push_back(choice);}layout->addWidget(table,1);
    auto* actions=new QHBoxLayout;const auto action=[&](const char* key,ConversionConflict value){auto* button=new QPushButton(text(key),&dialog);actions->addWidget(button);QObject::connect(button,&QPushButton::clicked,&dialog,[&,value]{for(auto* choice:choices)choice->setCurrentIndex(choice->findData(static_cast<int>(value)));});};
    action(C::AllOverwrite,ConversionConflict::Overwrite);action(C::AllSkip,ConversionConflict::Skip);action(C::AllNumber,ConversionConflict::Number);actions->addStretch();auto* buttons=new QDialogButtonBox(&dialog);buttons->addButton(text(C::Cancel),QDialogButtonBox::RejectRole);buttons->addButton(text(C::Continue),QDialogButtonBox::AcceptRole);actions->addWidget(buttons);layout->addLayout(actions);QObject::connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);QObject::connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);
    if(dialog.exec()!=QDialog::Accepted)return {};Infrastructure::ConversionConflictDecisions result;for(std::size_t row=0;row<choices.size();++row)result.insert(paths[static_cast<int>(row)],static_cast<ConversionConflict>(choices[row]->currentData().toInt()));return result;
}
}
