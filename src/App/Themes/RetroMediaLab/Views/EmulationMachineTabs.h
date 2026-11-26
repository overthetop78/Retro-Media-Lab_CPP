#pragma once

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QTabBar>
#include <QTabWidget>

namespace RetroMediaLab {

class EmulationMachineTabs final : public QTabWidget {
    class Bar final : public QTabBar {
    public:
        explicit Bar(QTabWidget& owner) : QTabBar(&owner), owner_(owner) {}

    protected:
        void mousePressEvent(QMouseEvent* event) override {
            if (event->button() == Qt::LeftButton) {
                resetDrag();
                press_ = event->position().toPoint();
                const int index = tabAt(press_);
                if (index > Home) dragged_ = owner_.widget(index);
            }
            QTabBar::mousePressEvent(event);
        }

        void mouseMoveEvent(QMouseEvent* event) override {
            const int source = owner_.indexOf(dragged_);
            if (source <= Home || !event->buttons().testFlag(Qt::LeftButton)) {
                resetDrag();
                QTabBar::mouseMoveEvent(event);
                return;
            }
            pointer_ = event->position().toPoint();
            if (preview_.isNull() && (pointer_ - press_).manhattanLength() >= QApplication::startDragDistance()) {
                const auto bounds = tabRect(source);
                offset_ = press_.x() - bounds.left();
                preview_ = grab(bounds);
            }
            if (!preview_.isNull()) update();
            else QTabBar::mouseMoveEvent(event);
        }

        void mouseReleaseEvent(QMouseEvent* event) override {
            if (event->button() != Qt::LeftButton) {
                QTabBar::mouseReleaseEvent(event);
                return;
            }
            const auto source = dragged_;
            const bool moving = !preview_.isNull();
            const auto pointer = event->position().toPoint();
            resetDrag();
            QTabBar::mouseReleaseEvent(event);
            const int from = owner_.indexOf(source);
            const int target = tabAt(pointer);
            if (!moving || from <= Home || target <= Home || from == target) return;
            int to = target + (afterCenter(target, pointer) ? 1 : 0);
            if (from < to) --to;
            if (from != to) moveTab(from, to);
            owner_.setCurrentWidget(source);
        }

        bool event(QEvent* event) override {
            if (event->type() == QEvent::UngrabMouse) resetDrag();
            return QTabBar::event(event);
        }

        void paintEvent(QPaintEvent* event) override {
            QTabBar::paintEvent(event);
            const int target = tabAt(pointer_);
            if (preview_.isNull() || target <= Home) return;
            const auto bounds = tabRect(target);
            const bool rightEdge = afterCenter(target, pointer_) != isRightToLeft();
            const int x = rightEdge ? bounds.right() : bounds.left();
            QPainter painter(this);
            painter.setOpacity(PreviewOpacity);
            painter.drawPixmap(pointer_.x() - offset_, bounds.top(), preview_);
            painter.setOpacity(1.0);
            painter.setPen(palette().color(QPalette::Highlight));
            painter.drawLine(x, bounds.top(), x, bounds.bottom());
        }

    private:
        bool afterCenter(int index, QPoint pointer) const {
            return (pointer.x() > tabRect(index).center().x()) != isRightToLeft();
        }

        void resetDrag() {
            dragged_.clear();
            if (preview_.isNull()) return;
            preview_ = {};
            update();
        }

        static constexpr int Home = 0;
        static constexpr qreal PreviewOpacity = 0.65;
        QTabWidget& owner_;
        QPointer<QWidget> dragged_;
        QPoint press_;
        QPoint pointer_;
        int offset_ = 0;
        QPixmap preview_;
    };

public:
    explicit EmulationMachineTabs(QWidget* parent) : QTabWidget(parent) {
        setTabBar(new Bar(*this));
    }
};

}
