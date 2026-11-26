#pragma once
#include "AppApi.h"
#include <QScrollArea>
namespace RetroMediaLab {
// Recompute the viewport height after font, density and layout changes.
class RETROMEDIALAB_APP_API ContentScrollArea final : public QScrollArea {
public:
    explicit ContentScrollArea(int maximumContentHeight, QWidget* parent = nullptr);
    void setContent(QWidget* content);
protected:
    bool eventFilter(QObject*, QEvent*) override;
    void resizeEvent(QResizeEvent*) override;
private:
    void scheduleHeight();
    int limit_;
    bool pending_ = false;
};
}
