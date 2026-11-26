#pragma once

#include "Shared/Controls/ControllerArtwork.h"
#include <RetroMediaLab/Platform/Gamepads.h>
#include <QPainter>
#include <QApplication>
#include <initializer_list>
#include <algorithm>
#include <array>
#include <cmath>

namespace RetroMediaLab::ControllerHostOverlays {
inline constexpr QSizeF Canvas{620, 320};
inline constexpr QSizeF ImageArea{540, 316};

class Drawing {
    using Model = ControllerArtwork::HostModel;
    using Button = Platform::GamepadButton;
    enum Axis { LeftX, LeftY, RightX, RightY, LeftTrigger, RightTrigger };
    enum class Direction { Up, Right, Down, Left };
    static constexpr float DirectionThreshold = 0.5f;
    static constexpr float StickThreshold = 0.03f;
    static constexpr float TriggerThreshold = 0.01f;
    static constexpr auto FallbackDevicePrefix = QLatin1String("rawgamecontroller:");
    static constexpr auto Guide = QLatin1String(Platform::RawGamepad::GuideButton);
    static constexpr auto Share = QLatin1String(Platform::RawGamepad::ShareButton);
    static constexpr int SwitchStates = 8;
    static constexpr std::array Directions{Direction::Up, Direction::Right, Direction::Down, Direction::Left};
    static constexpr std::array DirectionButtons{Platform::Up, Platform::Right, Platform::Down, Platform::Left};
    static constexpr std::array DirectionNames{QLatin1String("Up"), QLatin1String("Right"), QLatin1String("Down"), QLatin1String("Left")};
    static constexpr std::array FaceButtons{Platform::Confirm, Platform::Cancel, Platform::X, Platform::Y};
    static constexpr std::array DefaultDirectionIndices{10, 11, 12, 13};
    static constexpr std::array DisabledDirectionIndices{-1, -1, -1, -1};

public:
    Drawing(QPainter& painter, QRectF bounds, const Platform::GamepadState& input)
        : painter_(painter), bounds_(bounds), input_(input),
          fallback_(input.deviceId.startsWith(FallbackDevicePrefix, Qt::CaseInsensitive)) {}

    void paint(Model model) const {
        switch (model) {
        case Model::XboxRematchCore: drawXbox(true); break;
        case Model::XboxSeries: case Model::XboxOne: drawXbox(false); break;
        case Model::Xbox360: case Model::Xbox360White: drawXbox360(); break;
        case Model::GenericGamepad: drawGeneric(); break;
        case Model::PlayStation4: case Model::PlayStation5: drawPlayStation(true, true); break;
        case Model::PlayStation1: drawPlayStation(false, false); break;
        case Model::PlayStation2: drawPlayStation(false, true); break;
        case Model::MasterSystem:
            dpad(.255, .600, .055, 0, 1, DefaultDirectionIndices, .048, .15);
            rawHalos({{0, .675, .665, .055}, {1, .850, .665, .055}});
            break;
        case Model::NintendoEntertainmentSystem:
            dpad(.237, .562, .114, 0, 1, DefaultDirectionIndices, .046, .145);
            rawHalos({{0, .665, .674, .101}, {1, .772, .674, .101}});
            rawMarker(8, .405, .689, .075, .083, 8);
            rawMarker(9, .504, .689, .075, .083, 8);
            break;
        case Model::Nintendo64: drawNintendo64(); break;
        case Model::SuperNintendo:
            dpad(.214, .535, .085, 1, 0, DefaultDirectionIndices, .060, .090);
            rawHalos({{0, .781, .436, .055}, {1, .872, .532, .055}, {3, .690, .535, .055}, {2, .784, .630, .055}});
            rawMarker(8, .419, .574, .070, .077, 10);
            rawMarker(9, .524, .574, .070, .077, 10);
            rawMarker(4, .212, .230, .180, .063, 10);
            rawMarker(5, .790, .230, .180, .063, 10);
            break;
        case Model::MegaDrive3: case Model::MegaDrive6: drawMegaDrive(model == Model::MegaDrive6); break;
        case Model::Saturn:
            dpad(.245, .520, .045);
            rawHalos({{0,.660,.635,.042}, {1,.755,.565,.042}, {2,.850,.510,.042},
                {3,.630,.475,.042}, {4,.715,.420,.042}, {5,.800,.380,.042},
                {6,.500,.580,.042}, {7,.230,.170,.050}, {8,.770,.170,.050}});
            break;
        case Model::Dreamcast:
            stick(.250, .275, .065, true);
            dpad(.270, .530, .045);
            faces({{.700,.470}, {.800,.390}, {.625,.390}, {.700,.300}});
            halo(.500, .680, .035, button(Platform::Menu, 7));
            shoulders(.200, .800, .150);
            break;
        case Model::ArcadeStick: drawArcade(); break;
        case Model::RacingWheel: drawWheel(); break;
        case Model::FlightStick: drawFlight(); break;
        }
    }

private:
    static QColor fill() { return {255, 255, 255, 130}; }
    static QColor strong() { return {255, 255, 255, 245}; }
    QPointF point(qreal x, qreal y) const { return {bounds_.x() + x * bounds_.width(), bounds_.y() + y * bounds_.height()}; }
    qreal radius(qreal part) const { return std::min(bounds_.width(), bounds_.height()) * part; }

