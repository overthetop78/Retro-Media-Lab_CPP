#include "EmulationNetworkPanel.h"

#include <RetroMediaLab/Emulation/Networks.h>
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QAbstractTableModel>
#include <QHeaderView>
#include <QLabel>
#include <QLocale>
#include <QPainter>
#include <QStringList>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QVBoxLayout>
#include <algorithm>
#include <array>

namespace RetroMediaLab {
namespace {
using namespace Emulation;
constexpr auto DescriptionKey = "Emulation.Network.Description";
constexpr QLatin1String ValueSeparator(" \xB7 ");
constexpr QLatin1String TypedSeparator(": ");
constexpr int OuterMargin = 12, RowHeight = 36, FontPixels = 13, CellInset = 12, HeaderInset = 10;
enum Column { Running, Machine, Interface, Network, Protocol, Format, Transport, Ip, Port, Address, Relay, Endpoint };
constexpr std::pair<const char*, int> Columns[] = {
    {"Emulation.Network.Running", 90},
    {"Emulation.Network.Machine", 160},
    {"Emulation.Network.Interface", 100},
    {"Emulation.Network.Network", 100},
    {"Emulation.Network.Protocol", 140},
    {"Emulation.Network.Format", 130},
    {"Emulation.Network.Transport", 100},
    {"Emulation.Network.Ip", 140},
    {"Emulation.Network.Port", 80},
    {"Emulation.Network.Address", 170},
    {"Emulation.Network.Relay", 130},
    {"Emulation.Network.Endpoint", 160},
};

class NetworkTable final : public QAbstractTableModel {
public:
    NetworkTable(QObject* parent, Translations::TranslationService& translations)
        : QAbstractTableModel(parent), translations_(translations) {}

    int rowCount(const QModelIndex& parent = {}) const override { return parent.isValid() ? 0 : int(rows_.size()); }
    int columnCount(const QModelIndex& parent = {}) const override { return parent.isValid() ? 0 : int(std::size(Columns)); }
    Qt::ItemFlags flags(const QModelIndex& index) const override {
        return index.isValid() ? Qt::ItemIsEnabled : Qt::NoItemFlags;
    }
    QVariant headerData(int column, Qt::Orientation orientation, int role) const override {
        if (orientation == Qt::Horizontal && column >= 0 && column < columnCount() &&
            (role == Qt::DisplayRole || role == Qt::ToolTipRole))
            return QString::fromStdString(translations_.text(Columns[column].first));
        return {};
    }
    QVariant data(const QModelIndex& index, int role) const override {
        if (!index.isValid()) return {};
        const auto& row = rows_.at(index.row());
        if (index.column() == Running)
            return role == Qt::CheckStateRole ? QVariant(row.running ? Qt::Checked : Qt::Unchecked) : QVariant{};
        if (role == Qt::DisplayRole || role == Qt::ToolTipRole) return row.values[index.column()];
        return {};
    }

    void replace(const QList<NetworkEndpoint>& endpoints) {
        QList<Row> next;
        next.reserve(endpoints.size());
        QLocale locale(QString::fromStdString(translations_.language()));
        locale.setNumberOptions(QLocale::OmitGroupSeparator);
        for (const auto& value : endpoints) {
            Row row;
            row.running = value.isRunning;
            row.values[Machine] = value.machineId;
            row.values[Interface] = display(value.interfaceType);
            if (row.values[Interface] != value.interfaceId)
                row.values[Interface] += ValueSeparator + value.interfaceId;
            row.values[Network] = display(value.network);
            row.values[Protocol] = join(value.protocols, [this](auto protocol) { return display(protocol); });
            row.values[Format] = display(value.transferFormat);
            row.values[Transport] = display(value.transport);
            if (value.ipAddress) row.values[Ip] = *value.ipAddress;
            if (value.port) row.values[Port] = locale.toString(*value.port);
            row.values[Address] = join(value.addresses, [this](const auto& address) {
                return display(address.type) + TypedSeparator + address.value;
            });
            row.values[Relay] = join(value.relays, [this](const auto& relay) {
                return display(relay.type) + TypedSeparator + relay.target;
            });
            if (value.hostEndpoint) row.values[Endpoint] = *value.hostEndpoint;
            next.append(std::move(row));
        }
        beginResetModel();
        rows_ = std::move(next);
        endResetModel();
        Q_EMIT headerDataChanged(Qt::Horizontal, 0, columnCount() - 1);
    }

private:
    struct Row {
        bool running = false;
        std::array<QString, std::size(Columns)> values;
    };
    template<class Enum>
    QString display(Enum value) const {
        return QString::fromStdString(translations_.text(networkResourceKey(value).toStdString()));
    }
    template<class Values, class Formatter>
    static QString join(const Values& values, Formatter format) {
        QStringList labels;
        labels.reserve(values.size());
        for (const auto& value : values) labels.append(format(value));
        return labels.join(ValueSeparator);
    }

