#include "CaptureProgressView.h"
#include "CaptureConstants.h"
#include "../ThemeConstants.h"
#include <QPainter>
#include <algorithm>
namespace RetroMediaLab {
namespace C=CaptureConstants;
using Infrastructure::GwTrackState;
CaptureProgressView::CaptureProgressView(ThemeService& themes,QWidget* parent):QWidget(parent),themes_(themes){setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);themeSubscription_=themes.subscribe([this]{update();});}
void CaptureProgressView::setProgress(const Infrastructure::GwTrackProgress& progress,const QStringList& labels,bool running){progress_=progress;labels_.clear();progress_.heads.clear();for(std::size_t index=0;index<progress.heads.size();++index)if(progress.heads[index]==0 || progress.heads[index]==1){progress_.heads.push_back(progress.heads[index]);labels_.push_back(labels.value(static_cast<int>(index)));}running_=running;setVisible(!progress_.cylinders.empty() && !progress_.heads.empty());setFixedHeight(static_cast<int>(progress_.heads.size())*C::ProgressRowHeight);update();}
void CaptureProgressView::paintEvent(QPaintEvent*){
    if(progress_.cylinders.empty())return;QPainter painter(this);const auto theme=themes_.data();const auto area=std::max(C::ProgressMinimumWidth,static_cast<double>(width()-C::ProgressLabelWidth-C::ProgressCountWidth));const auto cell=area/progress_.cylinders.size();
    bool retry=false;if(progress_.cylinder && progress_.head){const auto found=progress_.states.find({*progress_.cylinder,*progress_.head});retry=found!=progress_.states.end() && found->second==GwTrackState::Retry;}
    for(std::size_t row=0;row<progress_.heads.size();++row){const auto head=progress_.heads[row];const auto y=static_cast<int>(row)*C::ProgressRowHeight;painter.setPen(theme->color(ThemeConstants::TextBrush));painter.drawText(QRect(0,y,C::ProgressLabelWidth,C::ProgressRowHeight),Qt::AlignVCenter|Qt::AlignLeft,labels_.value(static_cast<int>(row)));const auto count=progress_.completedPerHead.find(head);const auto completed=count==progress_.completedPerHead.end()?0:count->second;
        for(std::size_t index=0;index<progress_.cylinders.size();++index){const auto cylinder=progress_.cylinders[index];const auto found=progress_.states.find({cylinder,head});const auto state=found==progress_.states.end()?GwTrackState::Pending:found->second;const char* role=ThemeConstants::ControlBrush;switch(state){case GwTrackState::Success:role=C::ProgressSuccessBrush;break;case GwTrackState::Retry:role=C::ProgressRetryBrush;break;case GwTrackState::Failed:role=C::ProgressFailedBrush;break;default:break;}
            if(running_ && !retry && progress_.nextCylinder==cylinder && progress_.nextHead==head)role=ThemeConstants::AccentBrush;const QRectF rect(C::ProgressLabelWidth+index*cell,y+C::Gap,std::max(C::ProgressMinimumWidth,cell-C::ProgressCellGap),C::ProgressRowHeight-C::Gap-C::Gap);painter.fillRect(rect,theme->color(role));}
        painter.drawText(QRect(width()-C::ProgressCountWidth,y,C::ProgressCountWidth,C::ProgressRowHeight),Qt::AlignVCenter|Qt::AlignRight,QString::number(completed)+QString::fromLatin1(C::ProgressDivider)+QString::number(progress_.cylinders.size()));
    }
}
}
