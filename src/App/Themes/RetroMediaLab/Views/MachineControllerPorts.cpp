#include "MachineControllerPorts.h"
#include "Shared/Controls/ControllerOutputs.h"
#include "Shared/Controls/ControllerArtwork.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPointer>
#include <QTabWidget>
#include <QVBoxLayout>
#include <algorithm>
#include <array>

namespace RetroMediaLab {
namespace {
using namespace Emulation;
using Translate = MachineInputBindings::Translate;
constexpr int BindingWeight = 3, ArtworkWeight = 2;
constexpr auto PortLabel = "Emulation.Controller.Port";
constexpr auto Mappings = "Emulation.Controller.Mappings";
constexpr auto ControllerType = "Emulation.Controller.Type";
constexpr auto PeripheralType = "Emulation.Controller.PeripheralType";
constexpr auto Model = "Emulation.Controller.Visual";
constexpr auto ModelPrefix = "Emulation.Controller.Visual.Model.";
constexpr auto InKeyboardTab = "Emulation.Controller.ConfigureInKeyboardTab";
constexpr auto InMouseTab = "Emulation.Controller.ConfigureInMouseTab";
constexpr auto KeyboardId = "Keyboard";
constexpr auto MouseId = "Mouse";
constexpr auto NetPlayControllerId = "NetPlayController";
constexpr std::array CategoryKeys{
    "", "Emulation.Controller.None", "Controllers.Enum.Gamepad", "Emulation.Controller.Category.Joystick",
    "Emulation.Controller.Category.PointingDevice", "Emulation.Controller.Category.FlightSimulation",
    "Emulation.Controller.Category.RacingSimulation", "Emulation.Controller.Category.DanceSimulation",
    "Emulation.Controller.Category.SingingSimulation", "Emulation.Controller.Category.OtherSimulation"
};

class PortEditor final : public QWidget {
public:
    PortEditor(ControllerPortSettings& port, Platform::GamepadSource& source, Translate text, Translate moduleText,
        std::function<void()> notify, QWidget* parent)
        : QWidget(parent), port_(port), source_(source), text_(std::move(text)), moduleText_(std::move(moduleText)),
          notify_(std::move(notify)), initialChoice_(port.selectedControllerId),
          initialModel_(port.visualId), initialBindings_(port.bindings.values), initialOutputs_(port.outputBindings),
          type_(new QComboBox(this)), model_(new QComboBox(this)), hint_(new QLabel(this)),
          layout_(new QVBoxLayout(this)) {
        auto* form = new QFormLayout;
        form->addRow(text_(QLatin1String(port.isUsb ? PeripheralType : ControllerType)), type_);
        form->addRow(text_(QLatin1String(Model)), model_);
        layout_->addLayout(form);
        hint_->setWordWrap(true);
        layout_->addWidget(hint_);
        grouped_ = !port_.controllerChoices.isEmpty() &&
            std::ranges::none_of(port_.controllerChoices, [](const auto& choice) {
                const auto category = static_cast<size_t>(choice.category);
                return category == static_cast<size_t>(ControllerCategory::Unspecified) || category >= CategoryKeys.size();
            });
        for (const auto& choice : port_.controllerChoices) {
            const auto category = static_cast<int>(choice.category);
            if (!grouped_) type_->addItem(choiceLabel(choice), choice.id);
            else if (type_->findData(category) < 0) type_->addItem(text_(QLatin1String(CategoryKeys[category])), category);
        }
        if (const auto* current = selectedChoice(port_.selectedControllerId))
            type_->setCurrentIndex(grouped_ ? type_->findData(static_cast<int>(current->category)) : type_->findData(current->id));
        fillModels();
        showBindings();
        showOutputs();
        connect(type_, &QComboBox::activated, this, [this] { fillModels(); selectModel(); });
        connect(model_, &QComboBox::activated, this, [this] { selectModel(); });
    }

    bool valid() const { return !bindings_ || bindings_->valid(); }
    bool changed() const {
        return initialChoice_ != port_.selectedControllerId || initialModel_ != port_.visualId ||
            initialBindings_ != port_.bindings.values || initialOutputs_ != port_.outputBindings;
    }

private:
    const ControllerChoice* selectedChoice(const QString& id) const {
        const auto found = std::ranges::find(port_.controllerChoices, id, &ControllerChoice::id);
        return found == port_.controllerChoices.end() ? nullptr : &*found;
    }

    QString choiceLabel(const ControllerChoice& choice) const {
        return choice.invariantDisplayValue.isEmpty() ? moduleText_(choice.displayResourceKey) : choice.invariantDisplayValue;
    }

    void fillModels() {
        candidates_.clear();
        model_->clear();
        int exact = -1, preferred = -1;
        for (const auto& choice : port_.controllerChoices) {
            if (grouped_ ? type_->currentData().toInt() != static_cast<int>(choice.category)
                         : type_->currentData().toString() != choice.id) continue;
            auto models = choice.compatibleVisualIds.value_or(QStringList{});
            if (models.isEmpty()) models.append(choice.defaultVisualId);
            for (const auto& visual : models) {
                const int row = model_->count();
                if (!visual.isEmpty()) {
                    const auto key = QLatin1String(ModelPrefix) + visual;
                    const auto localized = text_(key);
                    model_->addItem(localized == key ? choiceLabel(choice) : localized);
                } else {
                    model_->addItem(choiceLabel(choice));
                }
                candidates_.append({choice.id, visual});
                if (choice.id == port_.selectedControllerId) {
                    if (visual == port_.visualId) exact = row;
                    if (visual == choice.defaultVisualId || preferred < 0) preferred = row;
                } else if (!grouped_ && visual == choice.defaultVisualId && preferred < 0) {
                    preferred = row;
                }
            }
        }
        model_->setCurrentIndex(exact >= 0 ? exact : preferred >= 0 ? preferred : model_->currentIndex());
        model_->setEnabled(model_->count() > 1);
    }