    Translations::TranslationService& translations_;
    QList<Row> rows_;
};

class NetworkHeader final : public QHeaderView {
public:
    explicit NetworkHeader(QWidget* parent) : QHeaderView(Qt::Horizontal, parent) {
        auto headerFont = font();
        headerFont.setWeight(QFont::DemiBold);
        setFont(headerFont);
        setSectionResizeMode(QHeaderView::Fixed);
        setSectionsClickable(false);
        setSectionsMovable(false);
    }

protected:
    QSize sectionSizeFromContents(int column) const override {
        const auto width = Columns[column].second;
        const auto text = model()->headerData(column, Qt::Horizontal).toString();
        const auto height = fontMetrics().boundingRect(QRect(0, 0, width - 2 * CellInset, 0),
            Qt::TextWordWrap, text).height();
        return {width, height + 2 * HeaderInset};
    }

    void paintSection(QPainter* painter, const QRect& rect, int column) const override {
        QStyleOptionHeader section;
        initStyleOption(&section);
        initStyleOptionForIndex(&section, column);
        const auto text = model()->headerData(column, Qt::Horizontal).toString();
        section.text.clear();
        section.rect = rect;
        style()->drawControl(QStyle::CE_Header, &section, painter, this);
        painter->save();
        painter->setClipRect(rect);
        painter->setFont(font());
        painter->setPen(palette().buttonText().color());
        painter->drawText(rect.adjusted(CellInset, 0, -CellInset, 0),
            Qt::AlignLeading | Qt::AlignVCenter | Qt::TextWordWrap, text);
        painter->restore();
    }
};

class NetworkCell final : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        if (index.column() != Running) {
            auto textOption = option;
            textOption.rect.adjust(CellInset, 0, -CellInset, 0);
            QStyledItemDelegate::paint(painter, textOption, index);
            return;
        }
        QStyleOptionButton state;
        state.rect = option.rect;
        state.palette = option.palette;
        state.state = QStyle::State_Enabled |
            (index.data(Qt::CheckStateRole).toInt() == Qt::Checked ? QStyle::State_On : QStyle::State_Off);
        const auto* widget = option.widget;
        const auto* style = widget->style();
        const auto size = style->subElementRect(QStyle::SE_CheckBoxIndicator, &state, widget).size();
        state.rect = QStyle::alignedRect(option.direction, Qt::AlignCenter, size, option.rect);
        style->drawPrimitive(QStyle::PE_IndicatorCheckBox, &state, painter, widget);
    }
};

class NetworkPanel final : public QWidget {
public:
    NetworkPanel(QWidget* parent, Translations::TranslationService& translations)
        : QWidget(parent), translations_(translations), model_(new NetworkTable(this, translations)) {
        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(OuterMargin, OuterMargin, OuterMargin, OuterMargin);
        description_ = new QLabel(this);
        description_->setWordWrap(true);
        layout->addWidget(description_);

        auto* table = new QTableView(this);
        auto tableFont = table->font();
        tableFont.setPixelSize(FontPixels);
        table->setFont(tableFont);
        table->setModel(model_);
        table->setHorizontalHeader(new NetworkHeader(table));
        for (int index = 0; index < model_->columnCount(); ++index)
            table->setColumnWidth(index, Columns[index].second);
        table->verticalHeader()->hide();
        table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        table->verticalHeader()->setDefaultSectionSize(RowHeight);
        table->setItemDelegate(new NetworkCell(table));
        table->setFocusPolicy(Qt::NoFocus);
        table->setSelectionMode(QAbstractItemView::NoSelection);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setShowGrid(false);
        table->setWordWrap(false);
        layout->addWidget(table, 1);

        subscription_ = translations_.subscribe([this] { localize(); });
        localize();
    }

protected:
    void showEvent(QShowEvent* event) override {
        QWidget::showEvent(event);
        connection_ = connect(&networks(), &NetworkRegistry::changed, this, [this] {
            if (isVisible() && version_ != networks().version()) refresh();
        }, Qt::QueuedConnection);
        refresh();
    }
    void hideEvent(QHideEvent* event) override {
        disconnect(connection_);
        model_->replace({});
        QWidget::hideEvent(event);
    }

private:
    void localize() {
        description_->setText(QString::fromStdString(translations_.text(DescriptionKey)));
        if (isVisible()) refresh();
        else model_->replace({});
    }
    void refresh() {
        version_ = networks().version();
        model_->replace(networks().endpoints());
    }

    Translations::TranslationService& translations_;
    NetworkTable* model_;
    QLabel* description_;
    quint64 version_ = 0;
    QMetaObject::Connection connection_;
    Translations::TranslationService::Subscription subscription_;
};
}

QWidget* createEmulationNetworkPanel(QWidget* parent, Translations::TranslationService& translations) {
    return new NetworkPanel(parent, translations);
}
}
