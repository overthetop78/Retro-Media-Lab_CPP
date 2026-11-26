#include <RetroMediaLab/VideoPresentation/FullscreenOsd.h>

#include <QCloseEvent>
#include <QFrame>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QPainter>
#include <QPointer>
#include <QProgressBar>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QTimer>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <algorithm>
#include <utility>

namespace RetroMediaLab::VideoPresentation {
namespace {
constexpr int PanelWidth = 240;
constexpr int SubmenuWidth = 310;
constexpr int PanelMargin = 16;
constexpr int MainHeadingPixels = 21;
constexpr int EntryHeight = 40;
constexpr int IdleMilliseconds = 15000;
constexpr int CaptionSpacing = 4;
constexpr int EditorWidth = 420;
constexpr int EditorBottomMargin = 28;
constexpr int HitTestAlpha = 1;
constexpr auto PanelName = "rmlFullscreenOsdPanel";
constexpr auto VideoShaderBadgeName = "rmlVideoShaderBadge";
constexpr auto WindowedShaderBadgeStyle = R"(
QFrame#rmlVideoShaderBadge { background: rgba(20,22,26,190); border-radius: 8px; }
QFrame#rmlVideoShaderBadge QLabel { color: white; background: transparent; }
)";
constexpr auto FullscreenShaderBadgeStyle = R"(
QFrame#rmlVideoShaderBadge {
    background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #0b1c40,stop:1 #0f2a41);
    border: 1px solid #4da9db; border-radius: 10px;
}
QFrame#rmlVideoShaderBadge QLabel { color: white; background: transparent; font-size: 22px; }
QFrame#rmlVideoShaderBadge QProgressBar { border: 0; background: #18385b; }
QFrame#rmlVideoShaderBadge QProgressBar::chunk { background: #1bb0f8; }
)";
constexpr auto Style = R"(
QFrame#rmlFullscreenOsdPanel {
    background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 rgba(8,18,46,248),stop:1 rgba(11,27,52,242));
    border-right: 1px solid rgba(84,159,211,166);
}
QFrame#rmlFullscreenOsdPanel QLabel { color: #d5e2f6; background: transparent; }
QFrame#rmlFullscreenOsdPanel QListWidget { color: white; background: transparent; border: 0; outline: none; }
QFrame#rmlFullscreenOsdPanel QListWidget::item { padding: 0 10px 0 15px; border: 1px solid transparent; }
QFrame#rmlFullscreenOsdPanel QListWidget::item:selected {
    color: white; border: 1px solid #3dccfa; border-radius: 4px;
    background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #096eee,stop:1 #0545b8);
}
QFrame#rmlFullscreenOsdPanel QListWidget::item:disabled { color: #a1aec5; }
)";
constexpr auto SubmenuLabel = "%1  ›";
constexpr auto SettingLabel = "%1\n%2";
constexpr auto FooterKey = "<span style=\"background-color:#263957\">&nbsp;%1&nbsp;</span> ";
constexpr auto FooterLabel = "%1&nbsp;&nbsp;";
constexpr auto FooterRow = "<p style=\"margin-top:0;margin-bottom:5px;\">%1</p>";

QString hintGroup(const QStringList& keys, const QString& description) {
    if (keys.isEmpty()) return {};
    QString markup;
    for (const auto& key : keys) markup += QString::fromLatin1(FooterKey).arg(key.toHtmlEscaped());
    return markup + QString::fromLatin1(FooterLabel).arg(description.toHtmlEscaped());
}

QString hintRow(const QString& markup) {
    return markup.isEmpty() ? QString{} : QString::fromLatin1(FooterRow).arg(markup);
}
}

ShaderLoadingBadge::ShaderLoadingBadge(bool fullscreen, QWidget* parent, Qt::WindowFlags flags)
    : QFrame(parent, flags), label_(new QLabel(this)), maximumWidth_(fullscreen ? 420 : 260) {
    constexpr int WindowInsetX = 20, WindowInsetY = 16, FullscreenInsetX = 34, FullscreenInsetY = 24;
    constexpr int WindowGap = 10, FullscreenGap = 16, WindowProgressHeight = 5, FullscreenProgressHeight = 4;
    setObjectName(QLatin1String(VideoShaderBadgeName));
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setFocusPolicy(Qt::NoFocus);
    setStyleSheet(QString::fromLatin1(fullscreen ? FullscreenShaderBadgeStyle : WindowedShaderBadgeStyle));
    auto* content = new QVBoxLayout(this);
    const int insetX = fullscreen ? FullscreenInsetX : WindowInsetX;
    const int insetY = fullscreen ? FullscreenInsetY : WindowInsetY;
    content->setContentsMargins(insetX, insetY, insetX, insetY);
    content->setSpacing(fullscreen ? FullscreenGap : WindowGap);
    label_->setTextFormat(Qt::PlainText);
    label_->setAlignment(Qt::AlignCenter);
    label_->setWordWrap(true);
    auto font = label_->font();
    font.setWeight(QFont::DemiBold);
    label_->setFont(font);
    content->addWidget(label_);
    auto* progress = new QProgressBar(this);
    progress->setRange(0, 0);
    progress->setTextVisible(false);
    progress->setFixedHeight(fullscreen ? FullscreenProgressHeight : WindowProgressHeight);
    content->addWidget(progress);
}

