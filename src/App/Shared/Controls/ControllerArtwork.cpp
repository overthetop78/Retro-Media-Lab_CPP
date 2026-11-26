#include "Shared/Presentation/AssetPath.h"
#include "Shared/Controls/ControllerArtwork.h"
#include "Shared/Controls/MachineInputBindings.h"
#include "Shared/Emulation/EmulationPhysicalShortcuts.h"

#include <QApplication>
#include <QKeyEvent>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMouseEvent>
#include <QPainter>
#include <cmath>
#include <limits>

namespace RetroMediaLab {
namespace {
using Control = Emulation::ControllerVisualControl;
constexpr auto ResourceDirectory = "Controllers/";
constexpr auto ProfileCatalog = "profiles.json";
constexpr auto ImageProperty = "image";
constexpr auto ZonesProperty = "zones";
constexpr QSizeF CanvasSize(620, 520), ImageArea(580, 480);
constexpr int MinimumHeight = 150, PreferredHeight = 270;
constexpr int ZoneFields = 6;
constexpr qreal Percentage = 100.0, EllipseRadius = 0.5;
constexpr qreal JoystickTravel = 0.2, JoystickMarker = 0.2;
constexpr qreal HorizontalWidth = 0.44, HorizontalHeight = 0.18;
constexpr qreal VerticalWidth = 0.18, VerticalHeight = 0.44;
constexpr qreal EdgeWidth = 1.5, CornerRadius = 5.0;
constexpr float HighlightOpacity = 0.45f, VisiblePressure = 0.01f;
constexpr auto PollInterval = std::chrono::milliseconds(33);
namespace Physical = EmulationPhysicalShortcuts;

enum class Direction { None, Left, Right, Up, Down };

Direction direction(Control control) {
    switch (control) {
    case Control::DirectionLeft: case Control::StickLeft: case Control::RightStickLeft: return Direction::Left;
    case Control::DirectionRight: case Control::StickRight: case Control::RightStickRight: return Direction::Right;
    case Control::DirectionUp: case Control::StickUp: case Control::RightStickUp: return Direction::Up;
    case Control::DirectionDown: case Control::StickDown: case Control::RightStickDown: return Direction::Down;
    default: return Direction::None;
    }
}

const QJsonObject& profiles() {
    static const auto catalog = [] {
        QFile file(assetPath(QString::fromLatin1(ResourceDirectory) + QLatin1String(ProfileCatalog)));
        return file.open(QIODevice::ReadOnly) ? QJsonDocument::fromJson(file.readAll()).object() : QJsonObject{};
    }();
    return catalog;
}
}

ControllerArtwork* ControllerArtwork::create(const QString& visualId, Commands commands, Assign assign, QWidget* parent) {
    if (visualId.isEmpty()) return nullptr;
    const auto profile = profiles().value(visualId).toObject();
    const auto file = profile.value(QLatin1String(ImageProperty)).toString();
    if (file.isEmpty()) return nullptr;
    QPixmap image(assetPath(QString::fromLatin1(ResourceDirectory) + file));
    if (image.isNull()) return nullptr;
    QList<Zone> zones;
    for (const auto& entry : profile.value(QLatin1String(ZonesProperty)).toArray()) {
        const auto values = entry.toArray();
        if (values.size() != ZoneFields) continue;
        const auto control = static_cast<Control>(values.at(0).toInt());
        if (!commands.contains(control)) continue;
        zones.append({control, static_cast<Shape>(values.at(1).toInt()), {
            values.at(2).toDouble() / Percentage, values.at(3).toDouble() / Percentage,
            values.at(4).toDouble() / Percentage, values.at(5).toDouble() / Percentage}});
    }
    return new ControllerArtwork(std::move(image), std::move(zones), std::move(commands), std::move(assign), parent);
}

ControllerArtwork::ControllerArtwork(QPixmap image, QList<Zone> zones, Commands commands, Assign assign, QWidget* parent)
    : QWidget(parent), image_(std::move(image)), zones_(std::move(zones)), commands_(std::move(commands)), assign_(std::move(assign)) {
    setMinimumHeight(MinimumHeight);
    setMaximumHeight(PreferredHeight);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setMouseTracking(!zones_.isEmpty());
}

void ControllerArtwork::observeInput(Platform::GamepadSource& source, MachineInputBindings& bindings) {
    if (zones_.isEmpty()) return;
    source_ = &source;
    bindings_ = &bindings;
    connect(&timer_, &QTimer::timeout, this, [this] { pollInput(); });
}

void ControllerArtwork::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    if (!bindings_) return;
    qApp->installEventFilter(this);
    timer_.start(PollInterval);
}

