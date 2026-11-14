#pragma once
#include <QColor>
#include <QIcon>
namespace RetroMediaLab {
// Portable monochrome symbols; no operating-system icon set or glyph font.
QIcon themeIcon(const QString& id, const QColor& color);
}