void ShaderLoadingBadge::setCaption(const QString& caption) {
    label_->setText(caption);
}

void ShaderLoadingBadge::centerOn(const QWidget& anchor) {
    setFixedWidth(std::clamp(anchor.width(), 1, maximumWidth_));
    adjustSize();
    const QPoint offset((anchor.width() - width()) / 2, (anchor.height() - height()) / 2);
    move(isWindow() ? anchor.mapToGlobal(offset) : offset);
}

struct FullscreenOsd::State {
    struct Level {
        const std::vector<Entry>* entries;
        QFrame* panel;
        QListWidget* rows;
        const Entry* parent;
    };

    QWidget& host;
    QString footer;
    const std::vector<Entry> entries;
    std::function<void()> dismissed;
    std::vector<Level> levels;
    int submenuTop = 0;
    QTimer* idle = nullptr;
    QTimer* settingSave = nullptr;
    bool runningCommand = false;
    bool externalKeyboardNavigation = false;
    QFrame* editor = nullptr;
    ShaderLoadingBadge* shaderBadge = nullptr;
    QLabel* editorTitle = nullptr;
    QLabel* editorValue = nullptr;
    QLabel* editorHints = nullptr;
    QPushButton* editorBack = nullptr;
    QLabel* gameName = nullptr;
    QLabel* footerLabel = nullptr;
    QFrame* footerSeparator = nullptr;
    QSlider* slider = nullptr;
    const Entry* edited = nullptr;
    int draft = 0;
    bool settingPending = false;
    FooterHints hints;
};

FullscreenOsd::FullscreenOsd(QWidget& host, QString machine, QString media, QString footer,
    std::vector<Entry> entries, std::function<void()> dismissed)
    : QWidget(&host, Qt::Tool | Qt::FramelessWindowHint),
      state_(new State{host, std::move(footer), std::move(entries), std::move(dismissed)}) {
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::StrongFocus);
    setStyleSheet(QString::fromLatin1(Style));
    installEventFilter(this);
    state_->idle = new QTimer(this);
    state_->idle->setInterval(IdleMilliseconds);
    state_->idle->setSingleShot(true);
    connect(state_->idle, &QTimer::timeout, this, [this] {
        if (state_->slider->isSliderDown()) state_->idle->start();
        else close();
    });
    state_->settingSave = new QTimer(this);
    state_->settingSave->setSingleShot(true);
    connect(state_->settingSave, &QTimer::timeout, this, &FullscreenOsd::flushPendingSetting);
    state_->editor = new QFrame(this);
    state_->editor->setObjectName(QString::fromLatin1(PanelName));
    auto* editorLayout = new QVBoxLayout(state_->editor);
    editorLayout->setContentsMargins(PanelMargin, PanelMargin, PanelMargin, PanelMargin);
    state_->editorTitle = new QLabel(state_->editor);
    state_->editorTitle->setTextFormat(Qt::PlainText);
    state_->editorTitle->setWordWrap(true);
    editorLayout->addWidget(state_->editorTitle);
    state_->editorValue = new QLabel(state_->editor);
    state_->editorValue->setTextFormat(Qt::PlainText);
    editorLayout->addWidget(state_->editorValue);
    state_->slider = new QSlider(Qt::Horizontal, state_->editor);
    state_->slider->setFocusPolicy(Qt::NoFocus);
    editorLayout->addWidget(state_->slider);
    state_->slider->installEventFilter(this);
    state_->editorHints = new QLabel(state_->editor);
    state_->editorHints->setTextFormat(Qt::RichText);
    state_->editorHints->setWordWrap(true);
    editorLayout->addWidget(state_->editorHints);
    state_->editorBack = new QPushButton(state_->editor);
    state_->editorBack->setFocusPolicy(Qt::NoFocus);
    editorLayout->addWidget(state_->editorBack, 0, Qt::AlignLeft);
    connect(state_->editorBack, &QPushButton::clicked, this, [this] { navigate(Navigation::Cancel); });
    state_->editor->installEventFilter(this);
    state_->editor->hide();
    connect(state_->slider, &QSlider::valueChanged, this, &FullscreenOsd::adjustValue);
    openLevel(state_->entries, machine, media);
    host.installEventFilter(this);
    fitHost();
}

