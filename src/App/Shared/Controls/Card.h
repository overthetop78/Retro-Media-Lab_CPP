#pragma once
#include "AppApi.h"
#include <QWidget>
#include <memory>
namespace RetroMediaLab {
struct ThemeData;
class Card final : public QWidget {
public:
    RETROMEDIALAB_APP_API Card(std::shared_ptr<const ThemeData> theme, QWidget* parent = nullptr);
protected:
    void paintEvent(QPaintEvent*) override;
private:
    std::shared_ptr<const ThemeData> theme_;
};
}
