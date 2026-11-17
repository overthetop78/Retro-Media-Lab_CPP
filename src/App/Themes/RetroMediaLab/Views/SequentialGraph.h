#pragma once
#include <RetroMediaLab/MediaEngine/SequentialVisualization.h>
#include <QRectF>
#include <QSize>
class QPainter;
namespace RetroMediaLab {
struct ThemeData;
struct SequentialGraphPart {std::size_t segment=0;QRectF rect;};
std::vector<SequentialGraphPart> sequentialGraphParts(QSize,const MediaEngine::SequentialVisualizationModel&);
void paintSequentialGraph(QPainter&,QSize,const MediaEngine::SequentialVisualizationModel&,std::optional<std::size_t>,const ThemeData&);
std::optional<std::size_t> hitTestSequentialGraph(QSize,const MediaEngine::SequentialVisualizationModel&,QPointF);
}
