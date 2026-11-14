#include "StatusIndicator.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
namespace RetroMediaLab {
StatusIndicator::StatusIndicator(QWidget* parent) : QWidget(parent), label_(new QLabel(this)) {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(22,0,8,0);
    layout->addWidget(label_);
}
void StatusIndicator::setStatus(const QString& text, const QColor& color) {
    label_->setText(text); setAccessibleName(text); color_ = color; update();
}
void StatusIndicator::paintEvent(QPaintEvent*) {
    QPainter painter(this); painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen); painter.setBrush(color_);
    painter.drawEllipse(QPointF(10, height()/2.), 4, 4);
}
}
