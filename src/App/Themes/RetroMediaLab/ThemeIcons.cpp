#include "ThemeIcons.h"
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
namespace RetroMediaLab {
QIcon themeIcon(const QString& id, const QColor& color) {
    QPixmap pixmap(48, 48);
    pixmap.setDevicePixelRatio(2);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(color, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);
    QPainterPath path;
    if (id == "read") {
        path.moveTo(6, 3); path.lineTo(20, 12); path.lineTo(6, 21); path.closeSubpath();
    } else if (id == "write") {
        path.moveTo(3, 16); path.lineTo(17, 2); path.quadTo(19, 1, 21, 4);
        path.lineTo(7, 20); path.lineTo(2, 22); path.closeSubpath();
        path.moveTo(15, 5); path.lineTo(19, 9); path.moveTo(3, 16); path.lineTo(7, 20);
    } else if (id == "convert") {
        path.moveTo(3, 9); path.cubicTo(6, 0, 18, 0, 21, 8);
        path.moveTo(21, 3); path.lineTo(21, 9); path.lineTo(15, 9);
        path.moveTo(21, 15); path.cubicTo(18, 24, 6, 24, 3, 16);
        path.moveTo(3, 21); path.lineTo(3, 15); path.lineTo(9, 15);
    } else if (id == "visualize") {
        path.moveTo(3, 3); path.lineTo(3, 21); path.lineTo(22, 21);
        path.moveTo(6, 16); path.lineTo(10, 10); path.lineTo(14, 14); path.lineTo(18, 7); path.lineTo(21, 11);
        path.moveTo(6, 18); path.lineTo(21, 18);
    } else if (id == "explorer") {
        path.moveTo(4, 2); path.lineTo(19, 2); path.lineTo(19, 21); path.lineTo(4, 21); path.closeSubpath();
        path.moveTo(8, 2); path.cubicTo(5, 6, 12, 8, 8, 12);
        path.cubicTo(5, 16, 12, 18, 8, 21);
    } else if (id == "tools") {
        path.moveTo(14, 3); path.cubicTo(9, 2, 8, 7, 10, 11);
        path.lineTo(2, 19); path.quadTo(1, 23, 5, 22); path.lineTo(13, 14);
        path.cubicTo(19, 16, 24, 11, 21, 6); path.lineTo(17, 10);
        path.lineTo(14, 7); path.closeSubpath();
    } else if (id == "emulation") {
        path.moveTo(7, 6); path.lineTo(17, 6);
        path.cubicTo(23, 7, 25, 22, 19, 19); path.lineTo(16, 16);
        path.lineTo(8, 16); path.cubicTo(0, 24, -1, 8, 7, 6);
        path.moveTo(5, 11); path.lineTo(11, 11); path.moveTo(8, 8); path.lineTo(8, 14);
        p.drawEllipse(QPointF(17, 10), .8, .8); p.drawEllipse(QPointF(19, 13), .8, .8);
    } else if (id == "copy") {
        p.drawRect(QRectF(4, 6, 13, 15));
        path.moveTo(8, 3); path.lineTo(21, 3); path.lineTo(21, 17); path.lineTo(19, 17);
    } else if (id == "preferences") {
        p.drawEllipse(QPointF(12, 12), 7, 7);
        p.drawEllipse(QPointF(12, 12), 3, 3);
        for (int i = 0; i < 8; ++i) {
            p.save(); p.translate(12, 12); p.rotate(i * 45);
            p.drawLine(QPointF(0, -7), QPointF(0, -10)); p.restore();
        }
    } else if (id == "console") {
        p.drawRect(QRectF(2, 4, 20, 16));
        path.moveTo(6, 8); path.lineTo(10, 12); path.lineTo(6, 16);
        path.moveTo(13, 16); path.lineTo(18, 16);
    }
    p.drawPath(path);
    return QIcon(pixmap);
}
}
