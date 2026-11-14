#include "RetroMediaLabStyle.h"
#include <QPainter>
#include <QStyleFactory>
#include <QStyleOption>
#include <QComboBox>
namespace RetroMediaLab {
RetroMediaLabStyle::RetroMediaLabStyle(std::shared_ptr<const ThemeData> data)
    : QProxyStyle(QStyleFactory::create("Fusion")), data_(std::move(data)) {}
void RetroMediaLabStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
    if (element == PE_FrameTabBarBase) return;
    if (element == PE_FrameTabWidget) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setBrush(data_->color("WindowBrush"));
        painter->setPen(data_->color("BorderBrush"));
        painter->drawRoundedRect(QRectF(option->rect).adjusted(.5, .5, -.5, -.5), 9, 9);
        painter->restore();
        return;
    }
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
    if (type == CT_TabBarTab) {
        const auto* tab = qstyleoption_cast<const QStyleOptionTab*>(option);
        size = QSize(tab->fontMetrics.horizontalAdvance(tab->text) + 17 + 8 + data_->tabPaddingX * 2 + 3,
                     qMax(17, tab->fontMetrics.height()) + data_->tabPaddingY * 2);
    }
    if (type == CT_MenuBarItem) size = content + QSize(28, 18);
    if (type == CT_MenuItem) size.setHeight(option->fontMetrics.height() + 14);
    if (type == CT_PushButton || type == CT_ComboBox || type == CT_LineEdit)
        size.setHeight(qMax(size.height(), data_->controlHeight));
    return size;
}
int RetroMediaLabStyle::styleHint(StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* result) const {
    if (hint == SH_ComboBox_Popup) return 0;
    return QProxyStyle::styleHint(hint, option, widget, result);
}
void RetroMediaLabStyle::polish(QWidget* widget) {
    QProxyStyle::polish(widget);
    if (auto* combo = qobject_cast<QComboBox*>(widget)) combo->setMaxVisibleItems(12);
}
int RetroMediaLabStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const {
    if (metric == PM_TabBarBaseHeight || metric == PM_TabBarBaseOverlap) return 0;
    if (metric == PM_TabBarTabOverlap) return 0;
    if (metric == PM_MenuBarPanelWidth) return 0;
    return QProxyStyle::pixelMetric(metric, option, widget);
}
void RetroMediaLabStyle::drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
    if (element == CE_TabBarTabShape) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        const bool selected = option->state & State_Selected;
        painter->setBrush(data_->color(!selected && (option->state & State_MouseOver) ? "HoverBrush" : "CardBrush"));
        painter->setPen(data_->color("BorderBrush"));
        painter->drawRoundedRect(QRectF(option->rect).adjusted(.5,.5,-3.5,5),data_->tabRadius,data_->tabRadius);
        if(selected) {
            painter->setPen(QPen(data_->color("AccentBrush"),3));
            painter->drawLine(option->rect.bottomLeft()+QPoint(12,-2),option->rect.bottomRight()+QPoint(-12,-2));
        }
        painter->restore(); return;
    }
    if (element == CE_TabBarTabLabel) {
        auto tab = *qstyleoption_cast<const QStyleOptionTab*>(option);
        tab.palette.setColor(QPalette::WindowText,data_->color(tab.state & State_Selected ? "AccentBrush" : "MutedTextBrush"));
        painter->save();
        if(tab.state & State_Selected) { auto font=painter->font(); font.setWeight(QFont::DemiBold); painter->setFont(font); }
        QProxyStyle::drawControl(element,&tab,painter,widget); painter->restore(); return;
    }
    QProxyStyle::drawControl(element,option,painter,widget);
}
}
