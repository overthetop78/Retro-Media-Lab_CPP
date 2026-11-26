#include "CaptureProgressView.h"
#include "CaptureConstants.h"
#include "LoadingConstants.h"
#include "../DisplayDensity.h"
#include <Constants/DisplayConstants.h>
#include "../ThemeConstants.h"
#include <QPainter>
#include <algorithm>
namespace RetroMediaLab {
namespace C=CaptureConstants;
using Infrastructure::GwTrackState;
CaptureProgressView::CaptureProgressView(ThemeService& themes,QWidget* parent,bool compact):QWidget(parent),themes_(themes),compact_(compact){setProperty(DisplayConstants::ContentHeightProperty,true);if(compact_){auto small=font();small.setPixelSize(LoadingConstants::StripFont);setFont(small);}setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);themeSubscription_=themes.subscribe([this]{update();});}
void CaptureProgressView::setProgress(const Infrastructure::GwTrackProgress& progress,const QStringList& labels,bool running){linearStates_.clear();segmentKinds_.clear();progress_=progress;labels_.clear();progress_.heads.clear();for(std::size_t index=0;index<progress.heads.size();++index)if(progress.heads[index]==0 || progress.heads[index]==1){progress_.heads.push_back(progress.heads[index]);labels_.push_back(labels.value(static_cast<int>(index)));}running_=running;setVisible(!progress_.cylinders.empty() && !progress_.heads.empty());setFixedHeight(qRound(progress_.heads.size()*(compact_?LoadingConstants::StripRowHeight:C::ProgressRowHeight)*DisplayDensity::factor(this)));update();}
void CaptureProgressView::setSequential(const MediaEngine::SequentialVisualizationModel& model,std::size_t presented,const QString& label){linearStates_.clear();segmentKinds_.clear();for(const auto& segment:model.segments)segmentKinds_.push_back(segment.kind);presented_=std::min(presented,segmentKinds_.size());labels_={label,label};const auto rows=segmentKinds_.size()>=LoadingConstants::TapeRows?LoadingConstants::TapeRows:1;setFixedHeight(qRound(rows*LoadingConstants::StripRowHeight*DisplayDensity::factor(this)));setVisible(!segmentKinds_.empty());update();}
void CaptureProgressView::setLinear(const std::vector<GwTrackState>& states, const QString& label) {
    segmentKinds_.clear();
    linearStates_ = states;
    linearCompleted_ = static_cast<std::size_t>(std::count_if(states.begin(), states.end(),
        [](auto state) { return state != GwTrackState::Pending; }));
    labels_ = {label};
    setFixedHeight(qRound(LoadingConstants::StripRowHeight * DisplayDensity::factor(this)));
    setVisible(!states.empty());
    update();
}

