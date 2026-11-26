#pragma once

#include "Shared/Emulation/ModuleNumericSetting.h"

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QSignalBlocker>
#include <QSlider>
#include <QToolButton>
#include <QVBoxLayout>

#include <functional>

namespace RetroMediaLab {
class ModuleEditableSlider final : public QWidget {
public:
    static ModuleEditableSlider* create(const Emulation::SettingsNumericRange& range,
        const Emulation::EditableNumericSetting& settings, const QString& value, const QString& automaticLabel,
        QWidget* parent) {
        auto fine = range;
        fine.step = settings.fineStep.value_or(range.step);
        fine.automaticValue.reset();
        auto ticks = ModuleNumericSetting::from(fine);
        if (!ticks) return nullptr;
        return new ModuleEditableSlider(range, fine.step, std::move(*ticks), value, automaticLabel, parent);
    }

    QString value() const {
        if (automatic_ && automatic_->isChecked()) return *range_.automaticValue;
        return QString::number(input_->value(), 'f', Decimals);
    }

    void setValue(const QString& value) {
        const bool automatic = range_.automaticValue && value == *range_.automaticValue;
        if (automatic_) {
            const QSignalBlocker blocker(automatic_);
            automatic_->setChecked(automatic);
            row_->setEnabled(!automatic);
        }
        if (!automatic) {
            bool valid = false;
            const double parsed = value.toDouble(&valid);
            setNumber(valid && std::isfinite(parsed) ? parsed : range_.minimum);
        }
    }

    void onValueChanged(std::function<void()> changed) { changed_ = std::move(changed); }

private:
    static constexpr int Decimals = 2;
    static constexpr auto Decrement = "−";
    static constexpr auto Increment = "+";

    ModuleEditableSlider(Emulation::SettingsNumericRange range, double fineStep, ModuleNumericSetting ticks,
        const QString& value, const QString& automaticLabel, QWidget* parent)
        : QWidget(parent), range_(std::move(range)), ticks_(std::move(ticks)), fineStep_(fineStep) {
        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        if (range_.automaticValue) {
            automatic_ = new QCheckBox(automaticLabel, this);
            layout->addWidget(automatic_);
        }
        row_ = new QWidget(this);
        auto* controls = new QHBoxLayout(row_);
        controls->setContentsMargins(0, 0, 0, 0);
        slider_ = new QSlider(range_.vertical ? Qt::Vertical : Qt::Horizontal, row_);
        slider_->setRange(ticks_.minimum(), ticks_.maximum());
        const auto step = std::clamp(std::round(range_.step / fineStep_), 1., double(std::numeric_limits<int>::max()));
        slider_->setSingleStep(int(step));
        slider_->setPageStep(int(step));
        controls->addWidget(slider_, 1);
        input_ = new QDoubleSpinBox(row_);
        input_->setDecimals(Decimals);
        input_->setRange(range_.minimum, range_.maximum);
        input_->setSingleStep(range_.step);
        input_->setKeyboardTracking(false);
        input_->setButtonSymbols(QAbstractSpinBox::NoButtons);
        controls->addWidget(input_);
        auto* decrement = new QToolButton(row_);
        decrement->setText(QString::fromUtf8(Decrement));
        controls->addWidget(decrement);
        auto* increment = new QToolButton(row_);
        increment->setText(QString::fromLatin1(Increment));
        controls->addWidget(increment);
        layout->addWidget(row_);

        setNumber(range_.minimum);
        setValue(value);
        const auto apply = [this](double value) {
            setNumber(value);
            if (changed_) changed_();
        };
        connect(slider_, &QSlider::valueChanged, this, [this, apply](int index) { apply(ticks_.value(index).toDouble()); });
        connect(input_, &QDoubleSpinBox::valueChanged, this, apply);
        connect(decrement, &QToolButton::clicked, this, [this, apply] { apply(input_->value() - range_.step); });
        connect(increment, &QToolButton::clicked, this, [this, apply] { apply(input_->value() + range_.step); });
        if (automatic_) connect(automatic_, &QCheckBox::toggled, this, [this](bool checked) {
            row_->setEnabled(!checked);
            if (changed_) changed_();
        });
    }

    void setNumber(double number) {
        const auto bounded = std::clamp(number, range_.minimum, range_.maximum);
        const double snapped = std::nearbyint(bounded / fineStep_) * fineStep_;
        const auto value = std::clamp(snapped, range_.minimum, range_.maximum);
        const QSignalBlocker inputBlocker(input_);
        const QSignalBlocker sliderBlocker(slider_);
        input_->setValue(value);
        slider_->setValue(ticks_.index(QString::number(input_->value(), 'f', Decimals)));
    }

    Emulation::SettingsNumericRange range_;
    ModuleNumericSetting ticks_;
    double fineStep_;
    QWidget* row_ = nullptr;
    QCheckBox* automatic_ = nullptr;
    QSlider* slider_ = nullptr;
    QDoubleSpinBox* input_ = nullptr;
    std::function<void()> changed_;
};
}
