#pragma once
#include <QPixmap>
#include "Media/VisualizationSession.h"
#include "Shared/Presentation/ThemeService.h"
#include <QWidget>
#include <QPointF>
#include <map>
#include <tuple>
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
    QPixmap presentedDisk_;
    int renderedTracks_ = 0;
    void clearPresentedDisk() { presentedDisk_ = {}; renderedTracks_ = 0; }
    const MediaEngine::DiskFormatInfo* format() const;
    void rebuild();
    std::vector<std::int32_t> diskSurfaces()const;
    QPointF diskPoint(QPointF,std::size_t,std::size_t)const;
    void focusSurface(QPointF);
    ThemeService& themes_;
    std::shared_ptr<Infrastructure::VisualizationSession> session_;
    std::shared_ptr<const Infrastructure::VisualizationSnapshot> cached_;
    std::map<std::pair<std::int64_t,std::int64_t>,std::vector<const MediaEngine::SectorVisualizationItem*>> sectorTracks_;
    std::vector<std::int32_t> surfaces_;
    std::int32_t cylinders_ = 0;
    std::optional<std::string> cachedFormat_;
    using FluxRenderKey = std::tuple<const MediaEngine::FluxVisualizationModel*, std::size_t, bool,
        MediaEngine::DiskMediaCategory, std::vector<std::tuple<double,double,double>>>;
    std::optional<FluxRenderKey> fluxKey_;
    QPointF previousMouse_;
    Infrastructure::ExplorerSession::Subscription subscription_;
    ThemeService::Subscription themeSubscription_;
};
}
