#include "MediaGraph.h"
#include "LinearGraph.h"
#include "SequentialGraph.h"
#include "FluxGraph.h"
#include "VisualizationConstants.h"
#include "../ThemeConstants.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <set>
namespace RetroMediaLab {
using namespace VisualizationConstants;
using MediaEngine::RepresentationKind;
MediaGraph::MediaGraph(ThemeService& themes, std::shared_ptr<Infrastructure::VisualizationSession> session, QWidget* parent)
    : QWidget(parent), themes_(themes), session_(std::move(session)) {
    setMinimumSize(0, GraphMinimumHeight);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    subscription_ = session_->subscribe([this] { rebuild(); update(); });
    themeSubscription_ = themes.subscribe([this] { update(); });
    rebuild();
}
const MediaEngine::DiskFormatInfo* MediaGraph::format() const {
    if (!session_->snapshot() || !session_->snapshot()->source->diskExploration) return nullptr;
    const auto& info = session_->snapshot()->source->diskExploration->info();
    const auto found = std::find_if(info.formats.begin(), info.formats.end(), [&](const auto& value) { return value.formatId == info.primaryFormatId; });
    return found != info.formats.end() ? &*found : info.formats.empty() ? nullptr : &info.formats.front();
}
void MediaGraph::rebuild() {
    if (cached_ == session_->snapshot() && cachedRevolution_ == session_->revolution) return;
    cachedRevolution_ = session_->revolution;
    cached_ = session_->snapshot(); sectorTracks_.clear();surfaces_.clear(); cylinders_ = 0;
    if (!cached_) return;
    const auto kind = cached_->descriptor->fields().representationKind;
    if (kind != RepresentationKind::Flux && kind != RepresentationKind::Sectors) return;
    if(cached_->sectors){
        cylinders_=static_cast<std::int32_t>(cached_->sectors->cylinders);
        for(std::uint32_t head=0;head<cached_->sectors->heads;++head)surfaces_.push_back(static_cast<std::int32_t>(head));
        for(const auto& sector:cached_->sectors->sectors)sectorTracks_[{sector.head,sector.cylinder}].push_back(&sector);
        for(auto& [key,track]:sectorTracks_)std::stable_sort(track.begin(),track.end(),[](const auto* left,const auto* right){return left->number<right->number;});
        return;
    }

}
void MediaGraph::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const auto theme = themes_.data();
    painter.fillRect(rect(), theme->color(ThemeConstants::WindowBrush));
    if (!cached_) return;
    const auto foreground = theme->color(ThemeConstants::TextBrush);
    const auto success = theme->color(SuccessBrush);
    const auto unknown = theme->color(ThemeConstants::MutedTextBrush);
    const auto invalid = theme->color(ErrorBrush);
    const auto kind = cached_->descriptor->fields().representationKind;
    if(cached_->sectors)painter.fillRect(rect(),theme->color(SectorBackgroundBrush));
    if(cached_->sectors && cached_->sectors->logical){
        const auto& items=cached_->sectors->sectors;if(items.empty())return;
        painter.translate(width()*Half,height()*Half);painter.scale(session_->zoom,session_->zoom);painter.translate(-width()*Half+session_->panX*width(),-height()*Half+session_->panY*height());
        const auto columns=std::max<std::size_t>(1,static_cast<std::size_t>(std::sqrt(static_cast<double>(items.size())*std::max(1,width())/std::max(1,height()))));
        const auto rows=(items.size()+columns-1)/columns;const auto cellWidth=static_cast<double>(width())/columns,cellHeight=static_cast<double>(height())/rows;
        for(std::size_t index=0;index<items.size();++index){const QRectF cell((index%columns)*cellWidth,(index/columns)*cellHeight,std::max(MinViewSize,cellWidth-RingOverlap),std::max(MinViewSize,cellHeight-RingOverlap));painter.fillRect(cell,theme->color(SectorBrushes[static_cast<std::size_t>(items[index].state)]));if(session_->selectedSector==items[index].logicalBlock){painter.setPen(QPen(foreground,SelectionWidth));painter.drawRect(cell);}}
        return;
    }
    if(cached_->flux){painter.translate(width()*Half,height()*Half);painter.scale(session_->zoom,session_->zoom);painter.translate(-width()*Half+session_->panX*width(),-height()*Half+session_->panY*height());paintFluxGraph(painter,size(),*cached_->flux,session_->revolution,session_->fluxSynthesis,session_->surface,session_->position,session_->fluxInspection?&*session_->fluxInspection:nullptr,*theme);return;}
    const bool disk = !surfaces_.empty() && cylinders_ > 0 && (kind == RepresentationKind::Flux || kind == RepresentationKind::Sectors);
    if (disk) {
        painter.translate(width()*Half,height()*Half);
        painter.scale(session_->zoom,session_->zoom);
        painter.translate(-width()*Half+session_->panX*width(),-height()*Half+session_->panY*height());
        const double cellWidth = static_cast<double>(width()) / surfaces_.size();
        const double radius = kind==RepresentationKind::Sectors?std::max(MinViewSize,std::min(cellWidth,static_cast<double>(height()))*Half-SectorOuterMargin):std::min(cellWidth, static_cast<double>(height())) * OuterRadius;
        const double inner = radius * (kind==RepresentationKind::Sectors?SectorHoleRatio:HoleRadius);
        const double ringWidth = (radius - inner) / cylinders_;
        const auto* selectedFormat = format();
        for (std::size_t side = 0; side < surfaces_.size(); ++side) {
            const QPointF center(cellWidth * (static_cast<double>(side) + Half), height() * Half);
            const auto circle = [&](int cylinder) { const auto r = radius - (cylinder + Half) * ringWidth; return QRectF(center.x()-r, center.y()-r, r+r, r+r); };
            painter.setPen(QPen(unknown, LineWidth)); painter.setBrush(Qt::NoBrush); painter.drawEllipse(center, radius, radius);
            if (selectedFormat) {
                for(int cylinder=0;cylinder<cylinders_;++cylinder){
                    const auto found=sectorTracks_.find({surfaces_[side],cylinder});if(found==sectorTracks_.end())continue;
                    const auto& sectors=found->second;const auto sweep=static_cast<double>(FullCircle)/sectors.size();const auto outer=radius-cylinder*ringWidth,inside=std::max(inner,outer-ringWidth);const QRectF outerRect(center.x()-outer,center.y()-outer,outer+outer,outer+outer),innerRect(center.x()-inside,center.y()-inside,inside+inside,inside+inside);
                    painter.setPen(QPen(theme->color(SectorBoundaryBrush),LineWidth));painter.setBrush(Qt::NoBrush);painter.drawEllipse(outerRect);
                    for(std::size_t index=0;index<sectors.size();++index){const auto& sector=*sectors[index];const auto start=QuarterCircle-index*sweep-SectorGapDegrees*Half;const auto span=std::max(SectorMinimumSweep,sweep-SectorGapDegrees);QPainterPath path;path.arcMoveTo(outerRect,start);path.arcTo(outerRect,start,-span);const auto end=(start-span)/FullCircle*FullTurn;path.lineTo(center.x()+inside*std::cos(end),center.y()-inside*std::sin(end));path.arcTo(innerRect,start-span,span);path.closeSubpath();painter.setPen(Qt::NoPen);painter.setBrush(theme->color(SectorBrushes[static_cast<std::size_t>(sector.state)]));painter.drawPath(path);if(session_->selectedSector==sector.logicalBlock){painter.setPen(QPen(theme->color(VisualSelectionBrush),SectorSelectionWidth));painter.setBrush(Qt::NoBrush);painter.drawPath(path);}}

                }
            }
            if (kind==RepresentationKind::Flux && session_->surface == surfaces_[side] && session_->position < cylinders_) { painter.setPen(QPen(foreground, SelectionWidth)); painter.drawEllipse(circle(static_cast<int>(session_->position))); }
            if(kind==RepresentationKind::Flux){painter.setPen(foreground); painter.drawText(QRectF(cellWidth * side, 0, cellWidth, GraphLabelHeight), Qt::AlignCenter, QString::number(surfaces_[side]));}
        }
        return;
    }
    if(cached_->sequential && !session_->showWaveform){painter.fillRect(rect(),theme->color(SequentialBackground));painter.translate(width()*Half,height()*Half);painter.scale(session_->zoom,session_->zoom);painter.translate(-width()*Half+session_->panX*width(),-height()*Half+session_->panY*height());paintSequentialGraph(painter,size(),*cached_->sequential,session_->selectedElement,*theme);return;}
    if (cached_->tape && !cached_->tape->samples().empty()) {
        const auto samples = cached_->tape->samples();
        const auto visible = std::max<std::size_t>(1, static_cast<std::size_t>(samples.size() / session_->zoom));
        const auto offset = static_cast<std::size_t>(std::clamp(-session_->panX,MinimumOffset,1-1/session_->zoom)*samples.size());
        painter.setPen(QPen(success, LineWidth));
        for (int pixel = 0; pixel < width(); ++pixel) {
            const auto first = offset+static_cast<std::size_t>(static_cast<double>(pixel) / std::max(1,width()) * visible);
            const auto last = std::min(samples.size(), std::max(first+1, offset+static_cast<std::size_t>(static_cast<double>(pixel+1) / std::max(1,width()) * visible)));
            if (first >= last) continue;
            const auto extrema = std::minmax_element(samples.begin()+first, samples.begin()+last);
            painter.drawLine(QPointF(pixel, height()*Half - *extrema.first/MaximumSample*height()*AxisFraction), QPointF(pixel, height()*Half - *extrema.second/MaximumSample*height()*AxisFraction));
        }
        return;
    }
    if(cached_->linear){painter.fillRect(rect(),theme->color(VisualBackgroundBrush));painter.translate(width()*Half,height()*Half);painter.scale(session_->zoom,session_->zoom);painter.translate(-width()*Half+session_->panX*width(),-height()*Half+session_->panY*height());paintLinearGraph(painter,size(),*cached_->linear,*cached_->descriptor,session_->selectedElement,*theme);return;}
    const auto& elements = cached_->descriptor->fields().elements;
    if (elements.empty()) return;
    const auto maxPosition = std::max_element(elements.begin(), elements.end(), [](const auto& a,const auto& b) { return a.fields().position+a.fields().length.value_or(1) < b.fields().position+b.fields().length.value_or(1); });
    const auto length = std::max<std::int64_t>(1, maxPosition->fields().position+maxPosition->fields().length.value_or(1));
    for (std::size_t index = 0; index < elements.size(); ++index) {
        const auto& element = elements[index].fields();
        auto color = unknown;

        const auto x = (static_cast<double>(element.position)/length+session_->panX)*width()*session_->zoom;
        const auto extent = std::max(MinViewSize, static_cast<double>(element.length.value_or(1))/length*width()*session_->zoom);
        painter.fillRect(QRectF(x, height()*HoleRadius, extent, height()*(1-HoleRadius-HoleRadius)), color);
        painter.setPen(QPen(foreground, LineWidth)); painter.drawRect(QRectF(x, height()*HoleRadius, extent, height()*(1-HoleRadius-HoleRadius)));
    }
}
void MediaGraph::mousePressEvent(QMouseEvent* event) {
    previousMouse_=event->position();
    if (!cached_ || event->button() != Qt::LeftButton) return;
    const auto kind = cached_->descriptor->fields().representationKind;
    if(cached_->flux){const QPointF point((event->position().x()-width()*Half)/session_->zoom+width()*Half-session_->panX*width(),(event->position().y()-height()*Half)/session_->zoom+height()*Half-session_->panY*height());if(const auto selected=hitTestFluxGraph(size(),*cached_->flux,point))session_->select(selected->first,selected->second);return;}
    if(cached_->sequential && !session_->showWaveform){const QPointF point((event->position().x()-width()*Half)/session_->zoom+width()*Half-session_->panX*width(),(event->position().y()-height()*Half)/session_->zoom+height()*Half-session_->panY*height());if(const auto selected=hitTestSequentialGraph(size(),*cached_->sequential,point))session_->inspect(*selected);return;}
    if(cached_->sequential && session_->showWaveform)return;
    if(cached_->linear){const QPointF point((event->position().x()-width()*Half)/session_->zoom+width()*Half-session_->panX*width(),(event->position().y()-height()*Half)/session_->zoom+height()*Half-session_->panY*height());if(const auto selected=hitTestLinearGraph(size(),*cached_->linear,*cached_->descriptor,point))session_->inspect(*selected);return;}
    if(cached_->sectors && cached_->sectors->logical){
        const auto& items=cached_->sectors->sectors;if(items.empty())return;
        const QPointF point((event->position().x()-width()*Half)/session_->zoom+width()*Half-session_->panX*width(),(event->position().y()-height()*Half)/session_->zoom+height()*Half-session_->panY*height());
        if(point.x()<0 || point.y()<0 || point.x()>=width() || point.y()>=height())return;
        const auto columns=std::max<std::size_t>(1,static_cast<std::size_t>(std::sqrt(static_cast<double>(items.size())*std::max(1,width())/std::max(1,height()))));const auto rows=(items.size()+columns-1)/columns;
        const auto index=static_cast<std::size_t>(point.y()/height()*rows)*columns+static_cast<std::size_t>(point.x()/width()*columns);if(index<items.size())session_->selectSector(items[index].logicalBlock);return;
    }
    if (!surfaces_.empty() && cylinders_ > 0 && (kind == RepresentationKind::Flux || kind == RepresentationKind::Sectors)) {
        const double cellWidth = static_cast<double>(width()) / surfaces_.size();
        const QPointF point((event->position().x()-width()*Half)/session_->zoom+width()*Half-session_->panX*width(),(event->position().y()-height()*Half)/session_->zoom+height()*Half-session_->panY*height());
        const auto side = std::min(surfaces_.size()-1, static_cast<std::size_t>(std::max(MinimumOffset,point.x())/std::max(MinViewSize,cellWidth)));
        const QPointF center(cellWidth * (static_cast<double>(side)+Half), height()*Half);
        const auto radius = kind==RepresentationKind::Sectors?std::max(MinViewSize,std::min(cellWidth,static_cast<double>(height()))*Half-SectorOuterMargin):std::min(cellWidth,static_cast<double>(height()))*OuterRadius;
        const auto distance = std::hypot(point.x()-center.x(),point.y()-center.y());
        const auto hole=kind==RepresentationKind::Sectors?SectorHoleRatio:HoleRadius;
        if (distance > radius || distance < radius*hole) return;
        const auto cylinder = std::clamp(static_cast<int>((radius-distance)/(radius*(1-hole))*cylinders_),0,cylinders_-1);
        if(kind==RepresentationKind::Sectors){const auto found=sectorTracks_.find({surfaces_[side],cylinder});if(found!=sectorTracks_.end() && !found->second.empty()){auto angle=std::atan2(point.y()-center.y(),point.x()-center.x())+FullTurn/FullCircle*QuarterCircle;if(angle<0)angle+=FullTurn;if(angle>=FullTurn)angle-=FullTurn;const auto index=std::min(found->second.size()-1,static_cast<std::size_t>(angle/FullTurn*found->second.size()));session_->selectSector(found->second[index]->logicalBlock);return;}}
        session_->select(surfaces_[side],cylinder); return;
    }
    const auto& elements = cached_->descriptor->fields().elements;
    if (elements.empty()) return;
    const auto maxPosition = std::max_element(elements.begin(),elements.end(),[](const auto& a,const auto& b){return a.fields().position+a.fields().length.value_or(1)<b.fields().position+b.fields().length.value_or(1);});
    const auto length=std::max<std::int64_t>(1,maxPosition->fields().position+maxPosition->fields().length.value_or(1));
    const auto position=static_cast<std::int64_t>((event->position().x()/std::max(1,width())/session_->zoom-session_->panX)*length);
    for(std::size_t index=0;index<elements.size();++index)if(position>=elements[index].fields().position && position<elements[index].fields().position+elements[index].fields().length.value_or(1)) {
        session_->select(elements[index].fields().surface.value_or(0),elements[index].fields().position);
        if(kind==RepresentationKind::Blocks || kind==RepresentationKind::OpticalTracks)session_->inspect(index);
        break;
    }
}
void MediaGraph::mouseMoveEvent(QMouseEvent* event) {
    if(event->buttons() & Qt::RightButton) {
        const auto delta=event->position()-previousMouse_;
        session_->setPan(session_->panX+delta.x()/std::max(1,width())/session_->zoom,session_->panY+delta.y()/std::max(1,height())/session_->zoom);
    }
    previousMouse_=event->position();
}
void MediaGraph::wheelEvent(QWheelEvent* event) { session_->setZoom(std::clamp(session_->zoom*(event->angleDelta().y()>0?ZoomStep:1/ZoomStep),InitialZoom,MaximumZoom)); event->accept(); }
}