void ControllerArtwork::hideEvent(QHideEvent* event) {
    timer_.stop();
    if (bindings_) qApp->removeEventFilter(this);
    pressedKeys_.clear();
    liveValues_.clear();
    QWidget::hideEvent(event);
}

bool ControllerArtwork::eventFilter(QObject* watched, QEvent* event) {
    const auto type = event->type();
    if (type == QEvent::ApplicationDeactivate || (type == QEvent::WindowDeactivate && watched == window()))
        pressedKeys_.clear();
    if (type == QEvent::KeyPress || type == QEvent::KeyRelease) {
        const auto* target = qobject_cast<QWidget*>(watched);
        const auto* key = static_cast<QKeyEvent*>(event);
        if (target && target->window() == window() && !key->isAutoRepeat()) {
            if (type == QEvent::KeyPress) pressedKeys_.insert(key->key());
            else pressedKeys_.remove(key->key());
        }
    }
    return false;
}

void ControllerArtwork::pollInput() {
    QHash<Control, float> values;
    if (window()->isActiveWindow()) {
        const auto pads = source_->read();
        const auto modifiers = QApplication::queryKeyboardModifiers();
        const Physical::Input input{QApplication::mouseButtons(), {}, pads, modifiers, pressedKeys_};
        for (auto command = commands_.cbegin(); command != commands_.cend(); ++command) {
            const auto chord = Physical::chord(bindings_->binding(command.value()));
            const bool keyboardActive = chord.keyboard && !chord.reserved &&
                (modifiers & chord.modifiers) == chord.modifiers &&
                std::ranges::all_of(chord.keys, [&](int key) { return pressedKeys_.contains(key); });
            if (chord.reserved || (chord.keyboard && !keyboardActive)) continue;
            float pressure = 1.0f;
            for (const auto& source : chord.sources) {
                const auto value = source.startsWith(QLatin1String(Physical::ControllerPrefix), Qt::CaseInsensitive)
                    ? Physical::controllerPressure(source, pads) : static_cast<float>(Physical::pressed(source, input));
                pressure = std::min(pressure, value);
            }
            if ((keyboardActive || !chord.sources.isEmpty()) && pressure > VisiblePressure)
                values.insert(command.key(), pressure);
        }
    }
    if (liveValues_ == values) return;
    liveValues_ = std::move(values);
    update();
}

float ControllerArtwork::activation(const Zone& zone) const {
    return &zone == hovered_ ? 1.0f : liveValues_.value(zone.control);
}

QSize ControllerArtwork::sizeHint() const {
    return {qRound(PreferredHeight * CanvasSize.width() / CanvasSize.height()), PreferredHeight};
}

QRectF ControllerArtwork::artworkBounds() const {
    const auto scale = std::min(width() / CanvasSize.width(), height() / CanvasSize.height());
    const auto fitted = QSizeF(image_.size()).scaled(ImageArea * scale, Qt::KeepAspectRatio);
    return QRectF(QPointF((width() - fitted.width()) / 2, (height() - fitted.height()) / 2), fitted);
}

QRectF ControllerArtwork::boundsFor(const Zone& zone) const {
    const auto image = artworkBounds();
    return {image.left() + zone.bounds.left() * image.width(), image.top() + zone.bounds.top() * image.height(),
        zone.bounds.width() * image.width(), zone.bounds.height() * image.height()};
}

