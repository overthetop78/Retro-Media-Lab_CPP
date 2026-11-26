#include <Constants/AppConstants.h>
#include "StatusIndicator.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
namespace RetroMediaLab {
StatusIndicator::StatusIndicator(QWidget* parent) : QWidget(parent), label_(new QLabel(this)) {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(AppConstants::StatusIndicatorValues::IndicatorWidth,0,AppConstants::StatusIndicatorValues::DotDiameter,0);
    layout->addWidget(label_);
}
void StatusIndicator::setStatus(const QString& text, const QColor& color) {
    if (label_->text() != text) {
        label_->setText(text);
        setAccessibleName(text);
    }
    if (color_ != color) {
        color_ = color;
        update();
    }
}
void StatusIndicator::paintEvent(QPaintEvent*) {
    QPainter painter(this); painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen); painter.setBrush(color_);
    painter.drawEllipse(QPointF(AppConstants::StatusIndicatorValues::LabelOffset, height()/2.), AppConstants::StatusIndicatorValues::LeftInset, AppConstants::StatusIndicatorValues::LeftInset);
}
}
