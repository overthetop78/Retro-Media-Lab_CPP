#include "Shared/Controls/MachineInputBindings.h"
#include "Shared/Controls/EmulationShortcutCapture.h"
#include <RetroMediaLab/Emulation/InputProcessing.h>

#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QStyle>
#include <QToolButton>
#include <algorithm>
#include <span>

namespace RetroMediaLab {
namespace {
namespace Physical = EmulationPhysicalShortcuts;
using Labels = std::function<QString(const QString&)>;
using LabelPair = std::pair<const char*, const char*>;
constexpr auto LabelSeparator = u" · ";
constexpr auto BindingSeparator = " + ";
constexpr auto Unassigned = "Emulation.Input.Binding.LabelUnassigned";
constexpr LabelPair MouseLabels[] = {
    {"Left", "Emulation.Mouse.Button.Left"}, {"Right", "Emulation.Mouse.Button.Right"},
    {"Middle", "Emulation.Mouse.Button.Middle"}, {"XButton1", "Emulation.Mouse.Button.4"},
    {"XButton2", "Emulation.Mouse.Button.5"}, {"WheelUp", "Emulation.Mouse.Wheel.Up"},
    {"WheelDown", "Emulation.Mouse.Wheel.Down"}, {"WheelLeft", "Emulation.Mouse.Wheel.Left"},
    {"WheelRight", "Emulation.Mouse.Wheel.Right"},
};
constexpr auto LeftDirection = "Emulation.Controller.Action.Left";
constexpr auto RightDirection = "Emulation.Controller.Action.Right";
constexpr LabelPair Directions[] = {
    {"Up", "Emulation.Controller.Action.Up"}, {"Down", "Emulation.Controller.Action.Down"},
    {"Left", LeftDirection}, {"Right", RightDirection},
};
constexpr LabelPair ControllerButtons[] = {
    {"ButtonA", "A"}, {"ButtonB", "B"}, {"ButtonX", "X"}, {"ButtonY", "Y"},
    {"LeftShoulder", "LB"}, {"RightShoulder", "RB"}, {"LeftTrigger", "LT"}, {"RightTrigger", "RT"},
    {"LeftStickClick", "L3"}, {"RightStickClick", "R3"}, {"XboxButton", "Xbox"},
};
constexpr auto ControlLabel = "Emulation.Keyboard.Modifier.Control";
constexpr auto ShiftLabel = "Emulation.Keyboard.Modifier.Shift";
constexpr auto AltLabel = "Emulation.Keyboard.Modifier.Alt";
constexpr auto AltGrLabel = "Emulation.Keyboard.Modifier.AltGr";
constexpr LabelPair KeyboardModifiers[] = {
    {"Ctrl", ControlLabel}, {"Control", ControlLabel}, {"Shift", ShiftLabel}, {"Maj", ShiftLabel},
    {"Alt", AltLabel}, {"AltGr", AltGrLabel}, {"RightAlt", AltGrLabel},
};
struct SidedModifier { const char* token; const char* label; bool left; };
constexpr SidedModifier SidedModifiers[] = {
    {"LeftCtrl", ControlLabel, true}, {"RightCtrl", ControlLabel, false},
    {"LeftShift", ShiftLabel, true}, {"RightShift", ShiftLabel, false}, {"LeftAlt", AltLabel, true},
};

const char* findLabel(QStringView token, std::span<const LabelPair> labels) {
    const auto entry = std::ranges::find_if(labels,
        [&](const auto& label) { return token.compare(QLatin1String(label.first), Qt::CaseInsensitive) == 0; });
    return entry == labels.end() ? nullptr : entry->second;
}

QString controllerLabel(QStringView source, const Labels& translate) {
    const auto parts = source.split(Physical::Separator, Qt::SkipEmptyParts);
    if (parts.isEmpty()) return source.toString();
    const auto token = parts.back();
    QString label = token.toString();
    constexpr auto LeftStick = "LeftStick";
    constexpr auto RightStick = "RightStick";
    constexpr auto DPad = "DPad";
    constexpr auto XInput = "XInput";
    constexpr auto XInputPort = "X%1";
    constexpr LabelPair ControllerDirections[] = {
        {DPad, nullptr}, {LeftStick, "Emulation.Controller.Stick.Left"}, {RightStick, "Emulation.Controller.Stick.Right"},
    };
    if (const auto* alias = findLabel(token, ControllerButtons)) {
        label = QString::fromLatin1(alias);
    } else {
        for (const auto& [prefix, key] : ControllerDirections) {
            const auto name = QLatin1String(prefix);
            if (!token.startsWith(name, Qt::CaseInsensitive)) continue;
            const auto* direction = findLabel(token.sliced(name.size()), Directions);
            if (!direction) break;
            constexpr auto DPadLabel = "D-pad";
            label = (key ? translate(QLatin1String(key)) : QString::fromLatin1(DPadLabel)) +
                QString::fromUtf16(LabelSeparator) + translate(QLatin1String(direction));
            break;
        }
    }
    if (parts.size() >= 3 && parts.front().compare(QLatin1String(XInput), Qt::CaseInsensitive) == 0) {
        bool valid = false;
        const auto index = parts[1].toInt(&valid);
        if (valid && index >= 0)
            return QString::fromLatin1(XInputPort).arg(qint64(index) + 1) + QString::fromUtf16(LabelSeparator) + label;
    }
    return label;
}

QString bindingPartLabel(QStringView part, const Labels& translate) {
    const auto keyboard = QLatin1String(Physical::KeyboardPrefix);
    const auto mouse = QLatin1String(Physical::MousePrefix);
    const auto controller = QLatin1String(Physical::ControllerPrefix);
    if (part.startsWith(keyboard, Qt::CaseInsensitive)) part = part.sliced(keyboard.size());
    if (part.startsWith(controller, Qt::CaseInsensitive)) return controllerLabel(part.sliced(controller.size()), translate);
    if (part.startsWith(mouse, Qt::CaseInsensitive)) {
        if (const auto* key = findLabel(part.sliced(mouse.size()), MouseLabels)) return translate(QLatin1String(key));
        return part.toString();
    }
    if (const auto* key = findLabel(part, KeyboardModifiers)) return translate(QLatin1String(key));
    const auto modifier = std::ranges::find_if(SidedModifiers,
        [&](const auto& item) { return part.compare(QLatin1String(item.token), Qt::CaseInsensitive) == 0; });
    if (modifier == std::end(SidedModifiers)) return part.toString();
    constexpr auto SidedLabel = "%1 (%2)";
    return QString::fromLatin1(SidedLabel).arg(translate(QLatin1String(modifier->label)),
        translate(QLatin1String(modifier->left ? LeftDirection : RightDirection)));
}
constexpr auto Actions = "Emulation.Input.Actions";
constexpr auto Assign = "Emulation.Input.Assign";
constexpr auto Binding = "Emulation.Input.Binding.Label";
constexpr auto Conflict = "Emulation.Input.Binding.LabelConflict";
constexpr auto Reserved = "Emulation.Input.Binding.LabelReserved";
constexpr auto Restore = "Emulation.Input.Binding.RestoreDefaults";
constexpr auto Search = "Emulation.Input.Binding.Search";
constexpr auto CaptureHint = "Emulation.Input.Capture.Hint";
constexpr auto Remove = "Common.Delete";
constexpr int ActionColumn = 0;
constexpr int ValueColumn = 1;
constexpr int ControlsColumn = 2;
constexpr int HeadingRow = 0;
constexpr int FirstActionRow = 1;

}

QString machineBindingLabel(const QString& binding, const Labels& translate) {
    QStringList parts;
    for (const auto part : QStringView(binding).split(Physical::ChordSeparator, Qt::SkipEmptyParts)) {
        const auto token = part.trimmed();
        if (!token.isEmpty()) parts.push_back(bindingPartLabel(token, translate));
    }
    return parts.isEmpty() ? translate(QLatin1String(Unassigned)) : parts.join(QLatin1String(BindingSeparator));
}

MachineInputBindings::MachineInputBindings(Emulation::InputBindingSet& bindings,
    Translate text, Translate moduleText, std::function<void()> changed, QWidget* parent)
    : QWidget(parent), bindings_(bindings), original_(bindings.values), text_(std::move(text)),
      search_(new QLineEdit(this)), issue_(new QLabel(this)) {
    auto* layout = new QVBoxLayout(this);
    auto* group = new QGroupBox(text_(QLatin1String(Actions)), this);
    layout->addWidget(group);
    auto* body = new QVBoxLayout(group);
    search_->setPlaceholderText(text_(QLatin1String(Search)));
    body->addWidget(search_);
    auto* grid = new QGridLayout;
    body->addLayout(grid);
    grid->addWidget(new QLabel(text_(QLatin1String(Binding)), group), HeadingRow, ValueColumn);
    grid->setColumnStretch(ActionColumn, 1);
    grid->setColumnStretch(ValueColumn, 1);
    for (const auto& definition : bindings_.definitions) {
        auto* label = new QLabel(definition.invariantDisplayValue.isEmpty()
            ? moduleText(definition.displayResourceKey) : definition.invariantDisplayValue, group);
        auto* value = new QLabel(group);
        value->setTextFormat(Qt::PlainText);
        auto* controls = new QWidget(group);
        auto* buttons = new QHBoxLayout(controls);
        buttons->setContentsMargins({});
        auto* assign = new QPushButton(text_(QLatin1String(Assign)), controls);
        auto* clear = new QToolButton(controls);
        clear->setIcon(clear->style()->standardIcon(QStyle::SP_DialogDiscardButton));
        clear->setToolTip(text_(QLatin1String(Remove)));
        clear->setAccessibleName(clear->toolTip());
        buttons->addWidget(assign);
        buttons->addWidget(clear);
        const auto index = FirstActionRow + int(rows_.size());
        grid->addWidget(label, index, ActionColumn);
        grid->addWidget(value, index, ValueColumn);
        grid->addWidget(controls, index, ControlsColumn);
        rows_.append({&definition, label, value, controls, clear, assign});
        const auto set = [this, id = definition.id, oppositeId = definition.oppositeDirectionId, changed](
            const QString& replacement, bool captured) {
            bindings_.values.insert(id, replacement);
            if (captured && !oppositeId.isEmpty() &&
                std::ranges::find(bindings_.definitions, oppositeId, &Emulation::InputBindingDefinition::id)
                    != bindings_.definitions.cend()) {
                if (const auto opposite = Emulation::InputProcessing::oppositeControllerAxisSource(replacement))
                    bindings_.values.insert(oppositeId, *opposite);
            }
            refresh();
            changed();
        };
        connect(assign, &QPushButton::clicked, this, [this, label, set] {
            EmulationShortcutCapture capture(this, label->text(), text_(QLatin1String(CaptureHint)),
                false, bindings_.prefixKeyboardSource);
            if (capture.exec() == QDialog::Accepted) set(capture.binding(), true);
        });
        connect(clear, &QToolButton::clicked, this, [set] { set({}, false); });
    }
    auto* restore = new QPushButton(text_(QLatin1String(Restore)), group);
    body->addWidget(restore, 0, Qt::AlignRight);
    issue_->setTextFormat(Qt::PlainText);
    issue_->setWordWrap(true);
    body->addWidget(issue_);
    connect(search_, &QLineEdit::textChanged, this, [this] { refresh(); });
    connect(restore, &QPushButton::clicked, this, [this, changed] {
        for (const auto& row : std::as_const(rows_)) bindings_.values.remove(row.definition->id);
        refresh();
        changed();
    });
    refresh();
}

void MachineInputBindings::assign(const QString& actionId) {
    const auto row = std::ranges::find_if(rows_, [&](const auto& candidate) { return candidate.definition->id == actionId; });
    if (row == rows_.end()) return;
    search_->clear();
    row->assign->setFocus();
    row->assign->click();
}

QString MachineInputBindings::binding(const QString& actionId) const {
    const auto row = std::ranges::find_if(rows_, [&](const auto& candidate) { return candidate.definition->id == actionId; });
    return row == rows_.end() ? QString{} : bindings_.values.value(actionId, row->definition->defaultBinding);
}

bool MachineInputBindings::changed() const { return bindings_.values != original_; }
bool MachineInputBindings::valid() const { return issue_->text().isEmpty(); }

void MachineInputBindings::refresh() {
    const auto query = search_->text().trimmed();
    QList<std::pair<Physical::Chord, QString>> used;
    QStringList issues;
    for (const auto& row : std::as_const(rows_)) {
        const auto& definition = *row.definition;
        const auto binding = bindings_.values.value(definition.id, definition.defaultBinding);
        const auto display = machineBindingLabel(binding, text_);
        row.value->setText(display);
        row.clear->setEnabled(!binding.isEmpty());
        const bool visible = row.label->text().contains(query, Qt::CaseInsensitive) ||
            display.contains(query, Qt::CaseInsensitive) || binding.contains(query, Qt::CaseInsensitive);
        row.label->setVisible(visible);
        row.value->setVisible(visible);
        row.controls->setVisible(visible);
        if (binding.isEmpty()) continue;
        const auto chord = Physical::chord(binding);
        if (chord.reserved) issues.append(text_(QLatin1String(Reserved)) + QLatin1String(": ") + binding);
        if (!chord.keyboard && chord.sources.isEmpty()) continue;
        for (const auto& [previous, label] : std::as_const(used)) {
            if (previous.sources == chord.sources && previous.keys == chord.keys && previous.modifiers == chord.modifiers)
                issues.append(text_(QLatin1String(Conflict)) + QLatin1String(": ") + label + QLatin1String(" / ") + row.label->text());
        }
        used.append({chord, row.label->text()});
    }
    issue_->setText(issues.join(QLatin1Char('\n')));
    issue_->setVisible(!valid());
}
}
