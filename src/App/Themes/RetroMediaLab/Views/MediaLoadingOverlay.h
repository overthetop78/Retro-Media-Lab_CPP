#pragma once
#include "Media/OpeningProgress.h"
#include "Shared/Presentation/ThemeService.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QWidget>
#include <QElapsedTimer>
class QLabel;class QProgressBar;class QPushButton;
namespace RetroMediaLab {
class MediaLoadingOverlay final:public QWidget {
public:
    MediaLoadingOverlay(ThemeService&,Translations::TranslationService&,std::function<void()>,QWidget*,bool alignTop=false,bool showFormats=false);
    void display(bool,const Infrastructure::LoadingProgress&);
protected:
    bool eventFilter(QObject*,QEvent*)override;
    void resizeEvent(QResizeEvent*) override;
private:
    void refreshLabels();
    QString stageText_, detailText_, formatText_;
    QElapsedTimer formatTimer_;
    Translations::TranslationService& translations_;
    QWidget* card_;
    QLabel *stage_,*detail_,*format_=nullptr;
    QProgressBar* progress_;
    QPushButton* stop_;
};
}
