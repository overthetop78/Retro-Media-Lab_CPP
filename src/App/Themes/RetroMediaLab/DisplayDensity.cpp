#include "Shared/Controls/HexGridConstants.h"
#include "DisplayDensity.h"
#include "Shared/Presentation/DisplayFactor.h"
#include "ThemeConstants.h"
#include <QScopedValueRollback>
#include <Constants/DisplayConstants.h>
#include <QApplication>
#include <QEvent>
#include <QScreen>
#include <QWindow>
#include <QTimer>
#include <QPointer>
#include <QLayout>
#include <QGridLayout>
#include <QAbstractButton>
#include <QComboBox>
#include <QAbstractItemView>
#include <QTabWidget>
#include <QTabBar>
#include <QSplitter>
#include <QHeaderView>
#include <QListWidget>
#include <QVariant>
#include <algorithm>
namespace RetroMediaLab {
using namespace DisplayConstants;
using namespace ThemeConstants::Density;
namespace {
int metric(int value, qreal scale) { return value <= 0 || value >= UnboundedSize ? value : qMax(1, qRound(value * scale)); }
QSize size(QSize value, qreal scale) { return {metric(value.width(), scale), metric(value.height(), scale)}; }
QFont scaledFont(QFont font, qreal scale) {
    if (font.pixelSize() > 0) font.setPixelSize(metric(font.pixelSize(), scale));
    else font.setPointSizeF(qMax(Unit, font.pointSizeF() * scale));
    return font;
}
struct WidgetBaseline {
    QPointer<QWidget> widget;
    QSize minimum, maximum, icon;
    QFont font;
    int handle = 0, section = 0, minimumSection = 0;
    QVector<QSize> items;
};
struct LayoutBaseline {
    QPointer<QLayout> layout;
    QMargins margins;
    int spacing, horizontal, vertical;
};
}
struct DisplayDensity::Data {
    QWidget& root;
    WorkspaceState& state;
    WorkspaceState::DirectorySubscription subscription;
    QVector<WidgetBaseline> widgets;
    QVector<LayoutBaseline> layouts;
    bool scheduled = false, applying = false;
    bool belongs(QWidget* widget) const {
        for (auto* ancestor = widget; ancestor; ancestor = ancestor->parentWidget()) if (ancestor == &root) return true;
        return false;
    }
};
qreal DisplayDensity::factorFor(const QSize&, int percent) {
    return qreal(percent) / DefaultPercent;
}
qreal DisplayDensity::factor(const QWidget* widget) {
    return displayFactor(widget);
}
qreal DisplayDensity::tabFactor(const QWidget* widget) {
    if (!qobject_cast<const QTabBar*>(widget)) return Unit;
    int levels = 0;
    bool workspace = false;
    for (auto* parent = widget->parentWidget(); parent; parent = parent->parentWidget()) {
        if (qobject_cast<const QTabWidget*>(parent)) {
            ++levels;
            if (parent->objectName() == ThemeConstants::WorkspaceTabs) workspace = true;
        }
    }
    if (levels > 1) return NestedTabFactor;
    return workspace ? Unit : SecondaryTabFactor;
}
DisplayDensity::DisplayDensity(QWidget& root, WorkspaceState& state)
    : QObject(&root), data_(std::make_unique<Data>(root, state)) {
    data_->subscription = state.subscribeToInterfaceScale([this] { schedule(); });
    const auto watchScreen = [this](QScreen* screen) {
        connect(screen, &QScreen::availableGeometryChanged, this, [this] { schedule(); });
        connect(screen, &QScreen::logicalDotsPerInchChanged, this, [this] { schedule(); });
        connect(screen, &QScreen::physicalDotsPerInchChanged, this, [this] { schedule(); });
    };
    for (auto* screen : QGuiApplication::screens()) watchScreen(screen);
    connect(qApp, &QGuiApplication::screenAdded, this, watchScreen);
    qApp->installEventFilter(this);
    schedule();
}
DisplayDensity::~DisplayDensity() { if (qApp) qApp->removeEventFilter(this); }
bool DisplayDensity::eventFilter(QObject* object, QEvent* event) {
    if (data_->applying) return false;
    auto* widget = qobject_cast<QWidget*>(object);
    if (!widget || !data_->belongs(widget)) return false;
    if (event->type() == QEvent::Show || event->type() == QEvent::ChildPolished || event->type() == QEvent::ScreenChangeInternal)
        schedule();
    return false;
}
void DisplayDensity::schedule() {
    if (data_->scheduled) return;
    data_->scheduled = true;
    QTimer::singleShot(0, this, [this] { data_->scheduled = false; refresh(); });
}
void DisplayDensity::refresh() {
    const QScopedValueRollback guard(data_->applying, true);
    data_->widgets.removeIf( [](const auto& record) { return record.widget.isNull(); });
    data_->layouts.removeIf( [](const auto& record) { return record.layout.isNull(); });
    auto widgets = data_->root.findChildren<QWidget*>();
    widgets.prepend(&data_->root);
    // Capture the whole unmodified tree before changing inherited fonts or geometry constraints.
    for (auto* widget : widgets) {
        if (!data_->belongs(widget)) continue;
        if (std::none_of(data_->widgets.cbegin(), data_->widgets.cend(), [widget](const auto& record) { return record.widget == widget; })) {
            WidgetBaseline record{widget, widget->minimumSize(), widget->maximumSize(), {}, widget->font()};
            if (const auto* parent = widget->parentWidget()) {
                qreal inherited = 0;
                for (const auto* ancestor = parent; ancestor; ancestor = ancestor->parentWidget()) {
                    inherited = ancestor->property(AppliedProperty).toReal();
                    if (inherited > 0 || ancestor->isWindow()) break;
                }
                if (inherited > 0 && widget->font().pixelSize() == parent->font().pixelSize() && widget->font().pointSizeF() == parent->font().pointSizeF())
                    record.font = scaledFont(record.font, Unit / inherited);
            }
            if (auto* button = qobject_cast<QAbstractButton*>(widget)) record.icon = button->iconSize();
            if (auto* combo = qobject_cast<QComboBox*>(widget)) record.icon = combo->iconSize();
            if (auto* tabs = qobject_cast<QTabWidget*>(widget)) record.icon = tabs->iconSize();
            if (auto* tabs = qobject_cast<QTabBar*>(widget)) record.icon = tabs->iconSize();
            if (auto* view = qobject_cast<QAbstractItemView*>(widget)) record.icon = view->iconSize();
            if (auto* splitter = qobject_cast<QSplitter*>(widget)) record.handle = splitter->handleWidth();
            if (auto* header = qobject_cast<QHeaderView*>(widget)) { record.section = header->defaultSectionSize(); record.minimumSection = header->minimumSectionSize(); }
            if (auto* list = qobject_cast<QListWidget*>(widget)) for (int i = 0; i < list->count(); ++i) record.items.push_back(list->item(i)->sizeHint());
            data_->widgets.push_back(std::move(record));
        }
        const auto layouts = widget->findChildren<QLayout*>(QString(), Qt::FindDirectChildrenOnly);
        const auto capture = [this](auto&& self, QLayout* layout) -> void {
            if (std::none_of(data_->layouts.cbegin(), data_->layouts.cend(), [layout](const auto& record) { return record.layout == layout; })) {
                auto* grid = qobject_cast<QGridLayout*>(layout);
                data_->layouts.push_back({layout, layout->contentsMargins(), layout->spacing(), grid ? grid->horizontalSpacing() : layout->spacing(), grid ? grid->verticalSpacing() : layout->spacing()});
            }
            for (int i = 0; i < layout->count(); ++i) if (auto* child = layout->itemAt(i)->layout()) self(self, child);
        };
        for (auto* layout : layouts) capture(capture, layout);
    }
    for (const auto& record : data_->widgets) {
        auto* widget = record.widget.data();
        if (!widget) continue;
        const auto scale = factorFor(widget->window()->screen()->availableGeometry().size(), data_->state.interfaceScale());
        widget->window()->setProperty(FactorProperty, scale);
        widget->setFont(scaledFont(record.font, scale * tabFactor(widget)));
        if (!widget->isWindow()) {
            auto minimum = size(record.minimum, scale), maximum = size(record.maximum, scale);
            if (widget->property(ContentHeightProperty).toBool()) {
                minimum.setHeight(widget->minimumHeight()); maximum.setHeight(widget->maximumHeight());
            }
            widget->setMinimumSize(minimum);
            widget->setMaximumSize(maximum);
        }
        if (auto* button = qobject_cast<QAbstractButton*>(widget); button && record.icon.isValid()) button->setIconSize(size(record.icon, scale));
        if (auto* combo = qobject_cast<QComboBox*>(widget); combo && record.icon.isValid()) {
            combo->setIconSize(size(record.icon, scale));
            // FontChange only clears sizeHint; StyleChange also clears minimumSizeHint.
            QEvent metricsChanged(QEvent::StyleChange);
            QApplication::sendEvent(combo, &metricsChanged);
        }
        if (auto* tabs = qobject_cast<QTabWidget*>(widget); tabs && record.icon.isValid()) tabs->setIconSize(size(record.icon, scale));
        if (auto* tabs = qobject_cast<QTabBar*>(widget); tabs && record.icon.isValid()) tabs->setIconSize(size(record.icon, scale * tabFactor(widget)));
        if (auto* view = qobject_cast<QAbstractItemView*>(widget); view && record.icon.isValid()) view->setIconSize(size(record.icon, scale));
        if (auto* splitter = qobject_cast<QSplitter*>(widget)) splitter->setHandleWidth(metric(record.handle, scale));
        if (auto* header = qobject_cast<QHeaderView*>(widget); header && !header->property(HexGridConstants::OwnSectionMetricsProperty).toBool()) { header->setMinimumSectionSize(metric(record.minimumSection, scale)); header->setDefaultSectionSize(metric(record.section, scale)); }
        if (auto* list = qobject_cast<QListWidget*>(widget)) for (int i = 0; i < qMin(list->count(), record.items.size()); ++i) if (record.items[i].isValid()) list->item(i)->setSizeHint(size(record.items[i], scale));
        widget->setProperty(AppliedProperty, scale);
        widget->updateGeometry();
        widget->update();
    }
    for (const auto& record : data_->layouts) {
        auto* layout = record.layout.data();
        if (!layout || !layout->parentWidget()) continue;
        const auto scale = factor(layout->parentWidget());
        const auto m = record.margins;
        layout->setContentsMargins(metric(m.left(), scale), metric(m.top(), scale), metric(m.right(), scale), metric(m.bottom(), scale));
        layout->setSpacing(metric(record.spacing, scale));
        if (auto* grid = qobject_cast<QGridLayout*>(layout)) { grid->setHorizontalSpacing(metric(record.horizontal, scale)); grid->setVerticalSpacing(metric(record.vertical, scale)); }
        layout->invalidate();
    }

}
}
