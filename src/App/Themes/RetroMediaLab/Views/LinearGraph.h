#pragma once
#include <RetroMediaLab/MediaEngine/LinearVisualization.h>
#include <QPointF>
#include <QSize>
class QPainter;
namespace RetroMediaLab {
struct ThemeData;
void paintLinearGraph(QPainter&,QSize,const MediaEngine::LinearVisualizationModel&,const MediaEngine::VisualizationDescriptor&,std::optional<std::size_t>,const ThemeData&);
std::optional<std::size_t> hitTestLinearGraph(QSize,const MediaEngine::LinearVisualizationModel&,const MediaEngine::VisualizationDescriptor&,QPointF);
}