void CaptureProgressView::paintEvent(QPaintEvent*){
    namespace L=LoadingConstants;
    if (!linearStates_.empty()) {
        QPainter painter(this);
        painter.setClipRect(rect());
        const auto scale = DisplayDensity::factor(this);
        const auto labelWidth = L::StripLabelWidth * scale;
        const auto countWidth = L::StripCountWidth * scale;
        const auto available = std::max(C::ProgressMinimumWidth, width() - labelWidth - countWidth);
        const auto cellWidth = available / linearStates_.size();
        const auto barHeight = L::StripHeight * scale;
        const auto y = (height() - barHeight) * L::Half;
        painter.setPen(themes_.data()->color(ThemeConstants::TextBrush));
        painter.drawText(QRectF(0, 0, labelWidth, height()), Qt::AlignVCenter | Qt::AlignLeft,
            fontMetrics().elidedText(labels_.value(0), Qt::ElideRight, static_cast<int>(labelWidth)));
        painter.setPen(themes_.data()->color(ThemeConstants::MutedTextBrush));
        painter.drawText(QRectF(width() - countWidth, 0, countWidth, height()), Qt::AlignVCenter | Qt::AlignRight,
            QString::number(linearCompleted_) + QString::fromLatin1(C::ProgressDivider) + QString::number(linearStates_.size()));
        painter.setClipRect(QRectF(labelWidth, y, available, barHeight), Qt::IntersectClip);
        painter.setPen(Qt::NoPen);
        for (std::size_t index = 0; index < linearStates_.size(); ++index) {
            const auto state = linearStates_[index];
            const auto color = state == GwTrackState::Failed ? L::FailedRgb :
                state == GwTrackState::Success ? L::SuccessRgb : L::PendingRgb;
            painter.setBrush(QColor::fromRgb(color));
            const auto width = std::clamp(cellWidth - C::ProgressCellGap * scale,
                std::min(cellWidth, C::ProgressMinimumWidth), cellWidth);
            painter.drawRect(QRectF(labelWidth + index * cellWidth, y, width, barHeight));
        }
        return;
    }
    if(!segmentKinds_.empty()){QPainter painter(this);painter.setClipRect(rect());const auto scale=DisplayDensity::factor(this),rowHeight=L::StripRowHeight*scale,labelWidth=L::StripLabelWidth*scale,countWidth=L::StripCountWidth*scale;const auto rows=segmentKinds_.size()>=L::TapeRows?L::TapeRows:1;const auto rowSize=(segmentKinds_.size()+rows-1)/rows;const auto available=std::max(C::ProgressMinimumWidth,width()-labelWidth-countWidth);
        for(int row=0;row<rows;++row){const auto start=row*rowSize;const auto count=std::min(rowSize,segmentKinds_.size()-start);const auto y=row*rowHeight,cell=available/count;const auto completed=presented_>start?std::min(count,presented_-start):0;painter.setPen(themes_.data()->color(ThemeConstants::TextBrush));painter.drawText(QRectF(0,y,labelWidth,rowHeight),Qt::AlignLeft|Qt::AlignVCenter,fontMetrics().elidedText(labels_.value(row),Qt::ElideRight,static_cast<int>(labelWidth)));painter.save();painter.setClipRect(QRectF(labelWidth,y,available,rowHeight),Qt::IntersectClip);for(std::size_t index=0;index<count;++index){painter.setPen(Qt::NoPen);painter.setBrush(QColor::fromRgb(index<completed?L::TapeColors[static_cast<std::size_t>(segmentKinds_[start+index])]:L::PendingRgb));painter.drawRoundedRect(QRectF(labelWidth+index*cell+L::StripMargin*scale,y+(rowHeight-L::StripHeight*scale)*L::Half,std::max(C::ProgressMinimumWidth,cell-C::ProgressCellGap*scale),L::StripHeight*scale),L::StripRadius*scale,L::StripRadius*scale);}painter.restore();painter.setPen(themes_.data()->color(ThemeConstants::MutedTextBrush));painter.drawText(QRectF(width()-countWidth,y,countWidth,rowHeight),Qt::AlignRight|Qt::AlignVCenter,QString::number(completed)+QString::fromLatin1(C::ProgressDivider)+QString::number(count));}return;}
    if(progress_.cylinders.empty())return;QPainter painter(this);painter.setClipRect(rect());const auto theme=themes_.data();const auto scale=DisplayDensity::factor(this);const auto rowHeight=(compact_?L::StripRowHeight:C::ProgressRowHeight)*scale,labelWidth=(compact_?L::StripLabelWidth:C::ProgressLabelWidth)*scale,countWidth=(compact_?L::StripCountWidth:C::ProgressCountWidth)*scale;const auto area=std::max(C::ProgressMinimumWidth,static_cast<double>(width()-labelWidth-countWidth));
    bool retry=false;if(progress_.cylinder&&progress_.head){const auto found=progress_.states.find({*progress_.cylinder,*progress_.head});retry=found!=progress_.states.end()&&found->second==GwTrackState::Retry;}
    for(std::size_t row=0;row<progress_.heads.size();++row){const auto head=progress_.heads[row];
        const auto perHead = progress_.cylindersPerHead.find(head);
        const auto& cylinders = perHead == progress_.cylindersPerHead.end() ? progress_.cylinders : perHead->second;
        if (cylinders.empty()) continue;
        const auto cell = area / cylinders.size();
        const auto y=row*rowHeight;painter.setPen(theme->color(ThemeConstants::TextBrush));painter.drawText(QRectF(0,y,labelWidth,rowHeight),Qt::AlignVCenter|Qt::AlignLeft,fontMetrics().elidedText(labels_.value(static_cast<int>(row)),Qt::ElideRight,static_cast<int>(labelWidth)));const auto count=progress_.completedPerHead.find(head);const auto completed=count==progress_.completedPerHead.end()?0:count->second;
        painter.save();painter.setClipRect(QRectF(labelWidth,y,area,rowHeight),Qt::IntersectClip);
        for(std::size_t index=0;index<cylinders.size();++index){const auto cylinder=cylinders[index];const auto found=progress_.states.find({cylinder,head});const auto state=found==progress_.states.end()?GwTrackState::Pending:found->second;auto color=compact_?QColor::fromRgb(L::PendingRgb):theme->color(ThemeConstants::ControlBrush);switch(state){case GwTrackState::Success:color=compact_?QColor::fromRgb(L::SuccessRgb):theme->color(C::ProgressSuccessBrush);break;case GwTrackState::Retry:color=compact_?QColor::fromRgb(L::RetryRgb):theme->color(C::ProgressRetryBrush);break;case GwTrackState::Failed:color=compact_?QColor::fromRgb(L::FailedRgb):theme->color(C::ProgressFailedBrush);break;default:break;}
            if(running_&&!retry&&progress_.nextCylinder==cylinder&&progress_.nextHead==head)color=compact_?QColor::fromRgb(L::ActiveRgb):theme->color(ThemeConstants::AccentBrush);const auto margin=compact_?L::StripMargin*scale:C::Gap*scale;const auto height=compact_?L::StripHeight*scale:rowHeight-margin-margin;const QRectF rect(labelWidth+index*cell+L::StripMargin*scale,y+(rowHeight-height)*L::Half,std::max(C::ProgressMinimumWidth,cell-C::ProgressCellGap*scale),height);painter.setPen(Qt::NoPen);painter.setBrush(color);painter.drawRoundedRect(rect,L::StripRadius*scale,L::StripRadius*scale);}
        painter.restore();painter.setPen(theme->color(ThemeConstants::MutedTextBrush));painter.drawText(QRectF(width()-countWidth,y,countWidth,rowHeight),Qt::AlignVCenter|Qt::AlignRight,QString::number(completed)+QString::fromLatin1(C::ProgressDivider)+QString::number(cylinders.size()));
    }
}
}
