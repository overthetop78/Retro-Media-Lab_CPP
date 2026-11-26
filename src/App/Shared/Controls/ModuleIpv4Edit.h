#pragma once

#include <QLineEdit>
#include <QSignalBlocker>
#include <QStringList>
#include <QValidator>
#include <functional>
#include <utility>

namespace RetroMediaLab {
class ModuleIpv4Edit final : public QLineEdit {
public:
    explicit ModuleIpv4Edit(const QString& value, QWidget* parent = nullptr) : QLineEdit(parent) {
        setLayoutDirection(Qt::LeftToRight);
        setAttribute(Qt::WA_InputMethodEnabled, false);
        setValidator(new AddressValidator(this));
        setValue(value);
        connect(this, &QLineEdit::textEdited, this, [this](const QString& candidate) {
            if (addressState(candidate) != Complete || candidate == value_) return;
            value_ = candidate;
            if (changed_) changed_();
        });
        connect(this, &QLineEdit::editingFinished, this, [this] {
            if (text() != value_) setValue(value_);
        });
    }

    QString value() const { return value_; }

    void setValue(const QString& value) {
        value_ = value;
        const QSignalBlocker blocker(this);
        setText(value_);
    }

    void onValueChanged(std::function<void()> callback) { changed_ = std::move(callback); }

private:
    enum AddressState { Invalid, Partial, Complete };
    static AddressState addressState(const QString& value) {
        constexpr int OctetCount = 4, MaxOctetDigits = 3, MaxOctet = 255;
        const auto octets = value.split(u'.');
        if (octets.size() > OctetCount) return Invalid;
        for (qsizetype index = 0; index < octets.size(); ++index) {
            const auto& octet = octets[index];
            if (octet.isEmpty()) return index == octets.size() - 1 ? Partial : Invalid;
            if (octet.size() > MaxOctetDigits || (octet.size() > 1 && octet.front() == u'0')) return Invalid;
            int number = 0;
            for (const auto digit : octet) {
                if (digit < u'0' || digit > u'9') return Invalid;
                number = number * 10 + digit.unicode() - u'0';
            }
            if (number > MaxOctet) return Invalid;
        }
        return octets.size() == OctetCount ? Complete : Partial;
    }

    class AddressValidator final : public QValidator {
    public:
        using QValidator::QValidator;
        State validate(QString& text, int&) const override {
            // Let editingFinished run for prefixes so the editor can restore its last complete address.
            return addressState(text) == ModuleIpv4Edit::Invalid ? QValidator::Invalid : Acceptable;
        }
    };

    QString value_;
    std::function<void()> changed_;
};
}
