#pragma once
#include "State/WorkspaceState.h"
#include <QObject>
#include <memory>
class QWidget;
namespace RetroMediaLab {
class DisplayDensity final : public QObject {
public:
    DisplayDensity(QWidget&, WorkspaceState&);
    ~DisplayDensity() override;
    static qreal factorFor(const QSize& availableLogicalSize, int percent);
    static qreal factor(const QWidget*);
protected:
    bool eventFilter(QObject*, QEvent*) override;
private:
    void schedule();
    void refresh();
    struct Data;
    std::unique_ptr<Data> data_;
};
}
