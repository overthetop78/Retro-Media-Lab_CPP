#include "LogSettingsPage.h"
#include "../ThemeConstants.h"
#include "Shared/Controls/Card.h"
#include "Logging/DiagnosticLog.h"
#include <QCheckBox>
#include <QDesktopServices>
#include <QDir>
#include "Logging/LogPaths.h"
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpressionValidator>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QUrl>
#include <QVBoxLayout>
namespace RetroMediaLab {
LogSettingsPage::LogSettingsPage(ThemeService& themes,Translations::TranslationService& translations,WorkspaceState& state,QWidget* parent)
    :QWidget(parent),translations_(translations),state_(state) {
    setObjectName(ThemeConstants::LogPage::Page);
    auto* outer=new QVBoxLayout(this);
    const auto padding=ThemeConstants::PreferencesLayout::CardPadding;
    outer->setContentsMargins(padding,padding,padding,padding);
    outer->setSpacing(ThemeConstants::LogPage::DescriptionGap);
    description_=new QLabel(this);description_->setWordWrap(true);outer->addWidget(description_);
    auto* scroll=new QScrollArea(this);scroll->setObjectName(ThemeConstants::LogPage::Scroll);
    scroll->setWidgetResizable(true);scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto* content=new QWidget(scroll);auto* list=new QVBoxLayout(content);
    list->setContentsMargins(0,0,0,0);list->setSpacing(ThemeConstants::LogPage::RowGap);
    for(const auto& definition:logActions()) {
        auto* card=new Card(themes.data(),content);auto* layout=new QHBoxLayout(card);
        layout->setContentsMargins(ThemeConstants::PreferencesLayout::CardSpacing,ThemeConstants::LogPage::Gap,ThemeConstants::PreferencesLayout::CardSpacing,ThemeConstants::LogPage::Gap);
        layout->setSpacing(0);
        Row row;row.action=definition.id;row.labelKey=definition.label;
        row.label=new QLabel(card);auto font=row.label->font();font.setWeight(QFont::DemiBold);row.label->setFont(font);
        row.label->setFixedWidth(ThemeConstants::LogPage::ActionWidth);
        row.enabled=new QCheckBox(card);row.enabled->setFixedWidth(ThemeConstants::LogPage::ToggleWidth);
        row.enabled->setObjectName(QString(ThemeConstants::LogPage::EnabledPrefix)+row.action);
        auto* size=new QWidget(card);size->setFixedWidth(ThemeConstants::LogPage::SizeWidth);
        auto* sizeLayout=new QHBoxLayout(size);sizeLayout->setContentsMargins(0,0,ThemeConstants::LogPage::UnitGap,0);
        sizeLayout->setSpacing(ThemeConstants::LogPage::Gap);
        row.maximumLabel=new QLabel(size);row.maximum=new QLineEdit(size);
        row.maximum->setObjectName(QString(ThemeConstants::LogPage::MaximumPrefix)+row.action);
        row.maximum->setFixedWidth(ThemeConstants::LogPage::InputWidth);
        row.maximum->setValidator(new QRegularExpressionValidator(QRegularExpression(ThemeConstants::LogPage::Digits),row.maximum));
        row.maximum->installEventFilter(this);row.maximumLabel->setBuddy(row.maximum);
        row.unit=new QLabel(size);sizeLayout->addWidget(row.maximumLabel);sizeLayout->addWidget(row.maximum);sizeLayout->addWidget(row.unit);sizeLayout->addStretch();
        row.archives=new QCheckBox(card);row.archives->setObjectName(QString(ThemeConstants::LogPage::ArchivesPrefix)+row.action);
        layout->addWidget(row.label);layout->addWidget(row.enabled);layout->addWidget(size);layout->addWidget(row.archives,1);
        list->addWidget(card);const auto index=rows_.size();rows_.push_back(row);
        const auto changed=[this,index] {
            auto settings=state_.logging();auto value=settings.forAction(rows_[index].action);
            value.enabled=rows_[index].enabled->isChecked();value.keepArchives=rows_[index].archives->isChecked();
            settings.actions.insert(rows_[index].action,value);state_.setLogging(std::move(settings));
        };
        connect(row.enabled,&QCheckBox::clicked,this,changed);connect(row.archives,&QCheckBox::clicked,this,changed);
        connect(row.maximum,&QLineEdit::editingFinished,this,[this,index]{commitMaximum(rows_[index]);});
    }
    list->addStretch();scroll->setWidget(content);outer->addWidget(scroll,1);
    auto* footer=new QHBoxLayout;footer->setSpacing(ThemeConstants::LogPage::DescriptionGap);
    hint_=new QLabel(this);directory_=new QLabel(QDir::toNativeSeparators(state.logsDirectory),this);
    directory_->setObjectName(ThemeConstants::LogPage::Directory);directory_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    directory_->setTextFormat(Qt::PlainText);
    open_=new QPushButton(this);open_->setObjectName(ThemeConstants::LogPage::OpenFolder);
    footer->addWidget(hint_);footer->addStretch();footer->addWidget(directory_);footer->addWidget(open_);outer->addLayout(footer);
    connect(open_,&QPushButton::clicked,this,[this]{openFolder();});
    languageSubscription_=translations.subscribe([this]{refreshTexts();});
    settingsSubscription_=state.subscribeToLogging([this]{refreshSettings();});
    themeSubscription_=themes.subscribe([this]{update();});
    refreshTexts();refreshSettings();
}
LogSettingsPage::~LogSettingsPage()=default;
void LogSettingsPage::refreshTexts() {
    const auto text=[this](const char* key){return QString::fromStdString(translations_.text(key));};
    description_->setText(text(ThemeConstants::LogPage::Description));hint_->setText(text(ThemeConstants::LogPage::UnlimitedHint));open_->setText(text(ThemeConstants::LogPage::OpenFolderLabel));
    for(auto& row:rows_){row.label->setText(text(row.labelKey));row.enabled->setText(text(ThemeConstants::LogPage::Enable));row.maximumLabel->setText(text(ThemeConstants::LogPage::Maximum));row.unit->setText(text(ThemeConstants::LogPage::KiB));row.archives->setText(text(ThemeConstants::LogPage::Archives));row.maximum->setAccessibleName(row.label->text()+LogConstants::Space+row.maximumLabel->text());}
}
void LogSettingsPage::refreshSettings() {
    const auto settings=state_.logging();
    for(auto& row:rows_){const auto value=settings.forAction(row.action);const QSignalBlocker enabled(row.enabled),archives(row.archives);
        row.enabled->setChecked(value.enabled);row.archives->setChecked(value.keepArchives);
        if(!row.maximum->hasFocus())row.maximum->setText(QString::number(value.maximumKilobytes));}
}
void LogSettingsPage::commitMaximum(Row& row) {
    auto settings=state_.logging();auto value=settings.forAction(row.action);bool valid=false;
    const auto size=row.maximum->text().toInt(&valid);
    if(valid&&size>=LogConstants::Unlimited&&row.maximum->hasAcceptableInput()&&size!=value.maximumKilobytes) {
        value.maximumKilobytes=size;settings.actions.insert(row.action,value);state_.setLogging(std::move(settings));
    }
    row.maximum->setText(QString::number(state_.logging().forAction(row.action).maximumKilobytes));
}
void LogSettingsPage::commitEdits(){for(auto& row:rows_)commitMaximum(row);}
bool LogSettingsPage::eventFilter(QObject* object,QEvent* event) {
    if(event->type()==QEvent::FocusOut)for(auto& row:rows_)if(row.maximum==object){commitMaximum(row);break;}
    return false;
}
void LogSettingsPage::openFolder() {
    if(ensureLogDirectory(state_.logsDirectory)&&QDesktopServices::openUrl(QUrl::fromLocalFile(state_.logsDirectory)))return;
    const auto message=QString::fromStdString(translations_.text(ThemeConstants::LogPage::OpenFolderError)).replace(ThemeConstants::LogPage::Detail,state_.logsDirectory);
    writeDiagnostic(state_.logsDirectory,DiagnosticSeverity::Error,message,LogConstants::OpenFolderContext,QString::fromStdString(translations_.language()),QString());
    if(!error_)error_=std::make_unique<QMessageBox>(QMessageBox::Warning,QString(),QString(),QMessageBox::Ok,this);
    error_->setTextFormat(Qt::PlainText);
    error_->setWindowTitle(QString::fromStdString(translations_.text(ThemeConstants::LogPage::ErrorTitle)));error_->setText(message);error_->open();
}
}
