#include "FluxGraph.h"
#include "VisualizationConstants.h"
#include "Shared/Presentation/ThemeService.h"
#include <QPainter>
#include <algorithm>
#include <cmath>
namespace RetroMediaLab {
namespace {
using namespace VisualizationConstants;
struct Face {std::int32_t head;std::vector<const MediaEngine::FluxVisualTrack*> tracks;};
std::vector<Face> faces(const MediaEngine::FluxVisualizationModel& model){std::vector<Face> result;for(const auto& track:model.tracks){auto found=std::find_if(result.begin(),result.end(),[&](const auto& face){return face.head==track.head;});if(found==result.end()){result.push_back({track.head,{&track}});}else found->tracks.push_back(&track);}std::sort(result.begin(),result.end(),[](const auto& a,const auto& b){return a.head<b.head;});for(auto& face:result)std::sort(face.tracks.begin(),face.tracks.end(),[](const auto* a,const auto* b){return a->cylinder<b->cylinder;});return result;}
QColor qualityColor(double quality,const ThemeData& theme){std::size_t index=0;while(index<FluxQualityLimits.size() && quality>FluxQualityLimits[index])++index;return theme.color(FluxQualityBrushes[index]);}
}
void paintFluxGraph(QPainter& painter,QSize size,const MediaEngine::FluxVisualizationModel& model,std::size_t revolution,bool synthesis,std::int32_t head,std::int64_t cylinder,const MediaEngine::FluxTrackInspection* inspection,const ThemeData& theme,std::optional<std::int32_t> onlyHead,MediaEngine::DiskMediaCategory category,std::size_t firstTrack,bool pendingOnly,bool selectionOnly,bool sectorSelected){
    const auto groups=faces(model);if(groups.empty())return;const auto width=static_cast<double>(size.width())/groups.size();const auto outer=std::min(width,static_cast<double>(size.height()))*(category==MediaEngine::DiskMediaCategory::Unknown?FluxRadius:FluxKnownRadius),inner=outer*FluxHole;
    for(std::size_t faceIndex=0;faceIndex<groups.size();++faceIndex){const auto& face=groups[faceIndex];if(onlyHead&&face.head!=*onlyHead)continue;const QPointF center(width*(faceIndex+Half),size.height()*Half);painter.setPen(Qt::NoPen);const auto ring=(outer-inner)/face.tracks.size();
        for(std::size_t index=0;index<face.tracks.size();++index){const auto& track=*face.tracks[index];if(track.revolutions.empty()!=pendingOnly || static_cast<std::size_t>(&track-model.tracks.data())<firstTrack)continue;const auto radius=outer-ring*(index+Half);const QRectF rect(center.x()-radius,center.y()-radius,radius+radius,radius+radius);if (selectionOnly) {
                if(track.head!=head || track.cylinder!=cylinder) continue;
                painter.setBrush(Qt::NoBrush);
                painter.setPen(QPen(theme.color(sectorSelected?WarningBrush:VisualSelectionBrush),sectorSelected?SectorSelectionWidth:SelectionWidth));
                painter.drawEllipse(rect);
                if(inspection) for(const auto& arc:inspection->structures){
                    painter.setPen(QPen(theme.color(FluxBrushes[static_cast<std::size_t>(arc.mark)]),std::max(MinViewSize,ring*FluxStroke),Qt::SolidLine,Qt::RoundCap));
                    painter.drawArc(rect,static_cast<int>(-arc.start*FluxDegreesPerUnit),static_cast<int>(-arc.sweep*FluxDegreesPerUnit));
                }
                continue;
            }
            if (pendingOnly) {
                painter.setBrush(Qt::NoBrush);
                painter.setPen(QPen(QColor::fromRgb(PendingFluxRgb),std::max(MinViewSize,ring*FluxBand)));
                painter.drawEllipse(rect);
                continue;
            }
            if(!synthesis && revolution>=track.revolutions.size())continue;const auto& value=synthesis?track.synthesis:track.revolutions[revolution];painter.setBrush(Qt::NoBrush);painter.setPen(QPen(qualityColor(value.quality,theme),std::max(MinViewSize,ring*FluxBand)));painter.drawEllipse(rect);
            painter.setRenderHint(QPainter::Antialiasing,false);for(const auto& arc:value.arcs){QPen pen(theme.color(FluxBrushes[static_cast<std::size_t>(arc.mark)]),std::max(MinViewSize,ring*FluxStroke),Qt::SolidLine,Qt::FlatCap);painter.setPen(pen);painter.drawArc(rect,static_cast<int>(-arc.start*FluxDegreesPerUnit),static_cast<int>(-arc.sweep*FluxDegreesPerUnit));}painter.setRenderHint(QPainter::Antialiasing,true);
        }
    }
}
std::optional<std::pair<std::int32_t,std::int32_t>> hitTestFluxGraph(QSize size,const MediaEngine::FluxVisualizationModel& model,QPointF point,MediaEngine::DiskMediaCategory category){const auto groups=faces(model);if(groups.empty() || point.x()<0 || point.y()<0 || point.x()>=size.width() || point.y()>=size.height())return {};const auto width=static_cast<double>(size.width())/groups.size();const auto faceIndex=std::min(groups.size()-1,static_cast<std::size_t>(point.x()/width));const auto& face=groups[faceIndex];const auto outer=std::min(width,static_cast<double>(size.height()))*(category==MediaEngine::DiskMediaCategory::Unknown?FluxRadius:FluxKnownRadius),inner=outer*FluxHole;const auto distance=std::hypot(point.x()-width*(faceIndex+Half),point.y()-size.height()*Half);if(distance<inner || distance>outer)return {};const auto ring=(outer-inner)/face.tracks.size();const auto index=std::min(face.tracks.size()-1,static_cast<std::size_t>((outer-distance)/ring));return std::pair{face.head,face.tracks[index]->cylinder};}
}
