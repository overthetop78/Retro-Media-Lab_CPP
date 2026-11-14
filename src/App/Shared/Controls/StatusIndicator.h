#pragma once
#include "AppApi.h"
#include <QWidget>
class QLabel;
namespace RetroMediaLab {
class StatusIndicator final : public QWidget {
public:
    RETROMEDIALAB_APP_API explicit StatusIndicator(QWidget*);
    RETROMEDIALAB_APP_API void setStatus(const QString&, const QColor&);
protected:
    void paintEvent(QPaintEvent*) override;
private:
    QLabel* label_;
    QColor color_;
};
}
