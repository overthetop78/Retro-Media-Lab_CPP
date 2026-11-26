#include "MediaLoadingOverlay.h"
#include "LoadingConstants.h"
#include "../DisplayDensity.h"
#include "Shared/Controls/Card.h"
#include "Shared/Localization/MediaText.h"
#include <QEvent>
#include <RetroMediaLab/MediaEngine/ImageFormat.h>
#include <RetroMediaLab/Translations/TranslationConstants.h>
#include <QLabel>
#include <QSizePolicy>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
namespace RetroMediaLab {
namespace C=LoadingConstants;
MediaLoadingOverlay::MediaLoadingOverlay(ThemeService& themes,Translations::TranslationService& translations,std::function<void()> stop,QWidget* parent,bool alignTop,bool showFormats):QWidget(parent),translations_(translations){
    setObjectName(C::Overlay);auto* outer=new QVBoxLayout(this);outer->setContentsMargins(C::Padding,C::Padding,C::Padding,C::Padding);if(!alignTop)outer->addStretch();auto* card=new Card(themes.data(),this);card_=card;card->setFixedWidth(C::Width);auto* layout=new QVBoxLayout(card);layout->setContentsMargins(C::Padding,C::Padding,C::Padding,C::Padding);layout->setSpacing(C::Spacing);
    stage_=new QLabel(card);stage_->setObjectName(C::Stage);stage_->setTextFormat(Qt::PlainText);stage_->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Fixed);auto bold=stage_->font();bold.setBold(true);stage_->setFont(bold);detail_=new QLabel(card);detail_->setObjectName(C::Detail);detail_->setTextFormat(Qt::PlainText);detail_->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Fixed);progress_=new QProgressBar(card);progress_->setObjectName(C::Bar);stop_=new QPushButton(card);layout->addWidget(stage_);
    if (showFormats) {
        format_ = new QLabel(card);
        format_->setTextFormat(Qt::PlainText);
        format_->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Fixed);
        format_->setFont(bold);
        layout->addWidget(format_);
    }
    layout->addWidget(detail_);layout->addWidget(progress_);layout->addWidget(stop_,0,Qt::AlignRight);outer->addWidget(card,0,Qt::AlignHCenter);outer->addStretch();connect(stop_,&QPushButton::clicked,this,[stop=std::move(stop)]{stop();});parent->installEventFilter(this);setGeometry(parent->rect());hide();
}
bool MediaLoadingOverlay::eventFilter(QObject* object,QEvent* event){if(object==parentWidget()&&event->type()==QEvent::Resize)setGeometry(parentWidget()->rect());return QWidget::eventFilter(object,event);}
void MediaLoadingOverlay::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    card_->setFixedWidth(qMax(1, qMin(width() - C::Padding - C::Padding,
        qRound(C::Width * DisplayDensity::factor(this)))));
    refreshLabels();
}

void MediaLoadingOverlay::refreshLabels() {
    const auto margins = card_->layout()->contentsMargins();
    const auto width = qMax(0, card_->width() - margins.left() - margins.right());
    const auto update = [width](QLabel* label, const QString& value) {
        const auto height = label->fontMetrics().height();
        if (label->minimumHeight() != height) label->setFixedHeight(height);
        const auto line = value.simplified();
        const auto visibleText = label->fontMetrics().elidedText(line, Qt::ElideRight, width);
        if (label->text() != visibleText) label->setText(visibleText);
        label->setToolTip(visibleText == line ? QString{} : value);
    };
    update(stage_, stageText_);
    update(detail_, detailText_);
    if (format_) update(format_, formatText_);
}

void MediaLoadingOverlay::display(bool visible, const Infrastructure::LoadingProgress& value) {
    if (!visible) {
        if (!isHidden()) hide();
        formatTimer_.invalidate();
        formatText_.clear();
        return;
    }

    const auto stage = mediaText(translations_, value.stage);
    const auto detail = value.detailMessage ? mediaText(translations_, *value.detailMessage)
                                            : QString::fromStdString(value.detail);
    bool formatChanged = false;
    if (format_ && value.formatCheck && (!formatTimer_.isValid() || formatTimer_.elapsed() >= C::FormatDisplayMilliseconds)) {
        auto message = *value.formatCheck;
        if (!message.arguments.empty()) {
            if (const auto* format = MediaEngine::findImageFormat(message.arguments.front()))
                message.arguments.front() = MediaEngine::imageFormatName(*format, [this](std::string_view key) {
                    return translations_.text(std::string(key), TranslationConstants::MediaEngine);
                });
        }
        auto rendered = mediaText(translations_, message);
        if (rendered != formatText_) {
            formatText_ = std::move(rendered);
            formatChanged = true;
            formatTimer_.start();
        }
    }
    if (formatChanged || isHidden() || stage != stageText_ || detail != detailText_) {
        stageText_ = stage;
        detailText_ = detail;
        refreshLabels();
    }
    const auto maximum = value.indeterminate ? 0 : C::Maximum;
    if (progress_->maximum() != maximum) progress_->setRange(0, maximum);
    if (!value.indeterminate) progress_->setValue(static_cast<int>(value.value));

    const auto stopText = QString::fromStdString(translations_.text(C::Stop));
    if (stop_->text() != stopText) stop_->setText(stopText);
    if (isHidden()) {
        setGeometry(parentWidget()->rect());
        raise();
        show();
    }
}
}
