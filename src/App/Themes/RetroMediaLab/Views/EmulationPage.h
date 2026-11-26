#pragma once
#include <QWidget>
#include <memory>

namespace RetroMediaLab {
namespace Translations { class TranslationService; }
struct WorkspaceState;
class ThemeService;

class EmulationPage final : public QWidget {
public:
    EmulationPage(ThemeService&, Translations::TranslationService&, WorkspaceState&, QWidget*);
    ~EmulationPage() override;
    void openSettings();
    void openModuleOptions(const QString&);
private:
    struct Data;
    std::unique_ptr<Data> data_;
};
}
