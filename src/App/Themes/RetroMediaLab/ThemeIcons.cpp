#include "Shared/Presentation/AssetPath.h"
#include <ErrorKeys.h>
#include "ThemeConstants.h"
#include "ThemeIcons.h"
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <stdexcept>
namespace RetroMediaLab {
namespace {
struct IconData {
    int canvas;
    qreal ratio, stroke;
    QMap<QString, QPainterPath> paths;
};
const IconData& iconData() {
    static const auto data = [] {
        QFile file(assetPath(QString::fromLatin1(ThemeConstants::ThemesRetroMediaLabIconsJson)));
        if (!file.open(QIODevice::ReadOnly)) throw std::runtime_error(ErrorKeys::CannotReadRetroMediaLabThemeData);
        QJsonParseError error;
        const auto document = QJsonDocument::fromJson(file.readAll(), &error);
        if (error.error != QJsonParseError::NoError || !document.isObject()) throw std::runtime_error(ErrorKeys::InvalidThemeJSON);
        const auto object = document.object();
        IconData result{object.value(ThemeConstants::CanvasSize).toInt(), object.value(ThemeConstants::DevicePixelRatio).toDouble(), object.value(ThemeConstants::StrokeWidth).toDouble()};
        if (result.canvas <= 0 || result.canvas > ThemeConstants::Icons::MaximumCanvasSize || result.ratio <= 0 || result.stroke <= 0)
            throw std::runtime_error(ErrorKeys::InvalidThemeMetric);
        const auto paths = object.value(ThemeConstants::Paths).toObject();
        for (auto it = paths.begin(); it != paths.end(); ++it) {
            QPainterPath path;
            for (const auto& value : it.value().toArray()) {
                const auto command = value.toArray();
                if (command.isEmpty()) throw std::runtime_error(ErrorKeys::InvalidThemeJSON);
                const auto op = command.at(ThemeConstants::Icons::OperationIndex).toString();
                const auto n = [&](int index) {
                    if (index >= command.size() || !command.at(index).isDouble()) throw std::runtime_error(ErrorKeys::InvalidThemeJSON);
                    return command.at(index).toDouble();
                };
                if (op == ThemeConstants::Move && command.size() == ThemeConstants::Icons::PairCommandLength) path.moveTo(n(ThemeConstants::Icons::FirstCoordinate), n(ThemeConstants::Icons::SecondCoordinate));
                else if (op == ThemeConstants::Line && command.size() == ThemeConstants::Icons::PairCommandLength) path.lineTo(n(ThemeConstants::Icons::FirstCoordinate), n(ThemeConstants::Icons::SecondCoordinate));
                else if (op == ThemeConstants::Quad && command.size() == ThemeConstants::Icons::QuadCommandLength) path.quadTo(n(ThemeConstants::Icons::FirstCoordinate), n(ThemeConstants::Icons::SecondCoordinate), n(ThemeConstants::Icons::ThirdCoordinate), n(ThemeConstants::Icons::FourthCoordinate));
                else if (op == ThemeConstants::Cubic && command.size() == ThemeConstants::Icons::CubicCommandLength) path.cubicTo(n(ThemeConstants::Icons::FirstCoordinate), n(ThemeConstants::Icons::SecondCoordinate), n(ThemeConstants::Icons::ThirdCoordinate), n(ThemeConstants::Icons::FourthCoordinate), n(ThemeConstants::Icons::FifthCoordinate), n(ThemeConstants::Icons::SixthCoordinate));
                else if (op == ThemeConstants::Rect && command.size() == ThemeConstants::Icons::QuadCommandLength) path.addRect(QRectF(n(ThemeConstants::Icons::FirstCoordinate), n(ThemeConstants::Icons::SecondCoordinate), n(ThemeConstants::Icons::ThirdCoordinate), n(ThemeConstants::Icons::FourthCoordinate)));
                else if (op == ThemeConstants::Ellipse && command.size() == ThemeConstants::Icons::QuadCommandLength) path.addEllipse(QPointF(n(ThemeConstants::Icons::FirstCoordinate), n(ThemeConstants::Icons::SecondCoordinate)), n(ThemeConstants::Icons::ThirdCoordinate), n(ThemeConstants::Icons::FourthCoordinate));
                else if (op == ThemeConstants::Close && command.size() == ThemeConstants::Icons::CloseCommandLength) path.closeSubpath();
                else throw std::runtime_error(ErrorKeys::InvalidThemeJSON);
            }
            result.paths.insert(it.key(), path);
        }
        return result;
    }();
    return data;
}
}
QIcon themeIcon(const QString& id, const QColor& color) {
    const auto& data = iconData();
    const auto found = data.paths.constFind(id);
    if (found == data.paths.cend()) throw std::runtime_error(ErrorKeys::InvalidThemeData);
    QPixmap pixmap(data.canvas, data.canvas);
    pixmap.setDevicePixelRatio(data.ratio);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(color, data.stroke, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(found.value());
    return QIcon(pixmap);
}
}