const ControllerArtwork::Zone* ControllerArtwork::hit(QPointF point) const {
    for (auto it = zones_.crbegin(); it != zones_.crend(); ++it) {
        const auto bounds = boundsFor(*it);
        if (!bounds.contains(point)) continue;
        const auto delta = point - bounds.center();
        if (it->shape == Shape::Ellipse) {
            if (bounds.isEmpty()) continue;
            const auto x = delta.x() / (bounds.width() * EllipseRadius);
            const auto y = delta.y() / (bounds.height() * EllipseRadius);
            if (x * x + y * y <= 1) return &*it;
        } else if (it->shape == Shape::RoundedRectangle) {
            return &*it;
        } else {
            bool horizontal = false, vertical = false;
            for (const auto& candidate : zones_) {
                if (candidate.shape != it->shape || candidate.bounds != it->bounds) continue;
                const auto side = direction(candidate.control);
                horizontal |= side == Direction::Left || side == Direction::Right;
                vertical |= side == Direction::Up || side == Direction::Down;
            }
            constexpr auto Epsilon = std::numeric_limits<qreal>::epsilon();
            const auto x = std::abs(delta.x()), y = std::abs(delta.y());
            if ((x < Epsilon && y < Epsilon) || (!vertical && x < Epsilon) || (!horizontal && y < Epsilon)) continue;
            const auto sector = horizontal && (!vertical || x > y)
                ? (delta.x() < 0 ? Direction::Left : Direction::Right)
                : (delta.y() < 0 ? Direction::Up : Direction::Down);
            if (direction(it->control) == sector) return &*it;
        }
    }
    return nullptr;
}

void ControllerArtwork::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.drawPixmap(artworkBounds(), image_, image_.rect());
    auto color = palette().color(QPalette::Highlight);
    painter.setPen(QPen(color, EdgeWidth));
    color.setAlphaF(HighlightOpacity);
    painter.setBrush(color);
    QList<QRectF> joysticks;
    for (const auto& zone : zones_) {
        auto bounds = boundsFor(zone);
        const auto side = direction(zone.control);
        if (zone.shape == Shape::JoystickDirection) {
            if (joysticks.contains(zone.bounds)) continue;
            joysticks.append(zone.bounds);
            qreal left = 0, right = 0, up = 0, down = 0;
            for (const auto& candidate : zones_) {
                if (candidate.shape != Shape::JoystickDirection || candidate.bounds != zone.bounds) continue;
                const auto pressure = static_cast<qreal>(activation(candidate));
                switch (direction(candidate.control)) {
                case Direction::Left: left = std::max(left, pressure); break;
                case Direction::Right: right = std::max(right, pressure); break;
                case Direction::Up: up = std::max(up, pressure); break;
                case Direction::Down: down = std::max(down, pressure); break;
                case Direction::None: break;
                }
            }
            if (left == right && up == down) continue;
            const auto size = std::min(bounds.width(), bounds.height());
            const auto movement = size * JoystickTravel;
            const auto center = bounds.center() + QPointF(right - left, down - up) * movement;
            painter.drawEllipse(center, size * JoystickMarker, size * JoystickMarker);
            continue;
        }
        if (activation(zone) <= VisiblePressure) continue;
        if (zone.shape == Shape::Ellipse) {
            painter.drawEllipse(bounds);
            continue;
        }
        if (zone.shape == Shape::DirectionalPad) {
            const auto center = bounds.center();
            const auto w = bounds.width(), h = bounds.height();
            if (side == Direction::Left || side == Direction::Right) {
                const auto segment = w * HorizontalWidth;
                bounds = {side == Direction::Left ? bounds.left() : bounds.right() - segment,
                    center.y() - h * HorizontalHeight / 2, segment, h * HorizontalHeight};
            } else if (side == Direction::Up || side == Direction::Down) {
                const auto segment = h * VerticalHeight;
                bounds = {center.x() - w * VerticalWidth / 2, side == Direction::Up ? bounds.top() : bounds.bottom() - segment,
                    w * VerticalWidth, segment};
            }
        }
        painter.drawRoundedRect(bounds, CornerRadius, CornerRadius);
    }
}

void ControllerArtwork::mouseMoveEvent(QMouseEvent* event) {
    const auto* zone = hit(event->position());
    if (zone == hovered_) return;
    hovered_ = zone;
    setCursor(hovered_ ? Qt::PointingHandCursor : Qt::ArrowCursor);
    update();
}

void ControllerArtwork::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && assign_) {
        if (const auto* zone = hit(event->position())) {
            assign_(commands_.value(zone->control));
            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void ControllerArtwork::leaveEvent(QEvent* event) {
    hovered_ = nullptr;
    unsetCursor();
    update();
    QWidget::leaveEvent(event);
}
}
