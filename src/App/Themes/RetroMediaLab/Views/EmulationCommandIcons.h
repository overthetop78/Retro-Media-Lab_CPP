#pragma once

#include <QColor>
#include <QFont>
#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QRawFont>
#include <QStyle>
#include <QWidget>

namespace RetroMediaLab::EmulationCommandIcons {

struct Glyph {
    char16_t code;
    QStyle::StandardPixmap fallback;
};

inline constexpr QSize ButtonSize(28, 28);
inline constexpr QSize IconSize(20, 20);
inline constexpr int FontSize = 17;
inline constexpr auto FontFamily = "Segoe MDL2 Assets";
inline constexpr Glyph Power{u'\uE7E8', QStyle::SP_MediaStop};
inline constexpr Glyph Pause{u'\uE769', QStyle::SP_MediaPause};
inline constexpr Glyph Resume{u'\uE768', QStyle::SP_MediaPlay};
inline constexpr Glyph SoftReset{u'\uE777', QStyle::SP_ArrowBack};
inline constexpr Glyph HardReset{u'\uE72C', QStyle::SP_BrowserReload};
inline constexpr Glyph Save{u'\uE74E', QStyle::SP_DialogSaveButton};
inline constexpr Glyph Load{u'\uE8E5', QStyle::SP_DialogOpenButton};
inline constexpr Glyph Capture{u'\uE722', QStyle::SP_DialogSaveButton};
inline constexpr Glyph Fullscreen{u'\uE740', QStyle::SP_TitleBarMaxButton};
inline constexpr Glyph Settings{u'\uE713', QStyle::SP_FileDialogDetailedView};
inline constexpr Glyph Audio{u'\uE767', QStyle::SP_MediaVolume};
inline constexpr Glyph Muted{u'\uE74F', QStyle::SP_MediaVolumeMuted};
inline constexpr Glyph Controller{u'\uE7FC', QStyle::SP_ComputerIcon};
inline constexpr Glyph Pointer{u'\uE962', QStyle::SP_ArrowUp};
inline constexpr QRgb Powered = qRgb(50, 205, 50);
inline constexpr QRgb Unpowered = qRgb(128, 128, 128);
inline constexpr QRgb Inactive = qRgb(211, 211, 211);
inline constexpr QRgb Audible = qRgb(0, 0, 0);
inline constexpr QRgb PauseColor = qRgb(255, 140, 0);
inline constexpr QRgb SoftResetColor = qRgb(120, 160, 48);
inline constexpr QRgb HardResetColor = qRgb(220, 92, 48);

inline QIcon icon(const QWidget& widget, Glyph glyph, QRgb color) {
    QFont font{QString::fromLatin1(FontFamily)};
    font.setPixelSize(FontSize);
    font.setStyleStrategy(QFont::NoFontMerging);
    const auto raw = QRawFont::fromFont(font);
    const auto ratio = widget.devicePixelRatioF();
    if (!raw.supportsCharacter(glyph.code)) {
        auto fallback = widget.style()->standardIcon(glyph.fallback).pixmap(IconSize, ratio);
        if (fallback.isNull()) return {};
        QPainter painter(&fallback);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(QRectF(QPointF(), fallback.deviceIndependentSize()), QColor::fromRgb(color));
        painter.end();
        return QIcon(fallback);
    }

    QPixmap pixmap(IconSize * ratio);
    pixmap.setDevicePixelRatio(ratio);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setFont(font);
    painter.setPen(QColor::fromRgb(color));
    painter.drawText(QRect(QPoint(), IconSize), Qt::AlignCenter, QString(QChar(glyph.code)));
    painter.end();
    return QIcon(pixmap);
}

}