FullscreenOsd::~FullscreenOsd() = default;

void FullscreenOsd::activate() {
    fitHost();
    show();
    raise();
    activateWindow();
    state_->levels.back().rows->setFocus();
    state_->idle->start();
}

void FullscreenOsd::setGameName(const QString& name) {
    if (state_->gameName->text() == name) return;
    state_->gameName->setText(name);
    state_->gameName->setVisible(!name.isEmpty());
}

void FullscreenOsd::setShaderLoading(bool loading, const QString& caption) {
    if (!loading) {
        if (state_->shaderBadge) state_->shaderBadge->hide();
        return;
    }
    if (!state_->shaderBadge) state_->shaderBadge = new ShaderLoadingBadge(true, this);
    state_->shaderBadge->setCaption(caption);
    state_->shaderBadge->centerOn(*this);
    state_->shaderBadge->show();
    state_->shaderBadge->raise();
    if (state_->edited) state_->editor->raise();
}

void FullscreenOsd::setFooterHints(const FooterHints& hints) {
    state_->hints = hints;
    const auto content = hintRow(hintGroup(hints.vertical + hints.horizontal, hints.navigateLabel)) +
        hintRow(hintGroup(hints.confirm, hints.confirmLabel) + hintGroup(hints.back, hints.backLabel));
    state_->footerLabel->setTextFormat(Qt::RichText);
    state_->footerLabel->setText(content);
    state_->footerLabel->setVisible(!content.isEmpty());
    state_->footerSeparator->setVisible(!content.isEmpty());
    refreshEditorHints();
}

void FullscreenOsd::openLevel(const std::vector<Entry>& entries, const QString& title, const QString& subtitle,
    const Entry* parent) {
    auto* panel = new QFrame(this);
    panel->setObjectName(QString::fromLatin1(PanelName));
    auto* content = new QVBoxLayout(panel);
    content->setContentsMargins(0, PanelMargin, 0, PanelMargin);
    const auto addCaption = [&](const QString& caption) {
        auto* label = new QLabel(caption, panel);
        label->setTextFormat(Qt::PlainText);
        label->setWordWrap(true);
        label->setContentsMargins(PanelMargin, CaptionSpacing, PanelMargin, CaptionSpacing);
        content->addWidget(label);
        return label;
    };
    auto* header = addCaption(title);
    auto font = header->font();
    font.setPixelSize(MainHeadingPixels);
    font.setWeight(QFont::DemiBold);
    header->setFont(font);
    if (state_->levels.empty()) {
        state_->gameName = addCaption(subtitle);
        state_->gameName->setVisible(!subtitle.isEmpty());
    }
    auto* rows = new QListWidget(panel);
    rows->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    rows->setTextElideMode(Qt::ElideRight);
    for (const auto& entry : entries) {
        if (entry.separatorBefore) {
            auto* separator = new QListWidgetItem(rows);
            separator->setFlags(Qt::NoItemFlags);
            auto* line = new QFrame(rows);
            line->setFrameShape(QFrame::HLine);
            rows->setItemWidget(separator, line);
        }
        auto* row = new QListWidgetItem(entry.icon,
            entry.children.empty() ? entry.label : QString::fromUtf8(SubmenuLabel).arg(entry.label), rows);
        row->setSizeHint({0, EntryHeight});
        row->setData(Qt::UserRole, entry.setting.has_value());
    }
    content->addWidget(rows, 1);
    if (state_->levels.empty()) {
        state_->footerSeparator = new QFrame(panel);
        state_->footerSeparator->setFrameShape(QFrame::HLine);
        content->addWidget(state_->footerSeparator);
        state_->footerLabel = addCaption(state_->footer);
        state_->footerLabel->setVisible(!state_->footer.isEmpty());
        state_->footerSeparator->setVisible(!state_->footer.isEmpty());
    }
    const auto level = state_->levels.size();
    state_->levels.push_back({&entries, panel, rows, parent});
    rows->installEventFilter(this);
    rows->viewport()->installEventFilter(this);
    rows->viewport()->setMouseTracking(true);
    connect(rows, &QListWidget::itemClicked, this, [this, level, rows](QListWidgetItem* item) {
        invoke(level, rows->row(item), false);
    });
    refresh();
    state_->levels.back().rows->setFocus();
}

