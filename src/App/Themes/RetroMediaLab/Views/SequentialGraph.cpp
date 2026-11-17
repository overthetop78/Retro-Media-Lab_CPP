#include "SequentialGraph.h"
#include "VisualizationConstants.h"
#include "Themes/ThemeService.h"
#include <QPainter>
#include <algorithm>
#include <cmath>
namespace RetroMediaLab {
namespace {
using namespace VisualizationConstants;
struct Layout {double width,rowHeight,bandHeight,lineLength;};
Layout layoutFor(QSize size,const MediaEngine::SequentialVisualizationModel& model){const auto rowHeight=std::max(MinViewSize,(size.height()-SequentialMargin-SequentialMargin)/(SequentialLines*static_cast<double>(std::max<std::size_t>(1,model.lanes.size()))));return {std::max(MinViewSize,size.width()-SequentialMargin-SequentialMargin),rowHeight,std::max(MinViewSize,std::min(SequentialBandMaximum,rowHeight-std::min(SequentialLaneGap,rowHeight*SequentialGapRatio))),model.totalLength/SequentialLines};}
}
std::vector<SequentialGraphPart> sequentialGraphParts(QSize size,const MediaEngine::SequentialVisualizationModel& model){
    std::vector<SequentialGraphPart> parts;if(model.totalLength<=0 || model.lanes.empty())return parts;const auto layout=layoutFor(size,model);
    for(std::size_t index=0;index<model.segments.size();++index){const auto& segment=model.segments[index];const auto lane=std::lower_bound(model.lanes.begin(),model.lanes.end(),segment.lane)-model.lanes.begin();double start=segment.start,remaining=segment.length;
        while(remaining>0){const auto line=std::clamp(static_cast<int>(start/layout.lineLength),0,SequentialLines-1);const auto length=std::min(remaining,(line+1)*layout.lineLength-start);if(length<=0)break;const auto x=SequentialMargin+(start-line*layout.lineLength)/layout.lineLength*layout.width;const auto center=SequentialMargin+(line*model.lanes.size()+lane+Half)*layout.rowHeight;parts.push_back({index,QRectF(x,center-layout.bandHeight*Half,std::max(SequentialMinimumPart,length/layout.lineLength*layout.width),layout.bandHeight)});const auto next=start+length;if(next<=start)break;start=next;remaining-=length;}
    }return parts;
}
void paintSequentialGraph(QPainter& painter,QSize size,const MediaEngine::SequentialVisualizationModel& model,std::optional<std::size_t> selected,const ThemeData& theme){
    if(model.totalLength<=0 || model.lanes.empty())return;const auto layout=layoutFor(size,model);painter.setPen(Qt::NoPen);painter.setBrush(theme.color(SequentialTape));
    for(int line=0;line<SequentialLines;++line)for(std::size_t lane=0;lane<model.lanes.size();++lane){const auto center=SequentialMargin+(line*model.lanes.size()+lane+Half)*layout.rowHeight;painter.drawRoundedRect(QRectF(SequentialMargin,center-layout.bandHeight*Half,layout.width,layout.bandHeight),layout.bandHeight*Half,layout.bandHeight*Half);}
    for(const auto& part:sequentialGraphParts(size,model)){auto color=theme.color(SequentialBrushes[static_cast<std::size_t>(model.segments[part.segment].kind)]);color.setAlpha(SequentialAlpha);painter.fillRect(part.rect,color);if(selected==part.segment){painter.setBrush(Qt::NoBrush);painter.setPen(QPen(theme.color(VisualSelectionBrush),SelectionWidth));painter.drawRoundedRect(part.rect,SelectionWidth,SelectionWidth);painter.setPen(Qt::NoPen);}}
}
std::optional<std::size_t> hitTestSequentialGraph(QSize size,const MediaEngine::SequentialVisualizationModel& model,QPointF point){const auto parts=sequentialGraphParts(size,model);for(auto it=parts.rbegin();it!=parts.rend();++it)if(it->rect.contains(point))return it->segment;return {};}
}
