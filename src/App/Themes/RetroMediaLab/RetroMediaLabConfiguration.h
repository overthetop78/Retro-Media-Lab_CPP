#pragma once
#include "Themes/ThemeService.h"
#include <QObject>
namespace RetroMediaLab {
class RetroMediaLabConfiguration final : public QObject {
public:
    explicit RetroMediaLabConfiguration(ThemeService&);
    void apply(const QJsonObject&);
protected:
    bool eventFilter(QObject*, QEvent*) override;
private:
    ThemeData paletteData(bool dark) const;
    void refreshSystem();
    void scheduleSystemRefresh();
    ThemeService& service_;
    ThemeData metrics_;
    QJsonObject settings_;
    bool applying_ = false;
    bool scheduled_ = false;
};
}