void FullscreenOsd::retainLevels(std::size_t count) {
    flushPendingSetting();
    restoreMenu(count);
}

void FullscreenOsd::restoreMenu(std::size_t count) {
    state_->edited = nullptr;
    state_->editor->hide();
    while (state_->levels.size() > count) {
        auto* panel = state_->levels.back().panel;
        state_->levels.pop_back();
        panel->hide();
        panel->deleteLater();
    }
    positionPanels();
    state_->levels.back().rows->setFocus();
}

void FullscreenOsd::positionPanels() {
    if (state_->levels.empty()) return;
    auto& root = state_->levels.front();
    const bool hasSubmenu = state_->levels.size() > 1;
    const int rootWidth = std::min(PanelWidth,
        hasSubmenu ? width() * PanelWidth / (PanelWidth + SubmenuWidth) : width());
    root.panel->setGeometry(0, 0, rootWidth, height());
    root.panel->setVisible(!state_->edited);
    for (std::size_t index = 1; index < state_->levels.size(); ++index) {
        auto& level = state_->levels[index];
        const bool current = index + 1 == state_->levels.size() && !state_->edited;
        level.panel->setVisible(current);
        if (!current) continue;
        const int top = std::clamp(state_->submenuTop, 0, std::max(0, height() - EntryHeight));
        const int available = height() - top;
        int panelHeight = available;
        if (!level.parent->fullHeight) {
            int rowsHeight = 0;
            for (int row = 0; row < level.rows->count(); ++row) {
                if (!level.rows->item(row)->isHidden())
                    rowsHeight += std::max(0, level.rows->sizeHintForRow(row));
            }
            const auto* layout = level.panel->layout();
            const int chrome = layout->sizeHint().height() - level.rows->sizeHint().height();
            panelHeight = std::min(available, chrome + rowsHeight);
        }
        level.panel->setGeometry(rootWidth, top, std::min(SubmenuWidth, width() - rootWidth), panelHeight);
    }
}

void FullscreenOsd::refresh() {
    auto validLevels = state_->levels.size();
    bool editedVisible = false;
    for (std::size_t depth = 0; depth < state_->levels.size(); ++depth) {
        const auto& level = state_->levels[depth];
        const auto* child = depth + 1 < state_->levels.size() ? state_->levels[depth + 1].parent : nullptr;
        bool childVisible = false;
        const int previous = level.rows->currentRow();
        int next = -1, before = -1;
        int index = 0;
        for (const auto& entry : *level.entries) {
            const bool visible = !entry.visible || entry.visible();
            if (entry.separatorBefore) level.rows->item(index++)->setHidden(!visible);
            auto* item = level.rows->item(index);
            const bool enabled = !entry.enabled || entry.enabled();
            item->setHidden(!visible);
            item->setFlags(enabled ? Qt::ItemIsEnabled | Qt::ItemIsSelectable : Qt::NoItemFlags);
            if (visible && enabled) {
                if (index < previous) before = index;
                else if (next == -1) next = index;
            }
            if (&entry == child) childVisible = visible;
            if (&entry == state_->edited) editedVisible = visible;
            ++index;
            if (entry.checked) item->setCheckState(entry.checked() ? Qt::Checked : Qt::Unchecked);
            if (entry.setting)
                item->setText(QString::fromLatin1(SettingLabel).arg(entry.label,
                    entry.setting->format(&entry == state_->edited ? state_->draft : entry.setting->read())));
            else if (entry.status)
                item->setText(QString::fromLatin1(SettingLabel).arg(entry.label, entry.status()));
        }
        const int selected = next == -1 ? before : next;
        if (selected != level.rows->currentRow()) level.rows->setCurrentRow(selected);
        if (child && !childVisible) {
            validLevels = depth + 1;
            break;
        }
    }
    if (!state_->settingPending &&
        (validLevels < state_->levels.size() || (state_->edited && !editedVisible))) {
        restoreMenu(validLevels);
        return;
    }
    positionPanels();
    refreshEditorHints();
}

