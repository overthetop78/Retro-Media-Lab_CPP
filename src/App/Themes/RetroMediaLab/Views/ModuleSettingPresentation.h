#pragma once

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPalette>
#include <QScrollArea>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidgetAction>
#include <array>

namespace RetroMediaLab {

class CompactSettingsCards final : public QWidget {
public:
    explicit CompactSettingsCards(QWidget* parent) : QWidget(parent) {
        auto* row = new QHBoxLayout(this);
        row->setContentsMargins(0, 0, 0, 0);
        for (auto*& column : columns_) {
            column = new QVBoxLayout;
            column->addStretch();
            row->addLayout(column, 1);
        }
        row->setStretch(1, 0);
    }

    void append(QWidget* card) {
        cards_.append(card);
        reflow();
    }

    void reflow() {
        constexpr int TwoColumnWidth = 760;
        const int count = width() >= TwoColumnWidth ? 2 : 1;
        QList<QWidget*> visible;
        for (auto* card : cards_)
            if (!card->isHidden()) visible.append(card);
        if (count == activeColumns_ && visible == visible_) return;
        for (auto* card : visible_)
            for (auto* column : columns_) column->removeWidget(card);
        for (qsizetype index = 0; index < visible.size(); ++index) {
            auto* column = columns_[index % count];
            column->insertWidget(column->count() - 1, visible[index]);
        }
        activeColumns_ = count;
        visible_ = std::move(visible);
        static_cast<QHBoxLayout*>(layout())->setStretch(1, count - 1);
    }

protected:
    void resizeEvent(QResizeEvent* event) override {
        QWidget::resizeEvent(event);
        reflow();
    }

private:
    std::array<QVBoxLayout*, 2> columns_{};
    QList<QWidget*> cards_, visible_;
    int activeColumns_ = 0;
};

inline void addModuleSettingRow(QFormLayout& form, QWidget* control, const QString& title,
        const QString& quickHelp, const QString& detailedHelp) {
    control->setAccessibleName(title);
    control->setToolTip(quickHelp);
    if (quickHelp.trimmed().isEmpty() || detailedHelp.trimmed().isEmpty()) {
        form.addRow(title, control);
        return;
    }

    constexpr int MaximumWidth = 320, MaximumHeight = 200, ContentPadding = 12;
    const QColor noteBackground(255, 245, 157), noteText(32, 29, 5);
    auto* heading = new QWidget(form.parentWidget());
    auto* headingLayout = new QHBoxLayout(heading);
    headingLayout->setContentsMargins(0, 0, 0, 0);
    auto* label = new QLabel(title, heading);
    label->setTextFormat(Qt::PlainText);
    label->setBuddy(control);
    label->setWordWrap(true);
    headingLayout->addWidget(label);

    auto* help = new QToolButton(heading);
    help->setAutoRaise(true);
    help->setIcon(help->style()->standardIcon(QStyle::SP_MessageBoxInformation));
    help->setToolTip(quickHelp);
    help->setAccessibleName(quickHelp);
    headingLayout->addWidget(help);
    headingLayout->addStretch();

    auto* menu = new QMenu(help);
    auto* action = new QWidgetAction(menu);
    auto* scroll = new QScrollArea;
    scroll->setFixedSize(MaximumWidth, MaximumHeight);
    scroll->setWidgetResizable(true);
    auto* note = new QWidget(scroll);
    const auto addText = [note, noteText](QVBoxLayout* layout, const QString& text, bool bold) {
        auto* line = new QLabel(text, note);
        line->setTextFormat(Qt::PlainText);
        line->setWordWrap(true);
        auto palette = line->palette();
        palette.setColor(QPalette::WindowText, noteText);
        line->setPalette(palette);
        auto font = line->font();
        font.setBold(bold);
        line->setFont(font);
        layout->addWidget(line);
    };
    auto palette = note->palette();
    palette.setColor(QPalette::Window, noteBackground);
    note->setPalette(palette);
    note->setAutoFillBackground(true);
    auto* contents = new QVBoxLayout(note);
    contents->setContentsMargins(ContentPadding, ContentPadding, ContentPadding, ContentPadding);
    addText(contents, title, true);
    addText(contents, detailedHelp, false);
    contents->addStretch();
    scroll->setWidget(note);
    action->setDefaultWidget(scroll);
    menu->addAction(action);
    help->setMenu(menu);
    help->setPopupMode(QToolButton::InstantPopup);
    form.addRow(heading, control);
}

}
