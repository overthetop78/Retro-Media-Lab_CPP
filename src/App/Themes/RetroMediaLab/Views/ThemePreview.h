#pragma once
#include "Themes/ThemeService.h"
#include <QPixmap>
#include <QPointer>
#include <QWidget>
namespace RetroMediaLab {
struct WorkspaceState;
class ThemePreview final : public QWidget {
public:
    ThemePreview(ThemeService&, Translations::TranslationService&, const WorkspaceState&, QWidget* source, QWidget* parent);
    void setSettings(const QJsonObject&);
    void refresh();
protected:
    void showEvent(QShowEvent*) override;
    void paintEvent(QPaintEvent*) override;
private:
    ThemeService& themes_;
    Translations::TranslationService& translations_;
    const WorkspaceState& state_;
    QPointer<QWidget> source_;
    QJsonObject settings_;
    QPixmap image_;
    bool scheduled_ = false;
};
}
