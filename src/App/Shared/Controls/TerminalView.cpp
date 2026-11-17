#include <Constants/AppConstants.h>
#include "TerminalView.h"
#include <QApplication>
#include <QClipboard>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QTextCursor>
#include <QVBoxLayout>
namespace RetroMediaLab {
TerminalView::TerminalView(ThemeService& themes, Translations::TranslationService& translations, ConsoleState& state, QWidget* parent)
    : QWidget(parent), state_(state), themes_(themes) {
    setObjectName(AppConstants::Console);
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(AppConstants::TerminalViewValues::Padding, AppConstants::TerminalViewValues::Padding, AppConstants::TerminalViewValues::Padding, AppConstants::TerminalViewValues::Padding);
    auto* line = new QHBoxLayout;
    prompt_ = new QLabel(QString::fromStdString(translations.text(AppConstants::ShellTerminalPrompt)), this);
    line->addWidget(prompt_);
    command_ = new QPlainTextEdit(this);
    command_->setObjectName(AppConstants::CommandPreview);
    command_->setReadOnly(true);
    command_->setMaximumHeight(AppConstants::TerminalViewValues::CommandHeight);
    command_->setPlainText(state.commandPreview);
    line->addWidget(command_, 1);
    outer->addLayout(line);
    output_ = new QPlainTextEdit(this);
    output_->setObjectName(AppConstants::ConsoleOutput);
    output_->setReadOnly(true);
    output_->setPlainText(state.output);
    outer->addWidget(output_, 1);
    copy_ = new QPushButton(this);
    copy_->setObjectName(AppConstants::CopyConsole);
    copy_->setFlat(true);
    copy_->setIconSize(QSize(AppConstants::TerminalViewValues::CopyIconSize, AppConstants::TerminalViewValues::CopyIconSize));
    copy_->setFixedSize(AppConstants::TerminalViewValues::CopyButtonSize, AppConstants::TerminalViewValues::CopyButtonSize);
    outer->addWidget(copy_, 0, Qt::AlignRight);
    for (auto* edit : {command_, output_}) {
        edit->setFrameShape(QFrame::NoFrame);
        edit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    }
    connect(copy_, &QPushButton::clicked, this, [this] {
        QApplication::clipboard()->setText(output_->toPlainText());
    });
    const auto texts = [this, &translations] {
        const auto copyText = QString::fromStdString(translations.text(AppConstants::CommonCopy));
        copy_->setToolTip(copyText);
        copy_->setAccessibleName(copyText);
        command_->setAccessibleName(QString::fromStdString(translations.text(AppConstants::CommonCommand)));
        output_->setAccessibleName(QString::fromStdString(translations.text(AppConstants::ToolRawOutput)));
        setAccessibleName(QString::fromStdString(translations.text(AppConstants::CommonConsole)));
        if (state_.output.isEmpty() && (output_->toPlainText().isEmpty() || output_->toPlainText() == initialStatus_)) {
            initialStatus_ = QString::fromStdString(translations.text(AppConstants::StatusReady));
            output_->setPlainText(initialStatus_);
        }
    };
    texts();
    languageSubscription_ = translations.subscribe(texts);
    const auto colors = [this] {
        auto palette = output_->palette();
        palette.setColor(QPalette::Base, themes_.data()->color(AppConstants::TerminalBackground));
        palette.setColor(QPalette::Text, themes_.data()->color(AppConstants::TerminalText));
        output_->setPalette(palette);
        palette.setColor(QPalette::Text, themes_.data()->color(AppConstants::TerminalCommand));
        command_->setPalette(palette);
        palette.setColor(QPalette::WindowText, themes_.data()->color(AppConstants::TerminalPrompt));
        prompt_->setPalette(palette);
        update();
    };
    colors();
    themeSubscription_ = themes.subscribe(colors);
    auto cursor = output_->textCursor();
    cursor.setPosition(qMin(state.selectionAnchor, output_->document()->characterCount() - 1));
    cursor.setPosition(qMin(state.selectionPosition, output_->document()->characterCount() - 1), QTextCursor::KeepAnchor);
    output_->setTextCursor(cursor);
    cursor = command_->textCursor();
    const int commandEnd = command_->document()->characterCount() - 1;
    cursor.setPosition(qBound(0, state.commandAnchor, commandEnd));
    cursor.setPosition(qBound(0, state.commandPosition, commandEnd), QTextCursor::KeepAnchor);
    command_->setTextCursor(cursor);
    restoreScroll();
    outputSubscription_=state.subscribe([this] {
        if(command_->toPlainText()!=state_.commandPreview)command_->setPlainText(state_.commandPreview);
        const auto previous=output_->toPlainText();
        if(previous==state_.output)return;
        const bool atEnd=output_->verticalScrollBar()->value()==output_->verticalScrollBar()->maximum();
        const auto selection=output_->textCursor();
        if(state_.output.startsWith(previous)) {
            auto tail=output_->textCursor();tail.movePosition(QTextCursor::End);tail.insertText(state_.output.mid(previous.size()));
            output_->setTextCursor(selection);
        } else output_->setPlainText(state_.output);
        if(atEnd)output_->verticalScrollBar()->setValue(output_->verticalScrollBar()->maximum());
    });
}
void TerminalView::setCopyIcon(const QIcon& icon) { copy_->setIcon(icon); }
void TerminalView::restoreScroll() {
    output_->verticalScrollBar()->setValue(state_.scroll);
    output_->horizontalScrollBar()->setValue(state_.horizontalScroll);
    command_->verticalScrollBar()->setValue(state_.commandScroll);
}
void TerminalView::synchronizeState() {
    state_.commandPreview = command_->toPlainText();
    if (!state_.output.isEmpty() || output_->toPlainText() != initialStatus_) state_.output = output_->toPlainText();
    // The translated initial status is presentation, not stored console data.
    state_.selectionAnchor = output_->textCursor().anchor();
    state_.selectionPosition = output_->textCursor().position();
    state_.scroll = output_->verticalScrollBar()->value();
    state_.horizontalScroll = output_->horizontalScrollBar()->value();
    state_.commandAnchor = command_->textCursor().anchor();
    state_.commandPosition = command_->textCursor().position();
    state_.commandScroll = command_->verticalScrollBar()->value();
}
void TerminalView::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(themes_.data()->color(AppConstants::TerminalBackground));
    painter.setPen(themes_.data()->color(AppConstants::TerminalBorder));
    painter.drawRoundedRect(QRectF(rect()).adjusted(AppConstants::TerminalViewValues::BorderInset,AppConstants::TerminalViewValues::BorderInset,-AppConstants::TerminalViewValues::BorderInset,-AppConstants::TerminalViewValues::BorderInset), AppConstants::TerminalViewValues::CornerRadius, AppConstants::TerminalViewValues::CornerRadius);
}
}
