#include "RetroMediaLabStyle.h"
#include <QPainter>
#include <QStyleFactory>
#include <QStyleOption>
namespace RetroMediaLab {
RetroMediaLabStyle::RetroMediaLabStyle(std::shared_ptr<const ThemeData> data)
    : QProxyStyle(QStyleFactory::create("Fusion")), data_(std::move(data)) {}
void RetroMediaLabStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
    if (element == PE_PanelButtonCommand || element == PE_PanelLineEdit) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        auto fill = data_->color("ControlBrush");
        if (option->state & State_Sunken) fill = data_->color("SelectedBrush");
        else if (option->state & State_MouseOver) fill = data_->color("HoverBrush");
        painter->setBrush(fill);
        painter->setPen(data_->color(option->state & State_HasFocus ? "AccentBrush" : "BorderBrush"));
        painter->drawRoundedRect(QRectF(option->rect).adjusted(0.5, 0.5, -0.5, -0.5), data_->controlRadius, data_->controlRadius);
        painter->restore();
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}
QSize RetroMediaLabStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& content, const QWidget* widget) const {
    auto size = QProxyStyle::sizeFromContents(type, option, content, widget);
    if (type == CT_PushButton || type == CT_ComboBox || type == CT_LineEdit)
        size.setHeight(qMax(size.height(), data_->controlHeight));
    return size;
}
}
