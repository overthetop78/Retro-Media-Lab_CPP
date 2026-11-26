#pragma once
#include "AppApi.h"

#include <RetroMediaLab/Emulation/InputSettings.h>
#include <QWidget>
#include <functional>

class QLabel;
class QLineEdit;
class QToolButton;
class QPushButton;

namespace RetroMediaLab {
RETROMEDIALAB_APP_API QString machineBindingLabel(const QString& binding,
    const std::function<QString(const QString&)>& translate);

class MachineInputBindings final : public QWidget {
public:
    using Translate = std::function<QString(const QString&)>;
    RETROMEDIALAB_APP_API MachineInputBindings(Emulation::InputBindingSet&, Translate, Translate moduleText,
        std::function<void()> changed, QWidget* parent);
    RETROMEDIALAB_APP_API bool changed() const;
    RETROMEDIALAB_APP_API void assign(const QString& actionId);
    RETROMEDIALAB_APP_API QString binding(const QString& actionId) const;
    RETROMEDIALAB_APP_API bool valid() const;

private:
    struct Row {
        const Emulation::InputBindingDefinition* definition;
        QLabel* label;
        QLabel* value;
        QWidget* controls;
        QToolButton* clear;
        QPushButton* assign;
    };
    void refresh();

    Emulation::InputBindingSet& bindings_;
    const QHash<QString, QString> original_;
    Translate text_;
    QLineEdit* search_;
    QLabel* issue_;
    QList<Row> rows_;
};
}
