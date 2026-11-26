#pragma once
#include "Shared/Presentation/ThemeService.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QTableWidget>
namespace RetroMediaLab {
enum class CapabilityState { Pass, Fail, Unknown, Information };
struct CapabilityRow {QString capability;CapabilityState state;QString source,details;};
class HardwareCapabilitiesTable final : public QTableWidget {
public:
    HardwareCapabilitiesTable(const QVector<CapabilityRow>&,std::shared_ptr<const ThemeData>,Translations::TranslationService&,QWidget*);
protected:
    void resizeEvent(QResizeEvent*) override;
private:
    void fitRows();
};
}
