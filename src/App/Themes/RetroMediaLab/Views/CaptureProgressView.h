#pragma once
#include "Shared/Presentation/ThemeService.h"
#include "Media/GwProgressTracker.h"
#include <QWidget>
#include <RetroMediaLab/MediaEngine/SequentialVisualization.h>
namespace RetroMediaLab {
class CaptureProgressView final:public QWidget {
public:
    CaptureProgressView(ThemeService&,QWidget*,bool compact=false);
    void setSequential(const MediaEngine::SequentialVisualizationModel&,std::size_t presented,const QString& label);
    void setLinear(const std::vector<Infrastructure::GwTrackState>&, const QString& label);
    void setProgress(const Infrastructure::GwTrackProgress&,const QStringList& faceLabels,bool running);
protected:
    void paintEvent(QPaintEvent*)override;
private:
    ThemeService& themes_;
    ThemeService::Subscription themeSubscription_;
    Infrastructure::GwTrackProgress progress_;
    QStringList labels_;
    std::vector<MediaEngine::SequentialVisualizationKind> segmentKinds_;
    std::vector<Infrastructure::GwTrackState> linearStates_;
    std::size_t linearCompleted_=0;
    std::size_t presented_=0;
    bool running_=false,compact_=false;
};
}
