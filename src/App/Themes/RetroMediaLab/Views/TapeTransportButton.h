#pragma once

#include "Shared/Emulation/TapeTransportPresentation.h"

#include <QPainter>
#include <QPolygonF>
#include <QToolButton>

namespace RetroMediaLab {
class TapeTransportButton final : public QToolButton {
    using TapeCommand = Emulation::TapeCommand;
    static constexpr QSize ButtonSize{31, 25};
    static constexpr QSize FaceSize{27, 21};
    static constexpr qreal FaceRadius = 4;

public:
    explicit TapeTransportButton(TapeCommand command, QWidget* parent)
        : QToolButton(parent), command_(command) {
        setAutoRaise(true);
        setFixedSize(ButtonSize);
    }

    void present(TapeButtonState state, bool blinkVisible) {
        setEnabled(state.enabled);
        if (state_ == state && (!state.blinking || blinkVisible_ == blinkVisible)) return;
        state_ = state;
        blinkVisible_ = blinkVisible;
        update();
    }

protected:
    void paintEvent(QPaintEvent* event) override {
        QToolButton::paintEvent(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.translate(QRectF(rect()).center());
        const auto color = foreground();
        const bool playOrRecord = command_ == TapeCommand::Play || command_ == TapeCommand::Record;
        painter.setOpacity(state_.blinking && !blinkVisible_ ? 0.25 :
            state_.supported ? 1.0 : playOrRecord ? 0.72 : 0.42);
        QRectF face(QPointF{}, QSizeF(FaceSize));
        face.moveCenter(QPointF{});
        constexpr qreal ActiveBorder = 2;
        constexpr qreal IdleBorder = 1;
        const auto stroke = state_.active ? ActiveBorder : IdleBorder;
        face.adjust(stroke / 2, stroke / 2, -stroke / 2, -stroke / 2);
        painter.setPen(QPen(color, stroke));
        auto fill = color;
        constexpr int PlayRecordAlpha = 55;
        constexpr int OtherActiveAlpha = 45;
        fill.setAlpha(state_.active ? (playOrRecord ? PlayRecordAlpha : OtherActiveAlpha) : 0);
        painter.setBrush(fill);
        painter.drawRoundedRect(face, FaceRadius, FaceRadius);

        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        drawGlyph(painter);
    }

private:
    QColor foreground() const {
        constexpr QColor ActivePlay{0, 200, 83}, SupportedPlay{22, 130, 59}, UnsupportedPlay{111, 156, 124};
        constexpr QColor ActiveRecord{235, 24, 54}, SupportedRecord{180, 35, 24}, UnsupportedRecord{185, 120, 120};
        constexpr QColor ActiveTransport{30, 144, 255}, IdleTransport{105, 105, 105}, UnsupportedTransport{128, 128, 128};
        if (command_ == TapeCommand::Play)
            return state_.active ? ActivePlay : state_.supported ? SupportedPlay : UnsupportedPlay;
        if (command_ == TapeCommand::Record)
            return state_.active ? ActiveRecord : state_.supported ? SupportedRecord : UnsupportedRecord;
        return !state_.supported ? UnsupportedTransport : state_.active ? ActiveTransport : IdleTransport;
    }

    void drawGlyph(QPainter& painter) const {
        constexpr QRectF RecordDot{-5, -5, 10, 10};
        constexpr QRectF StopSquare{-5, -5, 10, 10};
        constexpr QRectF PauseLeft{-4, -5, 3, 10}, PauseRight{1, -5, 3, 10};
        switch (command_) {
        case TapeCommand::Record: painter.drawEllipse(RecordDot); break;
        case TapeCommand::Stop: painter.drawRect(StopSquare); break;
        case TapeCommand::Pause:
            painter.drawRect(PauseLeft);
            painter.drawRect(PauseRight);
            break;
        case TapeCommand::Play:
            painter.drawPolygon(QPolygonF{{-4, -6}, {6, 0}, {-4, 6}});
            break;
        case TapeCommand::Rewind:
            painter.scale(-1, 1);
            [[fallthrough]];
        case TapeCommand::FastForward:
            painter.drawPolygon(QPolygonF{{-7, -5}, {0, 0}, {-7, 5}});
            painter.drawPolygon(QPolygonF{{0, -5}, {7, 0}, {0, 5}});
            break;
        }
    }

    TapeCommand command_;
    TapeButtonState state_{};
    bool blinkVisible_ = true;
};
}
