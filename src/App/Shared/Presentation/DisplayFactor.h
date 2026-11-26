#pragma once

#include <Constants/DisplayConstants.h>
#include <QWidget>

namespace RetroMediaLab {
inline qreal displayFactor(const QWidget* widget) {
    if (!widget) return DisplayConstants::Unit;
    const auto value = widget->window()->property(DisplayConstants::FactorProperty);
    return value.isValid() ? value.toReal() : DisplayConstants::Unit;
}
}
