#pragma once

#include <RetroMediaLab/Emulation/Sdk.h>
#include <QKeyEvent>
#include <QLineEdit>
#include <QLocale>
#include <QRegularExpression>
#include <QSignalBlocker>
#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>

namespace RetroMediaLab {
class ModuleNumberEdit final : public QLineEdit {
public:
    ModuleNumberEdit(const Emulation::SettingsNumericRange* range, const QString& invariant,
        const QLocale& locale, QWidget* parent = nullptr)
        : QLineEdit(parent), decimalPoint_(locale.decimalPoint()) {
        if (range && std::isfinite(range->minimum) && std::isfinite(range->maximum) &&
            range->minimum <= range->maximum && std::isfinite(range->step) && range->step > 0.) {
            minimum_ = range->minimum;
            maximum_ = range->maximum;
            step_ = range->step;
        }
        setLayoutDirection(Qt::LeftToRight);
        setValue(invariant);
        connect(this, &QLineEdit::textEdited, this, [this](const QString& text) { acceptEdit(text); });
        connect(this, &QLineEdit::editingFinished, this, [this] { restoreCompleteValue(); });
    }

    QString value() const { return format(lastValue_); }

    void setValue(const QString& invariant) {
        bool valid = false;
        const double parsed = invariant.toDouble(&valid);
        lastValue_ = std::clamp(valid && std::isfinite(parsed) ? parsed : 0., minimum_, maximum_);
        restoreCompleteValue();
    }

    void onValueChanged(std::function<void()> callback) { changed_ = std::move(callback); }

protected:
    void keyPressEvent(QKeyEvent* event) override {
        if (event->key() != Qt::Key_Up && event->key() != Qt::Key_Down) {
            QLineEdit::keyPressEvent(event);
            return;
        }
        const double current = parse(text()).value_or(lastValue_);
        const double offset = event->key() == Qt::Key_Up ? step_ : -step_;
        const double next = std::clamp(current + offset, minimum_, maximum_);
        const double old = lastValue_;
        lastValue_ = next;
        restoreCompleteValue();
        if (changed_ && old != next) changed_();
        event->accept();
    }

private:
    static QString format(double value) {
        return QLocale::c().toString(value, 'f', QLocale::FloatingPointShortest).remove(u',');
    }

    std::optional<double> parse(const QString& value) const {
        auto normalized = value;
        normalized.replace(decimalPoint_, QStringLiteral("."));
        static const QRegularExpression decimal(
            QRegularExpression::anchoredPattern(QStringLiteral(R"([+-]?(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+))")));
        if (!decimal.match(normalized).hasMatch()) return std::nullopt;
        bool valid = false;
        const double usable = normalized.toDouble(&valid);
        return valid && std::isfinite(usable) ? std::optional<double>(usable) : std::nullopt;
    }

    void restoreCompleteValue() {
        auto local = format(lastValue_);
        local.replace(QStringLiteral("."), decimalPoint_);
        acceptedText_ = local;
        const QSignalBlocker blocker(this);
        setText(local);
    }

    void acceptEdit(const QString& candidate) {
        auto normalized = candidate;
        normalized.replace(decimalPoint_, QStringLiteral("."));
        const bool integerStep = std::trunc(step_) == step_;
        const auto number = parse(candidate);
        const bool intermediate = normalized.isEmpty() || normalized == QStringLiteral(".") ||
            (minimum_ < 0. && (normalized == QStringLiteral("-") || normalized == QStringLiteral("-.")));
        const bool editingAllowed = !(integerStep && normalized.contains(u'.')) &&
            (intermediate || (number && *number >= std::min(0., minimum_) && *number <= std::max(0., maximum_)));
        if (!editingAllowed) {
            const QSignalBlocker blocker(this);
            setText(acceptedText_);
            return;
        }
        acceptedText_ = candidate;
        if (number && *number >= minimum_ && *number <= maximum_ && *number != lastValue_) {
            lastValue_ = *number;
            if (changed_) changed_();
        }
    }

    double minimum_ = std::numeric_limits<double>::lowest();
    double maximum_ = std::numeric_limits<double>::max();
    double step_ = 1.;
    double lastValue_ = 0.;
    QString decimalPoint_, acceptedText_;
    std::function<void()> changed_;
};
}