    float raw(QLatin1StringView prefix, int index, float missing = 0) const {
        return input_.controls.value(Platform::RawGamepad::controlName(prefix, index), missing);
    }
    bool rawButton(int index) const {
        return index >= 0 && raw(QLatin1String(Platform::RawGamepad::ButtonPrefix), index + int(fallback_)) >= DirectionThreshold;
    }
    float rawSigned(int index) const {
        return raw(QLatin1String(Platform::RawGamepad::AxisPrefix), index, Platform::RawGamepad::AxisCenter) * 2 - 1;
    }
    bool button(Button mask, int index) const { return (input_.buttons & mask) != 0 || rawButton(index); }
    bool system(QLatin1StringView name) const { return input_.controls.value(QString(name)) >= DirectionThreshold; }
    float horizontal(bool left) const {
        return input_.standardGamepad ? input_.axes.value(left ? LeftX : RightX) : rawSigned(left ? 0 : 2);
    }
    float vertical(bool left) const {
        return input_.standardGamepad ? -input_.axes.value(left ? LeftY : RightY) : rawSigned(left ? 1 : 3);
    }
    bool direction(Direction direction, int index, int x, int y) const {
        const auto i = static_cast<std::size_t>(direction);
        const auto flag = DirectionButtons[i];
        if (input_.standardGamepad) return (input_.buttons & flag) != 0;
        if (button(flag, index)) return true;
        return rawDirection(direction, x, y);
    }
    static bool matchesHat(Direction direction, int position) {
        if (position <= 0 || position > SwitchStates) return false;
        const int distance = std::abs(position - (int(direction) * 2 + 1));
        return distance <= 1 || distance == SwitchStates - 1;
    }
    bool rawDirection(Direction direction, int x, int y) const {
        const auto i = static_cast<std::size_t>(direction);
        const auto prefix = QLatin1String(Platform::RawGamepad::SwitchPrefix);
        if (fallback_ && input_.controls.value(Platform::RawGamepad::controlName(prefix, 1) + DirectionNames[i]) >= DirectionThreshold)
            return true;
        if (matchesHat(direction, static_cast<int>(raw(prefix, 0)))) return true;
        const bool vertical = direction == Direction::Up || direction == Direction::Down;
        const bool negative = direction == Direction::Up || direction == Direction::Left;
        return rawSigned(vertical ? y : x) * (negative ? -1 : 1) > DirectionThreshold;
    }