void FullscreenOsd::invoke(std::size_t levelIndex, int row, bool enterOnly) {
    if (state_->runningCommand || row < 0 || levelIndex >= state_->levels.size()) return;
    const auto& level = state_->levels[levelIndex];
    auto* item = level.rows->item(row);
    if (!item || item->isHidden() || !item->flags().testFlag(Qt::ItemIsEnabled)) return;
    for (const auto& entry : *level.entries) {
        if (entry.separatorBefore && row-- == 0) return;
        if (row-- != 0) continue;
        if (entry.children.empty() && (enterOnly || (!entry.command && !entry.setting))) return;
        retainLevels(levelIndex + 1);
        if (!entry.children.empty()) {
            if (levelIndex == 0) {
                state_->submenuTop = entry.fullHeight ? 0 : level.rows->viewport()->mapTo(
                    this, level.rows->visualItemRect(item).topLeft()).y();
            }
            openLevel(entry.children, entry.label, {}, &entry);
        } else if (entry.setting) {
            showEditor(entry);
        } else {
            state_->runningCommand = true;
            state_->idle->stop();
            const QPointer<FullscreenOsd> alive(this);
            const auto command = entry.command;
            command();
            if (alive && isVisible()) {
                state_->runningCommand = false;
                refresh();
                state_->idle->start();
                state_->levels.back().rows->setFocus();
            }
        }
        return;
    }
}

void FullscreenOsd::positionEditor() {
    auto* editor = state_->editor;
    editor->setFixedWidth(std::min(EditorWidth, std::max(1, width() - 2 * PanelMargin)));
    editor->adjustSize();
    editor->move((width() - editor->width()) / 2,
        std::max(0, height() - editor->height() - EditorBottomMargin));
}

void FullscreenOsd::showEditor(const Entry& entry) {
    state_->edited = &entry;
    const auto& setting = *entry.setting;
    const QSignalBlocker blocker(state_->slider);
    state_->draft = setting.read();
    state_->editorTitle->setText(entry.label);
    state_->editorValue->setText(setting.format(state_->draft));
    state_->slider->setRange(setting.minimum, setting.maximum);
    state_->slider->setValue(state_->draft);
    state_->slider->setVisible(setting.slider);
    positionPanels();
    setFocus();
    state_->editor->show();
    refreshEditorHints();
    state_->editor->raise();
}

void FullscreenOsd::refreshEditorHints() {
    if (!state_->edited) return;
    const auto& hints = state_->hints;
    const auto& entries = *state_->levels.back().entries;
    const auto editable = std::ranges::count_if(entries, [](const Entry& entry) {
        return entry.setting && (!entry.visible || entry.visible()) && (!entry.enabled || entry.enabled());
    });
    auto markup = hintGroup(hints.horizontal, hints.adjustLabel);
    if (editable > 1) markup += hintGroup(hints.vertical, hints.navigateLabel);
    state_->editorHints->setText(hintRow(markup));
    state_->editorHints->setVisible(!markup.isEmpty());
    auto keys = hints.confirm + hints.back;
    keys.removeDuplicates();
    keys.append(hints.backLabel);
    state_->editorBack->setText(keys.join(u' '));
    state_->editorBack->setVisible(!hints.backLabel.isEmpty());
    positionEditor();
}

void FullscreenOsd::adjustValue(int value) {
    if (!state_->edited) return;
    state_->idle->start();
    const auto& setting = *state_->edited->setting;
    value = std::clamp(value, setting.minimum, setting.maximum);
    if (state_->draft == value) return;
    state_->draft = value;
    if (setting.preview) setting.preview(value);
    state_->editorValue->setText(setting.format(value));
    const QSignalBlocker blocker(state_->slider);
    state_->slider->setValue(value);
    state_->settingPending = true;
    if (!state_->settingSave->isActive()) state_->settingSave->start();
    refresh();
}

void FullscreenOsd::flushPendingSetting() {
    if (!state_->edited || !std::exchange(state_->settingPending, false)) return;
    state_->settingSave->stop();
    const auto* entry = state_->edited;
    const auto& setting = *entry->setting;
    const int value = state_->draft;
    const QPointer<FullscreenOsd> alive(this);
    if (setting.read() != value) setting.save(value);
    if (!alive || state_->edited != entry) return;
    const int current = setting.read();
    state_->draft = current;
    if (current != value && setting.preview) setting.preview(current);
    state_->editorValue->setText(setting.format(current));
    const QSignalBlocker blocker(state_->slider);
    state_->slider->setValue(current);
    refresh();
}

