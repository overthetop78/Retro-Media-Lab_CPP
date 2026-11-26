#include "LinearGraph.h"
#include "VisualizationConstants.h"
#include "Shared/Presentation/ThemeService.h"
#include <QPainter>
#include <cmath>
#include <algorithm>
namespace RetroMediaLab {
namespace {
using namespace VisualizationConstants;
struct DiskLayout {int columns,rows;double width,height,radius,inner;};
DiskLayout diskLayout(QSize size,const MediaEngine::LinearVisualizationModel& model,bool optical){
    const auto heads=model.geometry?model.geometry->heads():1;const auto columns=std::max(1,static_cast<int>(std::ceil(std::sqrt(heads))));const auto rows=(heads+columns-1)/columns;
    const auto width=static_cast<double>(size.width())/columns,height=static_cast<double>(size.height())/rows;const auto radius=std::max(MinViewSize,std::min(width,height)*Half-LinearOuterMargin);
    return {columns,rows,width,height,radius,radius*(optical?OpticalHoleRatio:BlockHoleRatio)};
}
QPointF centerFor(const DiskLayout& layout,int head){return {(head%layout.columns+Half)*layout.width,(head/layout.columns+Half)*layout.height};}
QRectF circle(QPointF center,double radius){return {center.x()-radius,center.y()-radius,radius/Half,radius/Half};}
QRectF cartridgeBody(QSize size){const auto width=std::min(size.width()*CartridgeWidthRatio,size.height()*CartridgeHeightRatio*CartridgeAspect),height=width/CartridgeAspect;return {(size.width()-width)*Half,(size.height()-height)*Half,width,height};}
QRectF cartridgePanel(QRectF body){return body.adjusted(body.width()*PanelLeft,body.height()*PanelTop,-body.width()*PanelLeft,-body.height()*PanelBottom);}
QRectF cartridgeCell(QRectF body,std::size_t count,std::size_t index){auto panel=cartridgePanel(body);panel.adjust(body.width()*ContentHorizontal,body.height()*ContentVertical,-body.width()*ContentHorizontal,-body.height()*ContentVertical);const auto columns=std::min<std::size_t>(CartridgeColumns,std::max<std::size_t>(1,count)),rows=(count+columns-1)/columns;const auto width=panel.width()/columns,height=panel.height()/rows,gap=std::min(CartridgeGap,std::min(width,height)*ConnectorLeft);return {panel.left()+(index%columns)*width+gap,panel.top()+(index/columns)*height+gap,width-gap-gap,height-gap-gap};}
}
void paintLinearGraph(QPainter& painter,QSize size,const MediaEngine::LinearVisualizationModel& model,const MediaEngine::VisualizationDescriptor& descriptor,std::optional<std::size_t> selected,const ThemeData& theme,std::span<const Infrastructure::GwTrackState> progress){
    using namespace VisualizationConstants;
    const auto revealed = [&](std::size_t index) {
        return index >= progress.size() || progress[index] == Infrastructure::GwTrackState::Success;
    };const auto& elements=descriptor.fields().elements;const bool optical=descriptor.fields().representationKind==MediaEngine::RepresentationKind::OpticalTracks;
    const auto selection=theme.color(VisualSelectionBrush);
    if(model.cartridge){const auto body=cartridgeBody(size);painter.setPen(QPen(theme.color(CartridgeBorderBrush),LinearSelectionWidth));painter.setBrush(theme.color(CartridgeBodyBrush));painter.drawRoundedRect(body,CartridgeCorner,CartridgeCorner);painter.setPen(Qt::NoPen);painter.setBrush(theme.color(CartridgePanelBrush));painter.drawRoundedRect(cartridgePanel(body),CartridgeCellRadius,CartridgeCellRadius);
        const auto connector=body.adjusted(body.width()*ConnectorLeft,body.height()*(1-ConnectorTop),-body.width()*ConnectorLeft,-body.height()*ConnectorBottom);painter.drawRoundedRect(connector,CartridgeCellRadius,CartridgeCellRadius);const auto contactWidth=connector.width()/CartridgeContacts;for(int contact=0;contact<CartridgeContacts;++contact)painter.fillRect(QRectF(connector.left()+contact*contactWidth+contactWidth*ContactInset,connector.top()+CartridgeCellRadius,contactWidth*(1-ContactInset-ContactInset),std::max(MinViewSize,connector.height()-CartridgeCellRadius-CartridgeCellRadius)),theme.color(CartridgeContactBrush));
        for(std::size_t index=0;index<model.items.size();++index){painter.setPen(selected==index?QPen(selection,LinearSelectionWidth):QPen(Qt::NoPen));painter.setBrush(theme.color(BlockBrushes[revealed(index) ? static_cast<std::size_t>(model.items[index].blockState) : 0]));painter.drawRoundedRect(cartridgeCell(body,model.items.size(),index),CartridgeCellRadius,CartridgeCellRadius);}return;
    }
    const auto layout=diskLayout(size,model,optical);const auto heads=model.geometry?model.geometry->heads():1;const auto cylinders=model.geometry?model.geometry->cylinders():1;const auto thickness=(layout.radius-layout.inner)/cylinders;
    for(int head=0;head<heads;++head){painter.setPen(QPen(theme.color(BlockBrushes[0]),layout.radius-layout.inner));painter.setBrush(Qt::NoBrush);painter.drawEllipse(circle(centerFor(layout,head),(layout.radius+layout.inner)*Half));}
    for(std::size_t index=0;index<model.items.size();++index){const auto& fields=elements[index].fields();const auto trackBlocks=model.geometry?model.geometry->sectorsPerTrack():std::max<std::int64_t>(1,model.logicalLength);const auto track=fields.position/trackBlocks;const auto cylinder=model.geometry?track/heads:0;const auto head=model.geometry?static_cast<int>(track%heads):0;if(cylinder>=cylinders)continue;
        const auto state=optical?static_cast<std::size_t>(model.items[index].opticalKind):static_cast<std::size_t>(model.items[index].blockState);const auto color=theme.color(optical?OpticalBrushes[revealed(index) ? state : 0]:BlockBrushes[revealed(index) ? state : 0]);
        const auto start=QuarterCircle-FullCircle*static_cast<double>(fields.position%trackBlocks)/trackBlocks;const auto sweep=std::max(MinimumArcDegrees,FullCircle*static_cast<double>(fields.length.value_or(1))/trackBlocks-(optical?TrackGapDegrees:MinimumOffset));const auto center=centerFor(layout,head);const auto radial=layout.radius-(cylinder+Half)*thickness;
        painter.setPen(QPen(color,thickness,Qt::SolidLine,Qt::FlatCap));painter.drawArc(circle(center,radial),static_cast<int>(start*AngularUnits),-static_cast<int>(sweep*AngularUnits));
        if(selected==index){painter.setPen(QPen(selection,LinearSelectionWidth,Qt::SolidLine,Qt::FlatCap));painter.drawArc(circle(center,layout.radius-cylinder*thickness),static_cast<int>(start*AngularUnits),-static_cast<int>(sweep*AngularUnits));}
    }
}
std::optional<std::size_t> hitTestLinearGraph(QSize size,const MediaEngine::LinearVisualizationModel& model,const MediaEngine::VisualizationDescriptor& descriptor,QPointF point){
    using namespace VisualizationConstants;const auto& elements=descriptor.fields().elements;if(model.cartridge){const auto body=cartridgeBody(size);for(std::size_t index=0;index<model.items.size();++index)if(cartridgeCell(body,model.items.size(),index).contains(point))return index;return {};}
    const bool optical=descriptor.fields().representationKind==MediaEngine::RepresentationKind::OpticalTracks;const auto layout=diskLayout(size,model,optical);if(point.x()<0 || point.y()<0 || point.x()>=size.width() || point.y()>=size.height())return {};
    const auto column=static_cast<int>(point.x()/layout.width),row=static_cast<int>(point.y()/layout.height),head=row*layout.columns+column;const auto heads=model.geometry?model.geometry->heads():1;if(head>=heads)return {};const auto center=centerFor(layout,head);const auto distance=std::hypot(point.x()-center.x(),point.y()-center.y());if(distance<layout.inner || distance>layout.radius)return {};
    const auto cylinders=model.geometry?model.geometry->cylinders():1;const auto cylinder=std::min<std::int64_t>(cylinders-1,static_cast<std::int64_t>((layout.radius-distance)/(layout.radius-layout.inner)*cylinders));auto angle=std::atan2(point.y()-center.y(),point.x()-center.x())+FullTurn/FullCircle*QuarterCircle;if(angle<0)angle+=FullTurn;if(angle>=FullTurn)angle-=FullTurn;const auto trackBlocks=model.geometry?model.geometry->sectorsPerTrack():std::max<std::int64_t>(1,model.logicalLength);const auto position=(cylinder*heads+head)*trackBlocks+std::min<std::int64_t>(trackBlocks-1,static_cast<std::int64_t>(angle/FullTurn*trackBlocks));
    for(std::size_t index=0;index<elements.size();++index){const auto& fields=elements[index].fields();if(position>=fields.position && position-fields.position<fields.length.value_or(1))return index;}return {};
}
}