    void halo(qreal x, qreal y, qreal part, bool active) const {
        if (!active) return;
        const auto r = radius(part);
        const auto contained = std::max(2.0, r * .82);
        painter_.setPen(QPen(strong(), std::max(1.25, r * .08)));
        painter_.setBrush(fill());
        painter_.drawEllipse(point(x, y), contained, contained);
    }
    void stick(qreal x, qreal y, qreal part, bool left, bool showButton = true) const {
        halo(x, y, part, showButton && button(left ? Platform::LeftStick : Platform::RightStick, left ? 8 : 9));
        const auto dx = horizontal(left), dy = vertical(left);
        if (std::abs(dx) <= StickThreshold && std::abs(dy) <= StickThreshold) return;
        const auto r = radius(part);
        const auto moved = point(x, y) + QPointF(std::clamp(dx, -1.0f, 1.0f), std::clamp(dy, -1.0f, 1.0f)) * (r * .65);
        const auto marker = std::max(6.0, r * .55);
        painter_.setPen(QPen(strong(), std::max(1.5, r * .08)));
        painter_.setBrush(fill());
        painter_.drawEllipse(moved, marker, marker);
    }
    void dpad(qreal x, qreal y, qreal arm, int horizontalAxis = 0, int verticalAxis = 1,
        const std::array<int, 4>& indices = DefaultDirectionIndices, qreal width = .034, qreal length = .075) const {
        const auto center = point(x, y);
        const auto offset = radius(arm);
        const auto w = std::max(8.0, bounds_.width() * width);
        const auto h = std::max(8.0, bounds_.height() * length);
        const std::array rects{QRectF{center.x()-w/2, center.y()-offset-h/2, w, h},
            QRectF{center.x()+offset-h/2, center.y()-w/2, h, w},
            QRectF{center.x()-w/2, center.y()+offset-h/2, w, h},
            QRectF{center.x()-offset-h/2, center.y()-w/2, h, w}};
        painter_.setPen(QPen(strong(), 1.5));
        painter_.setBrush(fill());
        for (std::size_t i = 0; i < rects.size(); ++i) {
            if (!direction(Directions[i], indices[i], horizontalAxis, verticalAxis)) continue;
            const QRectF inset(rects[i].x()+2, rects[i].y()+2, std::max(1.0, rects[i].width()-4), std::max(1.0, rects[i].height()-4));
            const auto corner = std::max(3.0, std::min(inset.width(), inset.height()) * .3);
            painter_.drawRoundedRect(inset, corner, corner);
        }
    }
    void faces(std::initializer_list<QPointF> points) const {
        std::size_t i = 0;
        for (const auto& p : points) {
            halo(p.x(), p.y(), .045, button(FaceButtons[i], int(i)));
            ++i;
        }
    }
    void trigger(qreal x, float pressure) const {
        if (pressure <= TriggerThreshold) return;
        constexpr qreal TrackHeight = 7;
        const auto top = point(x - .08, .075) + QPointF(0, TrackHeight / 2);
        painter_.setPen(Qt::NoPen);
        painter_.setBrush(fill());
        painter_.drawRoundedRect(QRectF(top, QSizeF(bounds_.width() * .16,
            std::max(1.0, TrackHeight * .5 * std::clamp(pressure, 0.0f, 1.0f)))), 4, 4);
    }
    void shoulders(qreal left, qreal right, qreal y) const {
        halo(left, y, .050, button(Platform::LeftShoulder, 4));
        halo(right, y, .050, button(Platform::RightShoulder, 5));
        trigger(left, input_.axes.value(LeftTrigger));
        trigger(right, input_.axes.value(RightTrigger));
    }