    void selectModel() {
        if (model_->currentIndex() < 0) return;
        const auto& [id, visual] = candidates_.at(model_->currentIndex());
        const auto* choice = selectedChoice(id);
        if (!choice || (id == port_.selectedControllerId && visual == port_.visualId)) return;
        port_.selectedControllerId = id;
        port_.visualId = visual;
        port_.bindings.definitions = choice->visualBindingDefinitions.value(visual,
            choice->bindingDefinitions.value_or(QList<InputBindingDefinition>{}));
        port_.bindings.compatibleVisualIds = choice->compatibleVisualIds;
        port_.bindings.defaultVisualId = choice->defaultVisualId;
        port_.bindings.visualCommandIds = choice->visualCommandIds;
        showBindings();
        showOutputs();
        notify_();
    }

    void showBindings() {
        delete bindingsSection_.data();
        const auto& id = port_.selectedControllerId;
        const bool mouse = id == QLatin1String(MouseId);
        const bool separate = port_.isUsb && (mouse || id == QLatin1String(KeyboardId) || id == QLatin1String(NetPlayControllerId));
        hint_->setVisible(separate);
        if (separate) hint_->setText(text_(QLatin1String(mouse ? InMouseTab : InKeyboardTab)));
        if (separate || port_.bindings.definitions.isEmpty()) return;
        bindingsSection_ = new QWidget(this);
        auto* body = new QHBoxLayout(bindingsSection_);
        body->setContentsMargins({});
        bindings_ = new MachineInputBindings(port_.bindings, text_, moduleText_, notify_, bindingsSection_);
        body->addWidget(bindings_, BindingWeight);
        const auto visual = port_.visualId.isEmpty() ? modelVisual() : port_.visualId;
        const auto* choice = selectedChoice(id);
        auto commands = choice ? choice->visualCommandIds : port_.bindings.visualCommandIds;
        commands.removeIf([&](const auto& association) {
            return std::ranges::find(port_.bindings.definitions, association.value(), &InputBindingDefinition::id) ==
                port_.bindings.definitions.end();
        });
        if (auto* artwork = ControllerArtwork::create(visual, std::move(commands), [this](const QString& id) {
                if (bindings_) bindings_->assign(id);
            }, bindingsSection_)) {
            artwork->observeInput(source_, *bindings_);
            body->addWidget(artwork, ArtworkWeight, Qt::AlignTop);
        }
        layout_->addWidget(bindingsSection_);
    }

    QString modelVisual() const {
        return model_->currentIndex() >= 0 ? candidates_.at(model_->currentIndex()).second : QString{};
    }

    void showOutputs() {
        delete outputsPage_.data();
        auto feedback = controllerModelFeedback(port_);
        if (!feedback || feedback->availableCommands.isEmpty()) return;
        outputs_ = std::move(*feedback);
        outputsPage_ = new QWidget(this);
        auto* form = new QFormLayout(outputsPage_);
        layout_->addWidget(outputsPage_);
        addControllerOutputs(*form, outputs_, source_, text_, moduleText_, [this] {
            for (const auto& command : outputs_.availableCommands) port_.outputBindings.remove(command);
            for (auto binding = outputs_.outputs.cbegin(); binding != outputs_.outputs.cend(); ++binding)
                port_.outputBindings.insert(binding.key(), binding.value());
            notify_();
        });
    }

    ControllerPortSettings& port_;
    Platform::GamepadSource& source_;
    Translate text_, moduleText_;
    std::function<void()> notify_;
    QString initialChoice_, initialModel_;
    QHash<QString, QString> initialBindings_;
    QHash<QString, ControllerOutputBinding> initialOutputs_;
    QComboBox *type_, *model_;
    QLabel* hint_;
    QVBoxLayout* layout_;
    bool grouped_ = false;
    QList<std::pair<QString, QString>> candidates_;
    QPointer<MachineInputBindings> bindings_;
    QPointer<QWidget> bindingsSection_;
    QPointer<QWidget> outputsPage_;
    ControllerPortFeedback outputs_{};
};
}

MachineControllerPorts::MachineControllerPorts(QList<Emulation::ControllerPortSettings>& ports,
    Platform::GamepadSource& source, Translate text, Translate moduleText, std::function<void()> changed, QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    auto* group = new QGroupBox(text(QLatin1String(Mappings)), this);
    layout->addWidget(group);
    auto* body = new QVBoxLayout(group);
    auto* tabs = new QTabWidget(group);
    body->addWidget(tabs);
    for (auto& port : ports) {
        auto* editor = new PortEditor(port, source, text, moduleText, changed, tabs);
        const auto label = port.displayName.isEmpty()
            ? text(QLatin1String(PortLabel)).arg(port.displayNumber.value_or(port.number)) : port.displayName;
        tabs->addTab(editor, label);
        validators_.append([editor] { return editor->valid(); });
        changes_.append([editor] { return editor->changed(); });
    }
}

bool MachineControllerPorts::valid() const {
    return std::ranges::all_of(validators_, [](const auto& valid) { return valid(); });
}

bool MachineControllerPorts::changed() const {
    return std::ranges::any_of(changes_, [](const auto& changed) { return changed(); });
}
}
