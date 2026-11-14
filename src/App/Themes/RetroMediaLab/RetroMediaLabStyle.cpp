#include "ThemeConstants.h"
#include "Hardware/HardwareConstants.h"
#include "RetroMediaLabStyle.h"
#include "DisplayDensity.h"
#include <QPainter>
#include <QStyleFactory>
#include <QStyleOption>
#include <QComboBox>
namespace RetroMediaLab {
RetroMediaLabStyle::RetroMediaLabStyle(std::shared_ptr<const ThemeData> data)
    : QProxyStyle(QStyleFactory::create(ThemeConstants::Fusion)), data_(std::move(data)) {}
void RetroMediaLabStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
    if (element == PE_FrameTabBarBase) return;
    if (element == PE_FrameTabWidget) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setBrush(data_->color(ThemeConstants::WindowBrush));
        painter->setPen(data_->color(ThemeConstants::BorderBrush));
        painter->drawRoundedRect(QRectF(option->rect).adjusted(ThemeConstants::Style::BorderInset, ThemeConstants::Style::BorderInset, -ThemeConstants::Style::BorderInset, -ThemeConstants::Style::BorderInset), ThemeConstants::Style::TabFrameRadius, ThemeConstants::Style::TabFrameRadius);
        painter->restore();
        return;
    }
    if (element == PE_PanelButtonCommand || element == PE_PanelLineEdit) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        auto fill = data_->color(ThemeConstants::ControlBrush);
        if (element == PE_PanelButtonCommand && (option->state & State_Sunken)) fill = data_->color(ThemeConstants::SelectedBrush);
        else if (option->state & State_MouseOver) fill = data_->color(ThemeConstants::HoverBrush);
        painter->setBrush(fill);
        painter->setPen(data_->color(option->state & State_HasFocus ? ThemeConstants::AccentBrush : ThemeConstants::BorderBrush));
        painter->drawRoundedRect(QRectF(option->rect).adjusted(ThemeConstants::Style::ButtonBorderInset, ThemeConstants::Style::ButtonBorderInset, -ThemeConstants::Style::ButtonBorderInset, -ThemeConstants::Style::ButtonBorderInset), data_->controlRadius, data_->controlRadius);
        painter->restore();
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}
QSize RetroMediaLabStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& content, const QWidget* widget) const {
    auto size = QProxyStyle::sizeFromContents(type, option, content, widget);
    const auto scaled = [widget](int value) { return qRound(value * DisplayDensity::factor(widget)); };
    if (type == CT_TabBarTab) {
        const auto* tab = qstyleoption_cast<const QStyleOptionTab*>(option);
        size = QSize(tab->fontMetrics.horizontalAdvance(tab->text) + scaled(ThemeConstants::Style::TabIconExtent) + scaled(ThemeConstants::Style::IconTextGap) + scaled(data_->tabPaddingX) * ThemeConstants::Style::TabUnderlineBottomInset + ThemeConstants::Style::TabUnderlineThickness,
                     qMax(scaled(ThemeConstants::Style::TabIconExtent), tab->fontMetrics.height()) + scaled(data_->tabPaddingY) * ThemeConstants::Style::TabUnderlineBottomInset);
    }
    if (type == CT_TabBarTab && widget && widget->objectName() == HardwareConstants::CategoryTabBar) {
        const auto* tab = qstyleoption_cast<const QStyleOptionTab*>(option);
        size.setWidth(tab->fontMetrics.horizontalAdvance(tab->text) + scaled(HardwareConstants::CategoryTabPadding));
    }
    if (type == CT_MenuBarItem) size = content + QSize(scaled(ThemeConstants::Style::MenuBarHorizontalPadding), scaled(ThemeConstants::Style::MenuBarVerticalPadding));
    if (type == CT_MenuItem) size.setHeight(option->fontMetrics.height() + scaled(ThemeConstants::Style::MenuItemVerticalPadding));
    if (type == CT_PushButton || type == CT_ComboBox || type == CT_LineEdit)
        size.setHeight(qMax(size.height(), scaled(data_->controlHeight)));
    return size;
}
int RetroMediaLabStyle::styleHint(StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* result) const {
    if (hint == SH_ComboBox_Popup) return 0;
    return QProxyStyle::styleHint(hint, option, widget, result);
}
void RetroMediaLabStyle::polish(QWidget* widget) {
    QProxyStyle::polish(widget);
    if (auto* combo = qobject_cast<QComboBox*>(widget)) combo->setMaxVisibleItems(ThemeConstants::Style::TabUnderlineInset);
}
int RetroMediaLabStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const {
    if (metric == PM_TabBarBaseHeight || metric == PM_TabBarBaseOverlap) return 0;
    if (metric == PM_TabBarTabOverlap) return 0;
    if (metric == PM_MenuBarPanelWidth) return 0;
    const auto base = QProxyStyle::pixelMetric(metric, option, widget);
    switch (metric) {
    case PM_SmallIconSize: case PM_ButtonIconSize: case PM_ToolBarIconSize:
    case PM_IndicatorWidth: case PM_IndicatorHeight:
    case PM_ExclusiveIndicatorWidth: case PM_ExclusiveIndicatorHeight:
    case PM_CheckBoxLabelSpacing: case PM_ScrollBarExtent:
    case PM_SliderThickness: case PM_SliderLength: case PM_TabBarScrollButtonWidth:
        return qMax(1, qRound(base * DisplayDensity::factor(widget)));
    default: return base;
    }
}
void RetroMediaLabStyle::drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
    if (element == CE_TabBarTabShape) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        const bool selected = option->state & State_Selected;
        painter->setBrush(data_->color(!selected && (option->state & State_MouseOver) ? ThemeConstants::HoverBrush : ThemeConstants::CardBrush));
        painter->setPen(data_->color(ThemeConstants::BorderBrush));
        painter->drawRoundedRect(QRectF(option->rect).adjusted(ThemeConstants::Style::BorderInset,ThemeConstants::Style::BorderInset,-ThemeConstants::Style::TabRightInset,ThemeConstants::Style::TabBottomExtension),data_->tabRadius,data_->tabRadius);
        if(selected) {
            painter->setPen(QPen(data_->color(ThemeConstants::AccentBrush),ThemeConstants::Style::TabUnderlineThickness));
            painter->drawLine(option->rect.bottomLeft()+QPoint(ThemeConstants::Style::TabUnderlineInset,-ThemeConstants::Style::TabUnderlineBottomInset),option->rect.bottomRight()+QPoint(-ThemeConstants::Style::TabUnderlineInset,-ThemeConstants::Style::TabUnderlineBottomInset));
        }
        painter->restore(); return;
    }
    if (element == CE_TabBarTabLabel) {
        auto tab = *qstyleoption_cast<const QStyleOptionTab*>(option);
        tab.palette.setColor(QPalette::WindowText,data_->color(tab.state & State_Selected ? ThemeConstants::AccentBrush : ThemeConstants::MutedTextBrush));
        painter->save();
        if(tab.state & State_Selected) { auto font=painter->font(); font.setWeight(QFont::DemiBold); painter->setFont(font); }
        QProxyStyle::drawControl(element,&tab,painter,widget); painter->restore(); return;
    }
    QProxyStyle::drawControl(element,option,painter,widget);
}
}
