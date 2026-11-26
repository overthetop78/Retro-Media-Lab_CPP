#pragma once
#include "AppApi.h"

#include <RetroMediaLab/Emulation/InputSettings.h>
#include <QPixmap>
#include <QSet>
#include <QTimer>
#include <QWidget>
#include <functional>

namespace RetroMediaLab {
namespace Platform { class GamepadSource; }
class MachineInputBindings;
class ControllerArtwork final : public QWidget {
public:
    using Commands = QHash<Emulation::ControllerVisualControl, QString>;
    using Assign = std::function<void(const QString&)>;
    RETROMEDIALAB_APP_API static ControllerArtwork* create(const QString& visualId, Commands, Assign, QWidget* parent);

    RETROMEDIALAB_APP_API void observeInput(Platform::GamepadSource&, MachineInputBindings&);
    QSize sizeHint() const override;

protected:
    void showEvent(QShowEvent*) override;
    void hideEvent(QHideEvent*) override;
    bool eventFilter(QObject*, QEvent*) override;
    void paintEvent(QPaintEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void leaveEvent(QEvent*) override;

private:
    enum class Shape { Ellipse, RoundedRectangle, DirectionalPad, JoystickDirection };
    struct Zone {
        Emulation::ControllerVisualControl control;
        Shape shape;
        QRectF bounds;
    };
    ControllerArtwork(QPixmap, QList<Zone>, Commands, Assign, QWidget* parent);
    QRectF artworkBounds() const;
    QRectF boundsFor(const Zone&) const;
    const Zone* hit(QPointF) const;
    void pollInput();
    float activation(const Zone&) const;

    QPixmap image_;
    QList<Zone> zones_;
    Commands commands_;
    Assign assign_;
    Platform::GamepadSource* source_ = nullptr;
    MachineInputBindings* bindings_ = nullptr;
    QTimer timer_;
    QSet<int> pressedKeys_;
    QHash<Emulation::ControllerVisualControl, float> liveValues_;
    const Zone* hovered_ = nullptr;
};
}
