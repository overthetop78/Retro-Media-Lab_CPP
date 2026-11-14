#include "Card.h"
#include "Themes/ThemeService.h"
#include <QPainter>
namespace RetroMediaLab {
Card::Card(std::shared_ptr<const ThemeData> theme, QWidget* parent) : QWidget(parent), theme_(std::move(theme)) {}
void Card::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(theme_->color("CardBrush"));
    painter.setPen(theme_->color("BorderBrush"));
    painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), theme_->cardRadius, theme_->cardRadius);
}
}
