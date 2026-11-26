#include "GreaseweazleToolPage.h"
#include "../ThemeConstants.h"
#include "Maintenance/GwToolsConstants.h"
#include "Shared/Controls/FlowLayout.h"
#include "Shared/Controls/ContentScrollArea.h"
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QAbstractButton>
#include <QFontDatabase>
#include <QSignalBlocker>
#include <QTextCursor>
#include <algorithm>
#include <stdexcept>
namespace RetroMediaLab {
using namespace GwToolsConstants;
namespace View = ThemeConstants::GreaseweazleDialog;
GreaseweazleToolPage::GreaseweazleToolPage(const GwToolDefinition& definition, Translations::TranslationService& translations, WorkspaceState& state, QWidget* parent)
    : QWidget(parent), verb_(definition.id), translations_(translations), state_(state) {
    setObjectName(verb_);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(ThemeConstants::PreferencesLayout::CardPadding, ThemeConstants::PreferencesLayout::CardPadding, ThemeConstants::PreferencesLayout::CardPadding, ThemeConstants::PreferencesLayout::CardPadding);
    if (verb_ == Erase || verb_ == Clean) {
        warning_ = new QLabel(this);
        warning_->setTextFormat(Qt::PlainText);
        warning_->setWordWrap(true);
        layout->addWidget(warning_);
    }
    if (!definition.parameters.isEmpty()) {
        auto* scroll = new ContentScrollArea(View::ParametersMaximumHeight, this);
        auto* host = new QWidget(scroll); auto* flow = new FlowLayout(host); flow->setContentsMargins(0,0,0,0); flow->setSpacing(View::ParameterGap);
        for (const auto& parameter : definition.parameters) {
            auto* container = new QWidget(host); auto* row = new QVBoxLayout(container); row->setContentsMargins(0,0,0,0);
            Field field{QString::fromLatin1(parameter.id), parameter.label, parameter.kind, nullptr, nullptr, nullptr};
            if (parameter.kind == GwParameterKind::Required) { field.label = new QLabel(container); row->addWidget(field.label); }
            else { field.check = new QCheckBox(container); field.check->setObjectName(QString::fromLatin1(View::CheckPrefix) + verb_ + Dash + field.id); row->addWidget(field.check); }
            if (parameter.kind != GwParameterKind::Flag) {
                field.edit = new QLineEdit(container); field.edit->setFixedWidth(View::ParameterInputWidth); field.edit->setObjectName(QString::fromLatin1(View::FieldPrefix) + verb_ + Dash + field.id); row->addWidget(field.edit);
                if (field.label) field.label->setBuddy(field.edit);
                connect(field.edit, &QLineEdit::textEdited, this, [this, id = field.id](const QString& value) { state_.greaseweazleTools->setValue(verb_, id, value); });
            }
            if (field.check) connect(field.check, &QCheckBox::toggled, this, [this, id = field.id](bool enabled) { state_.greaseweazleTools->setEnabled(verb_, id, enabled); });
            fields_.push_back(field); flow->addWidget(container);
        }
        scroll->setContent(host); layout->addWidget(scroll);
    }
    summary_ = new QLabel(this); summary_->setWordWrap(true); summary_->setTextFormat(Qt::PlainText); summary_->setObjectName(QString::fromLatin1(View::SummaryPrefix) + verb_); layout->addWidget(summary_);
    outputTitle_ = new QLabel(this); layout->addWidget(outputTitle_);
    output_ = new QPlainTextEdit(this); output_->setReadOnly(true); output_->setObjectName(QString::fromLatin1(View::OutputPrefix) + verb_); output_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont)); layout->addWidget(output_, 1);
    commandTitle_ = new QLabel(this); layout->addWidget(commandTitle_);
    command_ = new QLineEdit(this); command_->setReadOnly(true); command_->setObjectName(QString::fromLatin1(View::CommandPrefix) + verb_); command_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont)); commandTitle_->setBuddy(command_); layout->addWidget(command_);
    execute_ = new QPushButton(this); execute_->setAutoDefault(false); execute_->setObjectName(QString::fromLatin1(View::ExecutePrefix) + verb_); layout->addWidget(execute_, 0, Qt::AlignRight); connect(execute_, &QPushButton::clicked, this, [this] { execute(); });
    toolsSubscription_ = state.greaseweazleTools->subscribe([this] { refresh(); });
    languageSubscription_ = translations.subscribe([this] { refresh(); }); refresh();
}
GreaseweazleToolPage::~GreaseweazleToolPage() = default;
QString GreaseweazleToolPage::text(const QString& key) const { return QString::fromStdString(translations_.text(key.toStdString())); }
void GreaseweazleToolPage::refresh() {
    if (warning_) warning_->setText(text(verb_ == Erase ? EraseWarning : CleanWarning));
    const auto& service = *state_.greaseweazleTools; const auto& draft = service.draft(verb_); const auto active = service.activeOperation(); const bool running = active == verb_;
    for (const auto& field : fields_) {
        if (field.label) field.label->setText(text(field.labelKey));
        if (field.check) { const QSignalBlocker blocked(field.check); field.check->setText(text(field.labelKey)); field.check->setChecked(draft.enabled.contains(field.id)); field.check->setEnabled(!running); }
        if (field.edit) { const QSignalBlocker blocked(field.edit); if (field.edit->text() != draft.values.value(field.id)) field.edit->setText(draft.values.value(field.id)); field.edit->setAccessibleName(text(field.labelKey)); field.edit->setEnabled(!running && (field.kind == GwParameterKind::Required || draft.enabled.contains(field.id))); }
    }
    QString validation;
    try { command_->setText(service.command(verb_, state_.hardware(), state_.greaseweazleDeviceId).displayText()); }
    catch (const std::exception& error) { validation = QString::fromUtf8(error.what()); command_->setText(text(validation)); }
    if (draft.summaryKey != ReadyKey && !draft.lastCommand.isEmpty()) command_->setText(draft.lastCommand);
    QString summary = text(draft.summaryKey);
    for (int i = 0; i < draft.summaryArguments.size(); ++i) summary.replace(QString(Argument).replace(QString::number(0), QString::number(i)), draft.summaryArguments[i]);
    if (draft.information) summary = (draft.model.isEmpty() ? text(ControllerFallbackKey) : draft.model) + View::InfoSeparator + (draft.firmware.isEmpty() ? text(FirmwareUnknownKey) : draft.firmware) + View::InfoSeparator + (draft.port.isEmpty() ? text(PortUnknownKey) : draft.port);
    if (draft.networkWarning) summary += QString(NewLine) + text(NetworkWarningKey);
    if (!validation.isEmpty() && draft.summaryKey == ReadyKey) summary = text(validation);
    if (draft.logFailed) summary += QString(NewLine) + text(LogFailedKey);
    summary_->setText(summary);
    if (output_->toPlainText() != draft.output) { output_->setPlainText(draft.output); output_->moveCursor(QTextCursor::End); }
    outputTitle_->setText(text(OutputKey)); output_->setAccessibleName(text(OutputKey)); commandTitle_->setText(text(CommandKey)); command_->setAccessibleName(text(CommandKey));
    execute_->setText(text(running ? StoppedKey : ExecuteKey)); execute_->setEnabled(running || (active.isEmpty() && validation.isEmpty()));
}
void GreaseweazleToolPage::confirm(const char* title, const char* message, std::function<void()> accepted,const QString& details) {
    if (confirmation_ && confirmation_->isVisible()) return;
    confirmation_ = std::make_unique<QMessageBox>(QMessageBox::Warning, text(title), text(message), QMessageBox::Yes | QMessageBox::No, this);
    confirmation_->setTextFormat(Qt::PlainText);
    if (!details.isEmpty()) confirmation_->setInformativeText(details);
    confirmation_->setDefaultButton(QMessageBox::No); confirmation_->button(QMessageBox::Yes)->setText(text(View::Yes)); confirmation_->button(QMessageBox::No)->setText(text(View::No));
    connect(confirmation_.get(), &QMessageBox::finished, this, [accepted = std::move(accepted)](int result) { if (result == QMessageBox::Yes) accepted(); }); confirmation_->open();
}
void GreaseweazleToolPage::execute() {
    if (state_.greaseweazleTools->activeOperation() == verb_) { confirm(StopTitleKey, StopConfirmKey, [this] { state_.greaseweazleTools->stop(); }); return; }
    if (verb_ == Erase) {
        try {
            const auto hardware = state_.hardware();
            const auto reader = state_.greaseweazleDeviceId;
            const auto approved = state_.greaseweazleTools->command(verb_,hardware,reader);
            const auto found = std::find_if(hardware.devices.begin(),hardware.devices.end(),[&reader](const auto& device) { return device.id == reader; });
            const auto details = QString::fromUtf8(View::DeviceFormat).arg(found->name,found->address,found->selection);
            confirm(EraseTitle, EraseWarning, [this,reader,approved] {
                try {
                    const auto current = state_.greaseweazleTools->command(verb_, state_.hardware(),state_.greaseweazleDeviceId);
                    if (state_.greaseweazleDeviceId != reader || current.executable != approved.executable || current.arguments != approved.arguments)
                        throw std::invalid_argument(InvalidKey);
                    state_.greaseweazleTools->executeCommand(verb_,approved);
                } catch (const std::exception& error) {
                    summary_->setText(text(QString::fromUtf8(error.what())));
                }
            }, details);
        } catch (const std::exception& error) {
            summary_->setText(text(QString::fromUtf8(error.what())));
        }
        return;
    }
    if (verb_ == Update && state_.greaseweazleTools->draft(verb_).enabled.contains(BootloaderParameter)) { confirm(BootTitleKey, BootWarningKey, [this] { start(); }); return; }
    start();
}
void GreaseweazleToolPage::start() {
    try { state_.greaseweazleTools->execute(verb_, state_.hardware(), state_.greaseweazleDeviceId); }
    catch (const std::exception& error) { summary_->setText(text(QString::fromUtf8(error.what()))); }
}
}
