#pragma once
#include "State/WorkspaceState.h"
#include "Maintenance/GreaseweazleTools.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QWidget>
#include <memory>
class QLabel;
class QLineEdit;
class QCheckBox;
class QPlainTextEdit;
class QPushButton;
class QMessageBox;
namespace RetroMediaLab {
class GreaseweazleToolPage final : public QWidget {
public:
    GreaseweazleToolPage(const GwToolDefinition&, Translations::TranslationService&, WorkspaceState&, QWidget*);
    ~GreaseweazleToolPage() override;
    void refresh();
private:
    void execute();
    void start();
    void confirm(const char* title, const char* message, std::function<void()>);
    QString text(const QString&) const;
    struct Field {QString id; const char* labelKey; GwParameterKind kind; QLabel* label; QLineEdit* edit; QCheckBox* check;};
    QString verb_;
    Translations::TranslationService& translations_;
    WorkspaceState& state_;
    QVector<Field> fields_;
    QLabel *summary_, *outputTitle_, *commandTitle_;
    QLineEdit* command_;
    QPlainTextEdit* output_;
    QPushButton* execute_;
    std::unique_ptr<QMessageBox> confirmation_;
    GreaseweazleTools::Subscription toolsSubscription_;
    Translations::TranslationService::Subscription languageSubscription_;
};
}