void FullscreenOsd::editAdjacent(int direction) {
    flushPendingSetting();
    if (!state_->edited) return;
    const auto index = state_->levels.size() - 1;
    const auto& level = state_->levels.back();
    auto* rows = level.rows;
    const int previous = rows->currentRow();
    int row = previous + direction;
    while (row >= 0 && row < rows->count()) {
        auto* item = rows->item(row);
        if (item->data(Qt::UserRole).toBool() && !item->isHidden() && item->flags().testFlag(Qt::ItemIsEnabled)) {
            rows->setCurrentRow(row);
            invoke(index, row, false);
            return;
        }
        row += direction;
    }
}

void FullscreenOsd::navigate(Navigation direction) {
    if (!isVisible() || state_->runningCommand || state_->levels.empty()) return;
    state_->idle->start();
    const auto index = state_->levels.size() - 1;
    auto* rows = state_->levels.back().rows;
    if (state_->edited) {
        switch (direction) {
        case Navigation::Left:
        case Navigation::Right:
            adjustValue(state_->draft + (direction == Navigation::Left ? -1 : 1));
            return;
        case Navigation::Up:
        case Navigation::Down:
            editAdjacent(direction == Navigation::Up ? -1 : 1);
            return;
        default:
            retainLevels(state_->levels.size());
            return;
        }
    }
    switch (direction) {
    case Navigation::Cancel:
        if (index == 0) close();
        else retainLevels(index);
        return;
    case Navigation::Left:
        if (index != 0) retainLevels(index);
        return;
    case Navigation::Right:
    case Navigation::Confirm:
        invoke(index, rows->currentRow(), direction == Navigation::Right);
        return;
    case Navigation::Up:
    case Navigation::Down:
        const int step = direction == Navigation::Up ? -1 : 1;
        for (int row = rows->currentRow() + step; row >= 0 && row < rows->count(); row += step) {
            const auto* item = rows->item(row);
            if (!item->isHidden() && item->flags().testFlag(Qt::ItemIsEnabled)) {
                rows->setCurrentRow(row);
                break;
            }
        }
        return;
    }
}

bool FullscreenOsd::isEditing() const noexcept { return state_->edited != nullptr; }

void FullscreenOsd::setExternalKeyboardNavigation(bool enabled) noexcept {
    state_->externalKeyboardNavigation = enabled;
}

bool FullscreenOsd::eventFilter(QObject* object, QEvent* event) {
    if (object == &state_->host) {
        if (event->type() == QEvent::Move || event->type() == QEvent::Resize) fitHost();
        return false;
    }
    if (!isVisible()) return false;
    if (event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::Wheel || event->type() == QEvent::KeyPress) {
        if (!state_->runningCommand) state_->idle->start();
    }
    if (object == state_->editor && event->type() == QEvent::Wheel && state_->edited) {
        const auto delta = static_cast<QWheelEvent*>(event)->angleDelta().y();
        if (delta != 0) adjustValue(state_->draft + (delta < 0 ? -1 : 1));
        return true;
    }
    if (event->type() == QEvent::KeyPress) {
        const auto navigateDefault = [this](Navigation direction) {
            if (!state_->externalKeyboardNavigation) navigate(direction);
            return true;
        };
        switch (static_cast<QKeyEvent*>(event)->key()) {
        case Qt::Key_Up: return navigateDefault(Navigation::Up);
        case Qt::Key_Down: return navigateDefault(Navigation::Down);
        case Qt::Key_Left: return navigateDefault(Navigation::Left);
        case Qt::Key_Right: return navigateDefault(Navigation::Right);
        case Qt::Key_Escape: return navigateDefault(Navigation::Cancel);
        case Qt::Key_Return:
        case Qt::Key_Enter: return navigateDefault(Navigation::Confirm);
        default: break;
        }
    }
    return false;
}

void FullscreenOsd::fitHost() {
    setGeometry(QRect(state_->host.mapToGlobal(QPoint{}), state_->host.size()));
    positionPanels();
    if (state_->shaderBadge && !state_->shaderBadge->isHidden()) state_->shaderBadge->centerOn(*this);
    if (state_->edited) positionEditor();
}

void FullscreenOsd::closeEvent(QCloseEvent* event) {
    flushPendingSetting();
    state_->idle->stop();
    hide();
    state_->host.activateWindow();
    if (auto dismissed = std::exchange(state_->dismissed, {})) dismissed();
    event->accept();
}

void FullscreenOsd::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(rect(), QColor(0, 0, 0, HitTestAlpha));
}

void FullscreenOsd::mousePressEvent(QMouseEvent* event) {
    event->accept();
    close();
}
}