    void drawXbox(bool rematch) const {
        stick(rematch ? .255 : .258, rematch ? .405 : .298, .08, true);
        stick(rematch ? .615 : .612, rematch ? .555 : .502, .08, false);
        if (rematch) {
            faces({{.730,.455}, {.790,.360}, {.665,.360}, {.730,.270}});
            halo(.430, .335, .032, button(Platform::View, 6));
            halo(.565, .335, .032, button(Platform::Menu, 7));
            halo(.500, .410, .027, system(Share));
            halo(.500, .255, .050, system(Guide));
            dpad(.390, .555, .040);
            shoulders(.275, .725, .210);
        } else {
            faces({{.736,.388}, {.795,.302}, {.674,.300}, {.739,.202}});
            halo(.434, .297, .032, button(Platform::View, 6));
            halo(.565, .297, .032, button(Platform::Menu, 7));
            halo(.500, .370, .027, system(Share));
            halo(.500, .173, .050, system(Guide));
            dpad(.382, .502, .040);
            shoulders(.275, .725, .150);
        }
        halo(.350, .650, .032, button(Platform::PaddleLeft1, 14));
        halo(.405, .715, .032, button(Platform::PaddleLeft2, 15));
        halo(.650, .650, .032, button(Platform::PaddleRight1, 16));
        halo(.595, .715, .032, button(Platform::PaddleRight2, 17));
    }
    void drawXbox360() const {
        stick(.249, .278, .078, true);
        stick(.610, .497, .078, false);
        dpad(.363, .498, .045);
        faces({{.735,.388}, {.799,.289}, {.670,.285}, {.735,.179}});
        halo(.402, .288, .027, button(Platform::View, 6));
        halo(.586, .287, .027, button(Platform::Menu, 7));
        halo(.499, .290, .052, system(Guide));
        shoulders(.270, .730, .080);
    }
    void drawGeneric() const {
        stick(.350, .550, .070, true);
        stick(.625, .550, .070, false);
        faces({{.765,.445}, {.835,.350}, {.695,.350}, {.765,.255}});
        dpad(.250, .350, .040);
        halo(.420, .350, .025, button(Platform::View, 6));
        halo(.565, .350, .025, button(Platform::Menu, 7));
        shoulders(.260, .740, .130);
    }
    void drawPlayStation(bool modern, bool analog) const {
        if (modern) {
            faces({{.790,.405}, {.855,.310}, {.725,.310}, {.790,.215}});
            dpad(.210, .305, .040);
            halo(.295, .135, .025, button(Platform::View, 6));
            halo(.705, .135, .025, button(Platform::Menu, 7));
            halo(.500, .525, .030, system(Guide));
        } else {
            faces({{.770,.510}, {.850,.410}, {.690,.410}, {.770,.310}});
            dpad(.205, .410, .040);
            halo(.430, .505, .025, button(Platform::View, 6));
            halo(.570, .505, .025, button(Platform::Menu, 7));
        }
        if (analog) {
            const auto y = modern ? .515 : .610;
            stick(.375, y, .070, true);
            stick(.625, y, .070, false);
        }
        shoulders(.220, .780, .145);
    }
    struct RawHalo { int index; qreal x, y, radius; };
    void rawHalos(std::initializer_list<RawHalo> buttons) const {
        for (const auto& b : buttons) halo(b.x, b.y, b.radius, rawButton(b.index));
    }
    void rawMarker(int index, qreal x, qreal y, qreal w, qreal h, qreal corner) const {
        if (!rawButton(index)) return;
        const auto center = point(x, y);
        w *= bounds_.width();
        h *= bounds_.height();
        painter_.setPen(Qt::NoPen);
        painter_.setBrush(fill());
        painter_.drawRoundedRect(QRectF(center.x() - w/2 + 3, center.y() - h/2 + 3,
            std::max(1.0, w - 6), std::max(1.0, h - 6)), corner - 2, corner - 2);
    }
    void drawNintendo64() const {
        dpad(.277, .374, .060, 3, 4, DisabledDirectionIndices, .043, .080);
        stick(.496, .563, .052, true, false);
        rawHalos({{5,.669,.454,.038}, {4,.620,.384,.038}, {9,.497,.382,.040},
            {0,.726,.265,.031}, {1,.768,.326,.031}, {2,.727,.389,.031}, {3,.685,.326,.031}});
        rawMarker(6, .249, .202, .085, .045, 7);
        rawMarker(7, .751, .202, .085, .045, 7);
        if (rawButton(8)) {
            const auto center = point(.500, .690);
            const QRectF badge(center.x()-13, center.y()-9, 26, 18);
            painter_.setPen(Qt::NoPen);
            painter_.setBrush(QColor(20,20,20,175));
            painter_.drawRoundedRect(badge, 7, 7);
            painter_.setPen(QColor(255,255,255,235));
            painter_.drawText(badge, Qt::AlignCenter, QLatin1String("Z"));
        }
    }
    void drawMegaDrive(bool six) const {
        dpad(.226, .523, .082, 1, 0, DefaultDirectionIndices, .058, .140);
        if (six) {
            rawHalos({{2,.680,.599,.071}, {1,.782,.568,.076}, {5,.878,.535,.076},
                {3,.671,.463,.055}, {0,.752,.421,.055}, {4,.831,.405,.055}});
            rawMarker(9, .500, .469, .081, .056, 10);
            rawMarker(8, .500, .590, .081, .056, 10);
        } else {
            rawHalos({{0,.655,.630,.052}, {1,.755,.570,.052}, {2,.850,.520,.052}});
            rawMarker(3, .500, .500, .070, .042, 6);
        }
    }
    template<class Input>
    bool specializedButton(const std::optional<Input>& state, typename Input::Button button, int index) const {
        return state ? (state->buttons & button) != 0 : rawButton(index);
    }

    float rawUnsigned(int index) const {
        return std::clamp(raw(QLatin1String(Platform::RawGamepad::AxisPrefix), index), 0.0f, 1.0f);
    }

    struct SpecializedHalo {
        qreal x, y, size;
        quint16 button;
        int rawIndex;
    };

    template<class Input>
    void specializedHalos(const std::optional<Input>& state, std::initializer_list<SpecializedHalo> values) const {
        for (const auto& value : values)
            halo(value.x, value.y, value.size,
                specializedButton(state, static_cast<typename Input::Button>(value.button), value.rawIndex));
    }

