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
    const auto tabScaled = [widget](int value) { return qRound(value * DisplayDensity::factor(widget) * DisplayDensity::tabFactor(widget)); };
    if (type == CT_TabBarTab) {
        const auto* tab = qstyleoption_cast<const QStyleOptionTab*>(option);
        auto font = widget ? widget->font() : QFont();
        if (tab->state & State_Selected) font.setWeight(QFont::DemiBold);
        const QFontMetrics metrics(font);
        const auto icon = tab->icon.isNull() ? QSize(0, 0) : tab->iconSize;
        const auto gap = icon.width() > 0 ? tabScaled(ThemeConstants::Style::IconTextGap) : 0;
        size = QSize(metrics.horizontalAdvance(tab->text) + icon.width() + gap + tabScaled(data_->tabPaddingX) * ThemeConstants::Style::TabUnderlineBottomInset,
            qMax(metrics.height(), icon.height()) + tabScaled(data_->tabPaddingY) * ThemeConstants::Style::TabUnderlineBottomInset);
    }
    if (type == CT_MenuBarItem) size = content + QSize(scaled(ThemeConstants::Style::MenuBarHorizontalPadding), scaled(ThemeConstants::Style::MenuBarVerticalPadding));
    if (type == CT_MenuItem) size.setHeight(option->fontMetrics.height() + scaled(ThemeConstants::Style::MenuItemVerticalPadding));
    if (type == CT_PushButton || type == CT_ComboBox || type == CT_LineEdit)
        size.setHeight(qMax(option->fontMetrics.height() + scaled(ThemeConstants::Style::ControlVerticalPadding), scaled(data_->controlHeight)));
    if (type == CT_PushButton)
        size.setWidth(content.width() + scaled(ThemeConstants::Style::ControlHorizontalPadding));
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
    case PM_ButtonMargin: case PM_FocusFrameHMargin: case PM_FocusFrameVMargin:
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
            const auto scale = DisplayDensity::factor(widget) * DisplayDensity::tabFactor(widget);
            const auto inset = ThemeConstants::Style::TabUnderlineInset * scale;
            const auto bottom = ThemeConstants::Style::TabUnderlineBottomInset * scale;
            painter->setPen(QPen(data_->color(ThemeConstants::AccentBrush), ThemeConstants::Style::TabUnderlineThickness * scale));
            painter->drawLine(QPointF(option->rect.bottomLeft()) + QPointF(inset, -bottom), QPointF(option->rect.bottomRight()) + QPointF(-inset, -bottom));
        }
        painter->restore(); return;
    }
    if (element == CE_TabBarTabLabel) {
        const auto& tab = *qstyleoption_cast<const QStyleOptionTab*>(option);
        auto palette = tab.palette;
        palette.setColor(QPalette::WindowText, data_->color(tab.state & State_Selected ? ThemeConstants::AccentBrush : ThemeConstants::MutedTextBrush));
        auto font = widget ? widget->font() : painter->font();
        if (tab.state & State_Selected) font.setWeight(QFont::DemiBold);
        painter->save();
        painter->setFont(font);
        const QFontMetrics metrics(font);
        const auto icon = tab.icon.isNull() ? QSize(0, 0) : tab.iconSize;
        const auto scale = DisplayDensity::factor(widget) * DisplayDensity::tabFactor(widget);
        const auto gap = icon.width() > 0 ? qRound(ThemeConstants::Style::IconTextGap * scale) : 0;
        const auto textWidth = metrics.horizontalAdvance(tab.text);
        const auto contentWidth = textWidth + icon.width() + gap;
        const auto left = tab.rect.x() + (tab.rect.width() - contentWidth) / ThemeConstants::Style::TabUnderlineBottomInset;
        if (!tab.icon.isNull()) {
            QRect rect(left, tab.rect.y() + (tab.rect.height() - icon.height()) / ThemeConstants::Style::TabUnderlineBottomInset, icon.width(), icon.height());
            rect = QStyle::visualRect(tab.direction, tab.rect, rect);
            tab.icon.paint(painter, rect, Qt::AlignCenter, tab.state & State_Enabled ? QIcon::Normal : QIcon::Disabled);
        }
        QRect textRect(left + icon.width() + gap, tab.rect.y(), textWidth, tab.rect.height());
        textRect = QStyle::visualRect(tab.direction, tab.rect, textRect);
        drawItemText(painter, textRect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextShowMnemonic, palette, tab.state & State_Enabled, tab.text, QPalette::WindowText);
        painter->restore();
        return;
    }
    QProxyStyle::drawControl(element,option,painter,widget);
}
}
