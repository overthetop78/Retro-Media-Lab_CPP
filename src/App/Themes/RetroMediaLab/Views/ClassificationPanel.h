#pragma once
#include "Media/ClassificationSelection.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QWidget>
#include <QIcon>
#include <functional>
class QComboBox;class QCheckBox;class QLabel;
namespace RetroMediaLab {
QIcon recognizedFormatIcon();
class ClassificationPanel final:public QWidget {
public:
    ClassificationPanel(Infrastructure::ClassificationSelection&,Translations::TranslationService&,bool showAutomatic,std::function<void(bool)>,QWidget*);
    void refresh();
private:
    Infrastructure::ClassificationSelection& selection_;
    Translations::TranslationService& translations_;
    std::function<void(bool)> changed_;
    bool showAutomatic_;
    QComboBox *machine_,*format_,*protection_;
    QCheckBox* automatic_;
    QLabel *machineLabel_,*formatLabel_,*protectionLabel_;
};
}
