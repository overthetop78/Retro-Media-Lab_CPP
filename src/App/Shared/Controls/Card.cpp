#include <Constants/AppConstants.h>
#include "Card.h"
#include "Themes/ThemeService.h"
#include <QPainter>
namespace RetroMediaLab {
Card::Card(std::shared_ptr<const ThemeData> theme, QWidget* parent) : QWidget(parent), theme_(std::move(theme)) {}
void Card::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(theme_->color(AppConstants::CardBrush));
    painter.setPen(theme_->color(AppConstants::BorderBrush));
    painter.drawRoundedRect(QRectF(rect()).adjusted(AppConstants::CardValues::BorderInset, AppConstants::CardValues::BorderInset, -AppConstants::CardValues::BorderInset, -AppConstants::CardValues::BorderInset), theme_->cardRadius, theme_->cardRadius);
}
}