    void specializedDirections(QPointF center, qreal halfWidth, qreal length, qreal gap,
        const std::array<bool, 4>& active) const {
        const auto x = center.x(), y = center.y();
        const auto width = halfWidth * 2, reach = length + gap;
        const std::array<QRectF, 4> rects{{
            {x-halfWidth, y-reach, width, length}, {x+gap, y-halfWidth, length, width},
            {x-halfWidth, y+gap, width, length}, {x-reach, y-halfWidth, length, width}}};
        painter_.setPen(Qt::NoPen);
        painter_.setBrush(fill());
        for (std::size_t i = 0; i < rects.size(); ++i)
            if (active[i]) painter_.drawRoundedRect(rects[i], 3, 3);
    }

    void specializedBadge(QPointF center, const QString& label) const {
        constexpr qreal Radius = 11;
        painter_.setPen(Qt::NoPen);
        painter_.setBrush(fill());
        painter_.drawEllipse(center, Radius, Radius);
        painter_.setPen(Qt::black);
        painter_.drawText(QRectF(center - QPointF(Radius, Radius), QSizeF(2 * Radius, 2 * Radius)),
            Qt::AlignCenter, label);
    }

    void drawWheel() const {
        using Wheel = Platform::RacingWheelInput;
        const auto& wheel = input_.racingWheel;
        enum RawAxis { Steering, Throttle, Brake, Clutch, Handbrake };
        const auto steering = wheel ? wheel->wheel : rawSigned(Steering);
        constexpr qreal TravelRadians = 130 * 3.14159265358979323846 / 180;
        if (std::abs(steering) > TriggerThreshold) {
            const auto center = point(.500, .285);
            const auto angle = steering * TravelRadians;
            painter_.setPen(QPen(strong(), 2));
            painter_.drawLine(center, center + QPointF(std::sin(angle), -std::cos(angle)) * radius(.19));
        }
        specializedDirections(point(.395, .285), 4, 14, 4, {
            specializedButton(wheel, Wheel::Up, 6), specializedButton(wheel, Wheel::Right, 9),
            specializedButton(wheel, Wheel::Down, 7), specializedButton(wheel, Wheel::Left, 8)});
        specializedHalos(wheel, {
            {.565,.315,.024,Wheel::A,0}, {.585,.285,.024,Wheel::B,1},
            {.545,.285,.024,Wheel::X,2}, {.565,.255,.024,Wheel::Y,3},
            {.380,.335,.030,Wheel::PreviousGear,4}, {.620,.335,.030,Wheel::NextGear,5},
            {.470,.350,.022,Wheel::View,10}, {.530,.350,.022,Wheel::Menu,11},
            {.405,.380,.025,Wheel::LeftThumbstick,12}, {.595,.380,.025,Wheel::RightThumbstick,13}});
        const std::array pedals{
            std::pair{.345, wheel ? wheel->clutch : rawUnsigned(Clutch)},
            std::pair{.500, wheel ? wheel->brake : rawUnsigned(Brake)},
            std::pair{.655, wheel ? wheel->throttle : rawUnsigned(Throttle)}};
        painter_.setPen(Qt::NoPen);
        painter_.setBrush(fill());
        for (const auto& [x, pressure] : pedals) {
            if (pressure <= TriggerThreshold) continue;
            const auto center = point(x, .800);
            painter_.drawRoundedRect(QRectF(center - QPointF(radius(.035), radius(.080)),
                QSizeF(radius(.070), radius(.160))), 5, 5);
        }
        constexpr qreal BadgeInset = 18;
        const auto handbrake = wheel ? wheel->handbrake : rawUnsigned(Handbrake);
        if (handbrake > TriggerThreshold)
            specializedBadge(bounds_.bottomRight() - QPointF(BadgeInset, BadgeInset), QStringLiteral("HB"));
        if (wheel && wheel->gear != 0)
            specializedBadge(bounds_.bottomLeft() + QPointF(BadgeInset, -BadgeInset), QString::number(wheel->gear));
    }

