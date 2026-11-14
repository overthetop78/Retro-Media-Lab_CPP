#pragma once
#include "../ThemeService.h"
#include <QProxyStyle>
namespace RetroMediaLab {
class RetroMediaLabStyle final : public QProxyStyle {
public:
    explicit RetroMediaLabStyle(std::shared_ptr<const ThemeData> data);
    void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& content, const QWidget* widget) const override;
private:
    std::shared_ptr<const ThemeData> data_;
};
}
