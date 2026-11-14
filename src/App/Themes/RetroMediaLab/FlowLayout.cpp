#include "FlowLayout.h"
#include <QWidget>
#include <QStyle>
namespace RetroMediaLab {
FlowLayout::FlowLayout(QWidget* parent) : QLayout(parent) {}
FlowLayout::~FlowLayout() { while (auto* item = takeAt(0)) delete item; }
void FlowLayout::addItem(QLayoutItem* item) { items_.append(item); }
int FlowLayout::count() const { return items_.size(); }
QLayoutItem* FlowLayout::itemAt(int index) const { return items_.value(index); }
QLayoutItem* FlowLayout::takeAt(int index) { return index >= 0 && index < items_.size() ? items_.takeAt(index) : nullptr; }
bool FlowLayout::hasHeightForWidth() const { return true; }
int FlowLayout::heightForWidth(int width) const { return arrange(QRect(0, 0, width, 0), false); }
QSize FlowLayout::minimumSize() const {
    QSize result;
    for (auto* item : items_) result = result.expandedTo(item->minimumSize());
    const auto margins = contentsMargins();
    return result + QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
}
QSize FlowLayout::sizeHint() const { return minimumSize(); }
void FlowLayout::setGeometry(const QRect& rect) { QLayout::setGeometry(rect); arrange(rect, true); }
int FlowLayout::arrange(const QRect& rect, bool apply) const {
    const auto margins = contentsMargins();
    const auto area = rect.marginsRemoved(margins);
    int x = area.x(), y = area.y(), rowHeight = 0;
    const auto gap = qMax(0, spacing());
    for (auto* item : items_) {
        if (item->isEmpty()) continue;
        auto extent = item->sizeHint();
        extent.setWidth(qMin(extent.width(), area.width()));
        if (item->hasHeightForWidth()) extent.setHeight(item->heightForWidth(extent.width()));
        if (rowHeight > 0 && x + extent.width() > area.x() + area.width()) { x = area.x(); y += rowHeight + gap; rowHeight = 0; }
        if (apply) item->setGeometry(QStyle::visualRect(parentWidget()->layoutDirection(), area, QRect(QPoint(x, y), extent)));
        x += extent.width() + gap;
        rowHeight = qMax(rowHeight, extent.height());
    }
    return y + rowHeight - rect.y() + margins.bottom();
}
}
