#pragma once
#include <RetroMediaLab/MediaEngine/FluxVisualization.h>
#include <RetroMediaLab/MediaEngine/DiskClassification.h>
#include <QSize>
#include <QPointF>
class QPainter;
namespace RetroMediaLab {
struct ThemeData;
void paintFluxGraph(QPainter&,QSize,const MediaEngine::FluxVisualizationModel&,std::size_t revolution,bool synthesis,std::int32_t head,std::int64_t cylinder,const MediaEngine::FluxTrackInspection*,const ThemeData&,std::optional<std::int32_t> onlyHead = {},MediaEngine::DiskMediaCategory category=MediaEngine::DiskMediaCategory::Unknown);
std::optional<std::pair<std::int32_t,std::int32_t>> hitTestFluxGraph(QSize,const MediaEngine::FluxVisualizationModel&,QPointF,MediaEngine::DiskMediaCategory category=MediaEngine::DiskMediaCategory::Unknown);
}
