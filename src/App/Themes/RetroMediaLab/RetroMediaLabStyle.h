#pragma once
#include "Shared/Presentation/ThemeService.h"
#include <QProxyStyle>
namespace RetroMediaLab {
class RetroMediaLabStyle final : public QProxyStyle {
public:
    explicit RetroMediaLabStyle(std::shared_ptr<const ThemeData> data);
    void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& content, const QWidget* widget) const override;
    void drawControl(ControlElement, const QStyleOption*, QPainter*, const QWidget* = nullptr) const override;
    int pixelMetric(PixelMetric, const QStyleOption* = nullptr, const QWidget* = nullptr) const override;
    int styleHint(StyleHint, const QStyleOption* = nullptr, const QWidget* = nullptr, QStyleHintReturn* = nullptr) const override;
    void polish(QWidget*) override;
private:
    std::shared_ptr<const ThemeData> data_;
};
}
