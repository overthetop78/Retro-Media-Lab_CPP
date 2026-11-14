#include "ContentScrollArea.h"
#include "DisplayDensity.h"
#include <Constants/DisplayConstants.h>
#include <QEvent>
#include <QLayout>
#include <QTimer>
namespace RetroMediaLab {
ContentScrollArea::ContentScrollArea(int maximumContentHeight, QWidget* parent)
    : QScrollArea(parent), limit_(maximumContentHeight) {
    setProperty(DisplayConstants::ContentHeightProperty, true);
    setWidgetResizable(true);
    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    viewport()->installEventFilter(this);
}
void ContentScrollArea::setContent(QWidget* content) {
    setWidget(content);
    content->installEventFilter(this);
    scheduleHeight();
}
bool ContentScrollArea::eventFilter(QObject* object, QEvent* event) {
    switch (event->type()) {
    case QEvent::LayoutRequest: case QEvent::FontChange: case QEvent::Show: case QEvent::Resize:
        scheduleHeight(); break;
    default: break;
    }
    return QScrollArea::eventFilter(object, event);
}
void ContentScrollArea::resizeEvent(QResizeEvent* event) {
    QScrollArea::resizeEvent(event);
    scheduleHeight();
}
void ContentScrollArea::scheduleHeight() {
    if (pending_) return;
    pending_ = true;
    QTimer::singleShot(0, this, [this] {
        pending_ = false;
        if (!widget() || !widget()->layout()) return;
        auto* contentLayout = widget()->layout();
        contentLayout->activate();
        const auto width = viewport()->width();
        const auto wanted = contentLayout->hasHeightForWidth() ? contentLayout->totalHeightForWidth(width) : contentLayout->totalSizeHint().height();
        const auto height = qMin(qRound(limit_ * DisplayDensity::factor(this)), wanted + frameWidth() + frameWidth());
        if (minimumHeight() != height || maximumHeight() != height) setFixedHeight(height);
    });
}
}