    void drawFlight() const {
        using Flight = Platform::FlightStickInput;
        const auto& flight = input_.flightStick;
        enum RawAxis { Roll, Pitch, Yaw, Throttle, HatHorizontal, HatVertical };
        const auto roll = flight ? flight->roll : rawSigned(Roll);
        const auto pitch = flight ? flight->pitch : rawSigned(Pitch);
        if (std::abs(roll) > TriggerThreshold || std::abs(pitch) > TriggerThreshold) {
            painter_.setPen(Qt::NoPen);
            painter_.setBrush(fill());
            painter_.drawEllipse(point(.745 + roll * .025, .360 + pitch * .025), radius(.075), radius(.075));
        }
        specializedHalos(flight, {
            {.715,.190,.030,Flight::PrimaryFire,0}, {.700,.095,.030,Flight::SecondaryFire,1},
            {.760,.085,.025,Flight::A,2}, {.805,.115,.025,Flight::B,3},
            {.740,.125,.025,Flight::X,4}, {.785,.155,.025,Flight::Y,5},
            {.360,.570,.026,Flight::View,6}, {.430,.570,.026,Flight::Menu,7},
            {.360,.665,.032,Flight::LeftShoulder,8}, {.475,.665,.032,Flight::RightShoulder,9}});
        std::array<bool, 4> hat{};
        for (std::size_t i = 0; i < Directions.size(); ++i)
            hat[i] = flight ? matchesHat(Directions[i], flight->hat) : rawDirection(Directions[i], HatHorizontal, HatVertical);
        specializedDirections(point(.745, .075), 5, 15, 3, hat);
        const auto throttle = flight ? flight->throttle : rawUnsigned(Throttle);
        if (throttle > TriggerThreshold) {
            const auto top = point(.250, .430);
            const auto height = bounds_.height() * .270;
            const auto filled = std::max(1.0, height * std::clamp(throttle, 0.0f, 1.0f));
            painter_.drawRoundedRect(QRectF(top + QPointF(0, height - filled), QSizeF(8, filled)), 3, 3);
        }
        const auto yaw = flight ? flight->yaw : rawSigned(Yaw);
        if (std::abs(yaw) > TriggerThreshold)
            specializedBadge(bounds_.bottomRight() - QPointF(18, 18), QStringLiteral("Y"));
    }

    void drawArcade() const {
        using Arcade = Platform::ArcadeStickInput;
        const auto& arcade = input_.arcadeStick;
        const auto pressed = [&](Arcade::Button button, int index) { return specializedButton(arcade, button, index); };
        const auto direction = [&](Arcade::Button positive, Arcade::Button negative) {
            return int((arcade->buttons & positive) != 0) - int((arcade->buttons & negative) != 0);
        };
        const auto colors = QApplication::palette();
        const auto outline = colors.color(QPalette::Mid);
        const auto buttonFill = [&colors](bool active) { return colors.color(active ? QPalette::Highlight : QPalette::Button); };
        painter_.setBrush(colors.color(QPalette::Window));
        painter_.setPen(QPen(outline, 2));
        painter_.drawRoundedRect(QRectF(55, 68, 510, 224), 18, 18);
        const auto dx = (arcade ? direction(Arcade::Right, Arcade::Left) : horizontal(true)) * 45;
        const auto dy = (arcade ? direction(Arcade::Down, Arcade::Up) : vertical(true)) * 45;
        painter_.setPen(QPen(outline, 12, Qt::SolidLine, Qt::RoundCap));
        painter_.drawLine(QPointF(180, 230), QPointF(180 + dx, 145 + dy));
        painter_.setPen(QPen(outline, 2));
        painter_.setBrush(buttonFill(pressed(Arcade::Action1, 0)));
        painter_.drawEllipse(QPointF(180 + dx, 125 + dy), 28, 28);
        const auto arcadeButton = [&](qreal x, qreal y, qreal r, bool active, const QString& label) {
            painter_.setPen(QPen(outline, 2));
            painter_.setBrush(buttonFill(active));
            painter_.drawEllipse(QPointF(x,y), r, r);
            painter_.setPen(colors.color(active ? QPalette::HighlightedText : QPalette::ButtonText));
            painter_.drawText(QRectF(x-r,y-r,2*r,2*r), Qt::AlignCenter, label);
        };
        constexpr std::array actions{Arcade::Action1, Arcade::Action2, Arcade::Action3, Arcade::Action4,
            Arcade::Action5, Arcade::Action6, Arcade::Special1, Arcade::Special2};
        constexpr int Columns = 4;
        for (int i = 0; i < int(actions.size()); ++i)
            arcadeButton(350 + (i % Columns) * 48, 135 + (i / Columns) * 58, 21,
                pressed(actions[i], i), QString::number(i+1));
        arcadeButton(288, 101, 11, pressed(Arcade::View, 8), QStringLiteral("◫"));
        arcadeButton(330, 101, 11, pressed(Arcade::Menu, 9), QStringLiteral("☰"));
    }

    QPainter& painter_;
    QRectF bounds_;
    const Platform::GamepadState& input_;
    bool fallback_;
};
}
