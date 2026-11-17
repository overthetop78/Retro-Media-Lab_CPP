#pragma once
#include "Themes/ThemeService.h"
#include "Media/GwProgressTracker.h"
#include <QWidget>
namespace RetroMediaLab {
class CaptureProgressView final:public QWidget {
public:
    CaptureProgressView(ThemeService&,QWidget*);
    void setProgress(const Infrastructure::GwTrackProgress&,const QStringList& faceLabels,bool running);
protected:
    void paintEvent(QPaintEvent*)override;
private:
    ThemeService& themes_;
    ThemeService::Subscription themeSubscription_;
    Infrastructure::GwTrackProgress progress_;
    QStringList labels_;
    bool running_=false;
};
}
