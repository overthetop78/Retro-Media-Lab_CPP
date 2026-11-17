#pragma once
#include "Media/VisualizationSession.h"
#include "Themes/ThemeService.h"
#include <QWidget>
#include <QPointF>
namespace RetroMediaLab {
class MediaGraph final : public QWidget {
public:
    MediaGraph(ThemeService&, std::shared_ptr<Infrastructure::VisualizationSession>, QWidget*);
protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;
private:
    const MediaEngine::DiskFormatInfo* format() const;
    void rebuild();
    ThemeService& themes_;
    std::shared_ptr<Infrastructure::VisualizationSession> session_;
    std::shared_ptr<const Infrastructure::VisualizationSnapshot> cached_;
    struct FluxRing {std::int32_t head, cylinder; std::vector<double> values;};
    std::vector<FluxRing> flux_;
    std::vector<std::int32_t> surfaces_;
    std::int32_t cylinders_ = 0;
    std::size_t cachedRevolution_ = 0;
    QPointF previousMouse_;
    Infrastructure::ExplorerSession::Subscription subscription_;
    ThemeService::Subscription themeSubscription_;
};
}
