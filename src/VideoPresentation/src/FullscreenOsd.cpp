#include <RetroMediaLab/VideoPresentation/FullscreenOsd.h>

#include <QApplication>
#include <QCloseEvent>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QScrollArea>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QProgressBar>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QStyleOptionSlider>
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
constexpr int InformationMargin = 24;
constexpr int CenterHeaderIconSize = 24;
constexpr int CenterHeaderIconGap = 12;
constexpr int MixerCardWidth = 76;
constexpr int MixerSliderHeight = 124;
constexpr int MixerGroupGap = 13;
constexpr int MixerCardPadding = 6;
constexpr int MixerLabelHeight = 52;
constexpr auto MixerAccentStyle = "color: lightskyblue";
constexpr auto MixerSelectionStyle = "QFrame#rmlMixerFader { border: 1px solid lightskyblue; border-radius: 8px; }";
constexpr auto MixerFaderName = "rmlMixerFader";
constexpr int InformationMaximumWidth = 1100;
constexpr int InformationMaximumHeight = 720;
constexpr int EditorBottomMargin = 28;
constexpr int HitTestAlpha = 1;
constexpr int FullOpacity = 255;
constexpr int FullTransparency = 100;
constexpr int MenuStartAlpha = 248;
constexpr int MenuEndAlpha = 242;
constexpr auto PanelName = "rmlFullscreenOsdPanel";
constexpr std::array SectionColors{
    qRgb(18, 38, 72), qRgb(13, 56, 57), qRgb(47, 30, 74),
    qRgb(64, 43, 24), qRgb(61, 29, 53), qRgb(34, 56, 35)};
constexpr auto SubmenuColor = qRgb(11, 25, 52);
constexpr auto SolidPanelBackground = "QFrame#rmlFullscreenOsdPanel { background: rgba(%1,%2,%3,%4); }";
constexpr auto EditorBackground = R"(
QFrame#rmlFullscreenOsdPanel {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(14,24,44,%1),stop:1 rgba(20,44,61,%1));
}
)";
constexpr auto VideoShaderBadgeName = "rmlVideoShaderBadge";
constexpr auto WindowedShaderBadgeStyle = R"(
QFrame#rmlVideoShaderBadge { background: rgba(20,22,26,190); border-radius: 8px; }
QFrame#rmlVideoShaderBadge QLabel { color: white; background: transparent; }
)";
constexpr auto FullscreenShaderBadgeStyle = R"(
QFrame#rmlVideoShaderBadge {
    background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 rgba(11,28,64,%1),stop:1 rgba(15,42,65,%1));
    border: 1px solid #4da9db; border-radius: 10px;
}
QFrame#rmlVideoShaderBadge QLabel { color: white; background: transparent; font-size: 22px; }
QFrame#rmlVideoShaderBadge QProgressBar { border: 0; background: #18385b; }
QFrame#rmlVideoShaderBadge QProgressBar::chunk { background: #1bb0f8; }
)";
constexpr auto Style = R"(
QFrame#rmlFullscreenOsdPanel {
    background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 rgba(8,18,46,%1),stop:1 rgba(11,27,52,%2));
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
constexpr auto StatisticsBadgeName = "rmlFullscreenStatisticsBadge";
constexpr auto StatisticsBadgeStyle = R"(
QFrame#rmlFullscreenStatisticsBadge { background: rgba(33,37,43,%1); border-radius: 5px; }
QFrame#rmlFullscreenStatisticsBadge QLabel { background: transparent; font-size: 14px; }
)";
constexpr int StatisticsInset = 12;
constexpr int StatisticsRowHeight = 26;
constexpr int StatisticsMinWidth = 124;
constexpr int StatisticsIconWidth = 23;
constexpr int StatisticsIconSize = 16;
constexpr int StatisticsTopBottomPadding = 5;
constexpr int StatisticsLeftPadding = 9;
constexpr int StatisticsRightPadding = 11;
constexpr std::array StatisticsColors{
    qRgb(155, 211, 218), qRgb(185, 213, 239), qRgb(174, 218, 188), qRgb(227, 202, 158), qRgb(208, 189, 229)};
constexpr int MuteInset = 44;
constexpr int MuteIconSize = 44;
constexpr int MuteShadowBlur = 5;
constexpr qreal MuteShadowOffset = 1.0;
constexpr int MuteShadowAlpha = 204;
constexpr auto VolumeBadgeName = "rmlFullscreenVolumeBadge";
constexpr int VolumeWidth = 288;
constexpr int VolumeBottomMargin = 25;
constexpr int VolumeHideMilliseconds = 2000;
constexpr int VolumeIconPixels = 19;
constexpr int VolumeBarHeight = 5;
constexpr int VolumeBarGap = 10;
constexpr int VolumeHeaderGap = 12;
constexpr int VolumeTopPadding = 12;
constexpr int VolumeBottomPadding = 14;
constexpr auto VolumeBadgeStyle = R"(
QFrame#rmlFullscreenVolumeBadge {
    background: rgba(15,29,49,%1); border: 1px solid lightskyblue; border-radius: 12px;
}
QFrame#rmlFullscreenVolumeBadge QLabel { color: white; background: transparent; font-size: 17px; }
QFrame#rmlFullscreenVolumeBadge QLabel#rmlVolumeValue { color: lightskyblue; font-size: 16px; font-weight: 600; }
QFrame#rmlFullscreenVolumeBadge QProgressBar { border: 0; background: #26384e; }
QFrame#rmlFullscreenVolumeBadge QProgressBar::chunk { background: lightskyblue; }
)";
constexpr auto VolumeValueName = "rmlVolumeValue";
constexpr auto SubmenuLabel = "%1  ›";
constexpr int SettingRole = Qt::UserRole;
constexpr int ActivationRole = Qt::UserRole + 1;
constexpr QRgb UnavailableEntryColor = qRgb(161, 174, 197);
constexpr auto SettingLabel = "%1\n%2";
constexpr auto FooterKey = "<span style=\"background-color:#263957\">&nbsp;%1&nbsp;</span> ";
constexpr auto FooterLabel = "%1&nbsp;&nbsp;";
constexpr auto FooterRow = "<p style=\"margin-top:0;margin-bottom:5px;\">%1</p>";
constexpr auto CenterBackCaption = "%1   %2";
constexpr auto CenterKeySeparator = " / ";

QString hintGroup(const QStringList& keys, const QString& description) {
    if (keys.isEmpty()) return {};
    QString markup;
    for (const auto& key : keys) markup += QString::fromLatin1(FooterKey).arg(key.toHtmlEscaped());
    return markup + QString::fromLatin1(FooterLabel).arg(description.toHtmlEscaped());
}

QString hintRow(const QString& markup) {
    return markup.isEmpty() ? QString{} : QString::fromLatin1(FooterRow).arg(markup);
}

class MixerSlider final : public QSlider {
public:
    explicit MixerSlider(QWidget* parent) : QSlider(Qt::Vertical, parent) {}

protected:
    void mousePressEvent(QMouseEvent* event) override {
        QStyleOptionSlider option;
        initStyleOption(&option);
        const auto control = style()->hitTestComplexControl(QStyle::CC_Slider, &option,
                                                            event->position().toPoint(), this);
        if (event->button() != Qt::LeftButton || control != QStyle::SC_SliderGroove) {
            QSlider::mousePressEvent(event);
            return;
        }
        absoluteDrag_ = true;
        setSliderDown(true);
        moveToPoint(option, event->position().toPoint().y());
        event->accept();
    }

    void mouseMoveEvent(QMouseEvent* event) override {
        if (!absoluteDrag_) {
            QSlider::mouseMoveEvent(event);
            return;
        }
        QStyleOptionSlider option;
        initStyleOption(&option);
        moveToPoint(option, event->position().toPoint().y());
        event->accept();
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
        if (!absoluteDrag_ || event->button() != Qt::LeftButton) {
            QSlider::mouseReleaseEvent(event);
            return;
        }
        absoluteDrag_ = false;
        setSliderDown(false);
        event->accept();
    }

private:
    void moveToPoint(const QStyleOptionSlider& option, int y) {
        const auto groove = style()->subControlRect(QStyle::CC_Slider, &option, QStyle::SC_SliderGroove, this);
        const auto handle = style()->subControlRect(QStyle::CC_Slider, &option, QStyle::SC_SliderHandle, this);
        const int span = groove.height() - handle.height();
        if (span <= 0) return;
        const int position = std::clamp(y - groove.top() - handle.height() / 2, 0, span);
        setSliderPosition(QStyle::sliderValueFromPosition(minimum(), maximum(), position, span, option.upsideDown));
    }

    bool absoluteDrag_ = false;
};
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
    if (fullscreen) setTransparency(0);
    else setStyleSheet(QString::fromLatin1(WindowedShaderBadgeStyle));
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

void ShaderLoadingBadge::setTransparency(int percent) {
    const int alpha = FullOpacity * (FullTransparency - std::clamp(percent, 0, FullTransparency)) / FullTransparency;
    const auto sheet = QString::fromLatin1(FullscreenShaderBadgeStyle).arg(alpha);
    if (sheet != styleSheet()) setStyleSheet(sheet);
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

FullscreenStatisticsBadge::FullscreenStatisticsBadge(QWidget& host, const Icons& icons)
    : QFrame(&host, Qt::Tool | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint |
        Qt::WindowDoesNotAcceptFocus | Qt::WindowTransparentForInput) {
    setObjectName(QLatin1String(StatisticsBadgeName));
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(StatisticsLeftPadding, StatisticsTopBottomPadding,
        StatisticsRightPadding, StatisticsTopBottomPadding);
    layout->setSpacing(0);
    for (std::size_t index = 0; index < values_.size(); ++index) {
        auto* row = new QWidget(this);
        row->setMinimumWidth(StatisticsMinWidth);
        row->setFixedHeight(StatisticsRowHeight);
        auto* columns = new QHBoxLayout(row);
        columns->setContentsMargins({});
        columns->setSpacing(0);
        auto* icon = new QLabel(row);
        icon->setFixedWidth(StatisticsIconWidth);
        auto glyph = icons[index].pixmap(QSize(StatisticsIconSize, StatisticsIconSize), devicePixelRatioF());
        if (!glyph.isNull()) {
            QPainter painter(&glyph);
            painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            painter.fillRect(QRectF(QPointF(), glyph.deviceIndependentSize()), QColor::fromRgb(StatisticsColors[index]));
        }
        icon->setPixmap(glyph);
        columns->addWidget(icon);
        auto* value = values_[index] = new QLabel(row);
        value->setTextFormat(Qt::PlainText);
        auto palette = value->palette();
        palette.setColor(QPalette::WindowText, QColor::fromRgb(StatisticsColors[index]));
        value->setPalette(palette);
        columns->addWidget(value, 1);
        layout->addWidget(row);
    }
    setTransparency(0);
}

void FullscreenStatisticsBadge::setValues(const QWidget& anchor, const Values& values) {
    bool changed = false;
    for (std::size_t index = 0; index < values_.size(); ++index) {
        if (values_[index]->text() == values[index]) continue;
        values_[index]->setText(values[index]);
        changed = true;
    }
    if (changed || !isVisible()) reposition(anchor);
}

void FullscreenStatisticsBadge::setTransparency(int percent) {
    const int alpha = FullOpacity * (FullTransparency - std::clamp(percent, 0, FullTransparency)) / FullTransparency;
    const auto sheet = QString::fromLatin1(StatisticsBadgeStyle).arg(alpha);
    if (sheet != styleSheet()) setStyleSheet(sheet);
}

void FullscreenStatisticsBadge::reposition(const QWidget& anchor) {
    adjustSize();
    move(anchor.mapToGlobal(QPoint(std::max(0, anchor.width() - width() - StatisticsInset), StatisticsInset)));
}

FullscreenMuteBadge::FullscreenMuteBadge(QWidget& host, const QIcon& icon)
    : QWidget(&host, Qt::Tool | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint |
        Qt::WindowDoesNotAcceptFocus | Qt::WindowTransparentForInput) {
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    constexpr int padding = MuteShadowBlur + int(MuteShadowOffset);
    auto* label = new QLabel(this);
    label->setGeometry(padding, padding, MuteIconSize, MuteIconSize);
    auto glyph = icon.pixmap(QSize(MuteIconSize, MuteIconSize), devicePixelRatioF());
    if (!glyph.isNull()) {
        QPainter tint(&glyph);
        tint.setCompositionMode(QPainter::CompositionMode_SourceIn);
        tint.fillRect(QRect(0, 0, MuteIconSize, MuteIconSize), Qt::white);
    }
    label->setPixmap(glyph);
    auto* shadow = new QGraphicsDropShadowEffect(label);
    shadow->setBlurRadius(MuteShadowBlur);
    shadow->setOffset(0.0, MuteShadowOffset);
    shadow->setColor(QColor(0, 0, 0, MuteShadowAlpha));
    label->setGraphicsEffect(shadow);
    setFixedSize(MuteIconSize + 2 * padding, MuteIconSize + 2 * padding);
}

void FullscreenMuteBadge::reposition(const QWidget& anchor) {
    constexpr int inset = MuteInset - MuteShadowBlur - int(MuteShadowOffset);
    move(anchor.mapToGlobal(QPoint(std::max(0, anchor.width() - width() - inset),
        std::max(0, anchor.height() - height() - inset))));
}

FullscreenVolumeBadge::FullscreenVolumeBadge(QWidget& host, const QIcon& icon)
    : QFrame(&host, Qt::ToolTip | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint |
        Qt::WindowDoesNotAcceptFocus | Qt::WindowTransparentForInput),
      title_(new QLabel(this)), value_(new QLabel(this)), level_(new QProgressBar(this)), hideTimer_(new QTimer(this)) {
    setObjectName(QLatin1String(VolumeBadgeName));
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    title_->setTextFormat(Qt::PlainText);
    value_->setTextFormat(Qt::PlainText);
    value_->setObjectName(QLatin1String(VolumeValueName));
    auto* marker = new QLabel(this);
    marker->setPixmap(icon.pixmap(QSize(VolumeIconPixels, VolumeIconPixels), devicePixelRatioF()));
    auto* header = new QHBoxLayout;
    header->setSpacing(VolumeHeaderGap);
    header->addWidget(marker);
    header->addWidget(title_, 1);
    header->addWidget(value_, 0, Qt::AlignRight);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(PanelMargin, VolumeTopPadding, PanelMargin, VolumeBottomPadding);
    layout->setSpacing(VolumeBarGap);
    layout->addLayout(header);
    level_->setRange(0, FullTransparency);
    level_->setFixedHeight(VolumeBarHeight);
    level_->setTextVisible(false);
    layout->addWidget(level_);
    hideTimer_->setSingleShot(true);
    hideTimer_->setInterval(VolumeHideMilliseconds);
    connect(hideTimer_, &QTimer::timeout, this, &QWidget::hide);
    setTransparency(0);
}

void FullscreenVolumeBadge::setTransparency(int percent) {
    const int alpha = FullOpacity * (FullTransparency - std::clamp(percent, 0, FullTransparency)) / FullTransparency;
    const auto sheet = QString::fromLatin1(VolumeBadgeStyle).arg(alpha);
    if (sheet != styleSheet()) setStyleSheet(sheet);
}

void FullscreenVolumeBadge::reposition(const QWidget& anchor) {
    setFixedWidth(std::min(VolumeWidth, std::max(1, anchor.width())));
    adjustSize();
    move(anchor.mapToGlobal(QPoint{std::max(0, (anchor.width() - width()) / 2),
        std::max(0, anchor.height() - height() - VolumeBottomMargin)}));
}

void FullscreenVolumeBadge::showVolume(const QWidget& anchor, const QString& title, const QString& value, int percent) {
    title_->setText(title);
    value_->setText(value);
    level_->setValue(percent);
    reposition(anchor);
    show();
    raise();
    hideTimer_->start();
}

struct FullscreenOsd::State {
    struct Level {
        const std::vector<Entry>* entries;
        QFrame* panel;
        QListWidget* rows;
        const Entry* parent;
        struct Fader {
            const Entry* entry;
            QFrame* card;
            QSlider* slider;
            QLabel* value;
        };
        struct MixerSection {
            QWidget* widget;
            std::vector<Fader> faders;
        };
        QScrollArea* mixer = nullptr;
        std::vector<MixerSection> sections{};
        QSlider* selected = nullptr;
        QLabel* centerHints = nullptr;
        QPushButton* centerBack = nullptr;
    };

    QWidget& host;
    QString footer;
    const std::vector<Entry> entries;
    std::function<void()> dismissed;
    std::vector<Level> levels;
    int submenuTop = 0;
    QTimer* idle = nullptr;
    QTimer* settingSave = nullptr;
    QTimer* faderSave = nullptr;
    std::vector<std::pair<const Setting*, int>> pendingFaders;
    bool runningCommand = false;
    bool externalKeyboardNavigation = false;
    QFrame* editor = nullptr;
    ShaderLoadingBadge* shaderBadge = nullptr;
    int transparency = 0;
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
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setTransparency(0);
    state_->idle = new QTimer(this);
    state_->idle->setInterval(IdleMilliseconds);
    state_->idle->setSingleShot(true);
    connect(state_->idle, &QTimer::timeout, this, [this] {
        if (hasPointerCapture()) state_->idle->start();
        else close();
    });
    state_->settingSave = new QTimer(this);
    state_->settingSave->setSingleShot(true);
    connect(state_->settingSave, &QTimer::timeout, this, &FullscreenOsd::flushPendingSetting);
    state_->faderSave = new QTimer(this);
    state_->faderSave->setSingleShot(true);
    connect(state_->faderSave, &QTimer::timeout, this, &FullscreenOsd::flushPendingFaders);
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
    state_->editorHints = new QLabel(state_->editor);
    state_->editorHints->setTextFormat(Qt::RichText);
    state_->editorHints->setWordWrap(true);
    editorLayout->addWidget(state_->editorHints);
    state_->editorBack = new QPushButton(state_->editor);
    state_->editorBack->setFocusPolicy(Qt::NoFocus);
    editorLayout->addWidget(state_->editorBack, 0, Qt::AlignLeft);
    connect(state_->editorBack, &QPushButton::clicked, this, [this] { navigate(Navigation::Cancel); });
    state_->editor->setMouseTracking(true);
    for (auto* child : state_->editor->findChildren<QWidget*>()) child->setMouseTracking(true);
    state_->editor->hide();
    connect(state_->slider, &QSlider::valueChanged, this, &FullscreenOsd::adjustValue);
    openLevel(state_->entries, machine, media);
    qApp->installEventFilter(this);
    fitHost();
}

FullscreenOsd::~FullscreenOsd() {
    qApp->removeEventFilter(this);
}

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

void FullscreenOsd::setTransparency(int percent) {
    state_->transparency = std::clamp(percent, 0, FullTransparency);
    const int opacity = FullTransparency - state_->transparency;
    const auto sheet = QString::fromLatin1(Style)
        .arg(MenuStartAlpha * opacity / FullTransparency).arg(MenuEndAlpha * opacity / FullTransparency);
    if (sheet != styleSheet()) setStyleSheet(sheet);
    if (state_->shaderBadge) state_->shaderBadge->setTransparency(state_->transparency);
    for (std::size_t depth = 0; depth < state_->levels.size(); ++depth) updateSectionBackground(depth);
}

void FullscreenOsd::setShaderLoading(bool loading, const QString& caption) {
    if (!loading) {
        if (state_->shaderBadge) state_->shaderBadge->hide();
        return;
    }
    if (!state_->shaderBadge) {
        state_->shaderBadge = new ShaderLoadingBadge(true, this);
        state_->shaderBadge->setTransparency(state_->transparency);
    }
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
    refreshCenterHints();
    positionPanels();
}

void FullscreenOsd::openLevel(const std::vector<Entry>& entries, const QString& title, const QString& subtitle,
    const Entry* parent) {
    auto* panel = new QFrame(this);
    panel->setObjectName(QString::fromLatin1(PanelName));
    auto* content = new QVBoxLayout(panel);
    content->setContentsMargins(0, PanelMargin, 0, PanelMargin);
    const auto addCaption = [&](const QString& caption, const QIcon& icon = QIcon{}) {
        auto* label = new QLabel(caption, panel);
        label->setTextFormat(Qt::PlainText);
        label->setWordWrap(true);
        if (icon.isNull()) {
            label->setContentsMargins(PanelMargin, CaptionSpacing, PanelMargin, CaptionSpacing);
            content->addWidget(label);
        } else {
            auto* row = new QHBoxLayout;
            row->setContentsMargins(PanelMargin, CaptionSpacing, PanelMargin, CaptionSpacing);
            row->setSpacing(CenterHeaderIconGap);
            auto* glyph = new QLabel(panel);
            glyph->setPixmap(icon.pixmap(QSize(CenterHeaderIconSize, CenterHeaderIconSize), devicePixelRatioF()));
            row->addWidget(glyph, 0, Qt::AlignVCenter);
            row->addWidget(label, 1);
            content->addLayout(row);
        }
        return label;
    };
    const bool centered = parent && (parent->centeredInformation || parent->centeredMixer);
    auto* header = addCaption(title, centered ? parent->icon : QIcon{});
    auto font = header->font();
    font.setPixelSize(MainHeadingPixels);
    font.setWeight(QFont::DemiBold);
    header->setFont(font);
    if (centered) {
        auto* separator = new QFrame(panel);
        separator->setFrameShape(QFrame::HLine);
        content->addWidget(separator);
    }
    if (state_->levels.empty()) {
        state_->gameName = addCaption(subtitle);
        state_->gameName->setVisible(!subtitle.isEmpty());
    }
    auto* rows = new QListWidget(panel);
    rows->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    const bool information = parent && parent->centeredInformation;
    rows->setTextElideMode(information ? Qt::ElideNone : Qt::ElideRight);
    if (information) {
        rows->setSelectionMode(QAbstractItemView::NoSelection);
        rows->setWordWrap(true);
    }
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
        if (information && !entry.status) {
            auto heading = row->font();
            heading.setWeight(QFont::DemiBold);
            row->setFont(heading);
        }
        if (!information) row->setSizeHint({0, EntryHeight});
        row->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        row->setData(SettingRole, entry.setting.has_value());
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
    panel->setMouseTracking(true);
    connect(rows, &QListWidget::currentRowChanged, this, [this, level] { updateSectionBackground(level); });
    connect(rows, &QListWidget::itemClicked, this, [this, level, rows](QListWidgetItem* item) {
        invoke(level, rows->row(item), false);
    });
    if (parent && parent->centeredMixer) buildMixer();
    if (centered) {
        auto& current = state_->levels.back();
        auto* footer = new QHBoxLayout;
        footer->setContentsMargins(parent->centeredMixer ? 0 : PanelMargin, 0,
                                   parent->centeredMixer ? 0 : PanelMargin, 0);
        current.centerHints = new QLabel(panel);
        current.centerHints->setTextFormat(Qt::RichText);
        current.centerHints->setWordWrap(true);
        footer->addWidget(current.centerHints, 1);
        current.centerBack = new QPushButton(panel);
        current.centerBack->setFocusPolicy(Qt::NoFocus);
        connect(current.centerBack, &QPushButton::clicked, this, [this] { navigate(Navigation::Cancel); });
        footer->addWidget(current.centerBack);
        content->addLayout(footer);
        refreshCenterHints();
    }
    for (auto* child : panel->findChildren<QWidget*>()) child->setMouseTracking(true);
    refresh();
    focusLevel();
}

void FullscreenOsd::focusLevel() {
    const auto& level = state_->levels.back();
    if (level.mixer) setFocus();
    else level.rows->setFocus();
}

void FullscreenOsd::refreshCenterHints() {
    const auto& hints = state_->hints;
    for (const auto& level : state_->levels) {
        if (!level.centerBack) continue;
        const auto markup = level.parent->centeredMixer
            ? hintGroup(hints.horizontal, hints.navigateLabel) + hintGroup(hints.vertical, hints.adjustLabel)
            : QString{};
        level.centerHints->setText(hintRow(markup));
        level.centerHints->setVisible(!markup.isEmpty());
        const auto keys = hints.back.join(QLatin1String(CenterKeySeparator));
        level.centerBack->setText(keys.isEmpty() ? hints.backLabel
            : QString::fromLatin1(CenterBackCaption).arg(keys, hints.backLabel));
        level.centerBack->setVisible(!hints.backLabel.isEmpty());
    }
}

void FullscreenOsd::buildMixer() {
    auto& level = state_->levels.back();
    level.rows->hide();
    auto* layout = static_cast<QVBoxLayout*>(level.panel->layout());
    layout->setContentsMargins(PanelMargin, PanelMargin, PanelMargin, PanelMargin);
    auto* area = new QScrollArea(level.panel);
    area->setFrameShape(QFrame::NoFrame);
    area->setWidgetResizable(true);
    area->setFocusPolicy(Qt::NoFocus);
    area->setStyleSheet(QStringLiteral("QScrollArea { background: transparent; }"));
    area->viewport()->setAutoFillBackground(false);
    auto* body = new QWidget(area);
    auto* groups = new QHBoxLayout(body);
    groups->setContentsMargins({});
    groups->setSpacing(MixerGroupGap);
    level.mixer = area;
    std::vector<QString> titles;
    for (const auto& entry : *level.entries) {
        const auto existing = std::ranges::find(titles, entry.mixerSection);
        const auto sectionIndex = std::size_t(existing - titles.begin());
        if (existing == titles.end()) {
            titles.push_back(entry.mixerSection);
            auto* section = new QWidget(body);
            auto* sectionLayout = new QVBoxLayout(section);
            sectionLayout->setContentsMargins({});
            auto* heading = new QLabel(entry.mixerSection, section);
            heading->setTextFormat(Qt::PlainText);
            heading->setStyleSheet(QString::fromLatin1(MixerAccentStyle));
            heading->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
            heading->setToolTip(entry.mixerSection);
            sectionLayout->addWidget(heading);
            auto* cards = new QHBoxLayout;
            cards->setSpacing(0);
            sectionLayout->addLayout(cards);
            groups->addWidget(section);
            level.sections.push_back({section, {}});
        }
        auto& section = level.sections[sectionIndex];
        auto* cards = static_cast<QVBoxLayout*>(section.widget->layout())->itemAt(1)->layout();
        auto* card = new QFrame(section.widget);
        card->setObjectName(QLatin1String(MixerFaderName));
        card->setFixedWidth(MixerCardWidth);
        auto* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(0, MixerCardPadding, 0, MixerCardPadding);
        auto* value = new QLabel(card);
        value->setTextFormat(Qt::PlainText);
        value->setStyleSheet(QString::fromLatin1(MixerAccentStyle));
        auto* slider = new MixerSlider(card);
        const auto& setting = *entry.setting;
        slider->setRange(setting.minimum, setting.maximum);
        slider->setFixedHeight(MixerSliderHeight);
        slider->setFocusPolicy(Qt::NoFocus);
        slider->setAccessibleName(entry.label);
        slider->setToolTip(entry.label);
        auto* name = new QLabel(entry.label, card);
        name->setTextFormat(Qt::PlainText);
        name->setWordWrap(true);
        name->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
        name->setFixedHeight(MixerLabelHeight);
        cardLayout->addWidget(value, 0, Qt::AlignHCenter);
        cardLayout->addWidget(slider, 0, Qt::AlignHCenter);
        cardLayout->addWidget(name);
        cards->addWidget(card);
        section.faders.push_back({&entry, card, slider, value});
        connect(slider, &QSlider::sliderPressed, this, [this, slider] {
            auto& active = state_->levels.back();
            if (!active.mixer || active.selected == slider || !active.panel->isAncestorOf(slider)) return;
            active.selected = slider;
            refreshMixer();
        });
        connect(slider, &QSlider::sliderReleased, this, [this, slider] {
            if (state_->levels.back().mixer && state_->levels.back().panel->isAncestorOf(slider))
                refreshMixer();
        });
        connect(slider, &QSlider::valueChanged, this, [this, slider, value, &setting](int draft) {
            if (!state_->levels.back().mixer || !state_->levels.back().panel->isAncestorOf(slider)) return;
            state_->idle->start();
            state_->levels.back().selected = slider;
            value->setText(setting.format(draft));
            auto& pending = state_->pendingFaders;
            const auto previous = std::ranges::find(pending, &setting, &std::pair<const Setting*, int>::first);
            if (previous == pending.end()) pending.emplace_back(&setting, draft);
            else previous->second = draft;
            if (!state_->faderSave->isActive()) state_->faderSave->start();
            if (setting.preview) setting.preview(draft);
        });
    }
    groups->addStretch();
    area->setWidget(body);
    layout->addWidget(area, 1);
}

void FullscreenOsd::flushPendingFaders() {
    if (state_->pendingFaders.empty()) return;
    state_->faderSave->stop();
    const auto pending = std::exchange(state_->pendingFaders, {});
    const QPointer<FullscreenOsd> alive(this);
    for (const auto& [setting, draft] : pending) {
        if (setting->read() != draft) setting->save(draft);
        if (!alive) return;
        const int current = setting->read();
        if (current != draft && setting->preview) setting->preview(current);
        if (!alive) return;
    }
    refresh();
}

void FullscreenOsd::refreshMixer() {
    auto& level = state_->levels.back();
    if (!level.mixer) return;
    QSlider* first = nullptr;
    bool selectionVisible = false;
    for (auto& section : level.sections) {
        bool sectionVisible = false;
        for (auto& fader : section.faders) {
            const auto& entry = *fader.entry;
            const auto& setting = *entry.setting;
            const bool visible = !entry.visible || entry.visible();
            fader.card->setVisible(visible);
            sectionVisible |= visible;
            if (visible && !first) first = fader.slider;
            selectionVisible |= visible && fader.slider == level.selected;
            fader.slider->setEnabled(!entry.enabled || entry.enabled());
            const auto& pending = state_->pendingFaders;
            if (!visible || fader.slider->isSliderDown() ||
                std::ranges::find(pending, &setting, &std::pair<const Setting*, int>::first) != pending.end()) continue;
            const QSignalBlocker block(fader.slider);
            fader.slider->setValue(setting.read());
            fader.value->setText(setting.format(fader.slider->value()));
        }
        section.widget->setVisible(sectionVisible);
    }
    if (!selectionVisible) level.selected = first;
    for (auto& section : level.sections) {
        for (auto& fader : section.faders) {
            const auto sheet = fader.slider == level.selected ? QString::fromLatin1(MixerSelectionStyle) : QString{};
            if (fader.card->styleSheet() != sheet) fader.card->setStyleSheet(sheet);
        }
    }
}

void FullscreenOsd::navigateMixer(Navigation direction) {
    auto& level = state_->levels.back();
    if (direction == Navigation::Cancel || direction == Navigation::Confirm) {
        retainLevels(state_->levels.size() - 1);
        return;
    }
    if (!level.selected) return;
    if (direction == Navigation::Up || direction == Navigation::Down) {
        if (level.selected->isEnabled())
            level.selected->triggerAction(direction == Navigation::Up
                ? QAbstractSlider::SliderSingleStepAdd : QAbstractSlider::SliderSingleStepSub);
        return;
    }
    std::vector<QSlider*> visible;
    for (const auto& section : level.sections)
        for (const auto& fader : section.faders)
            if (fader.card->isVisible()) visible.push_back(fader.slider);
    const auto found = std::ranges::find(visible, level.selected);
    if (found == visible.end()) return;
    const auto index = std::clamp(found - visible.begin() + (direction == Navigation::Left ? -1 : 1),
        std::ptrdiff_t{}, std::ssize(visible) - 1);
    level.selected = visible[std::size_t(index)];
    refreshMixer();
    level.mixer->ensureWidgetVisible(level.selected);
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
    focusLevel();
}

void FullscreenOsd::positionPanels() {
    if (state_->levels.empty()) return;
    auto& root = state_->levels.front();
    const bool hasSubmenu = state_->levels.size() > 1;
    const int rootWidth = std::min(PanelWidth,
        hasSubmenu ? width() * PanelWidth / (PanelWidth + SubmenuWidth) : width());
    root.panel->setGeometry(0, 0, rootWidth, height());
    const auto* parent = state_->levels.back().parent;
    const bool information = parent && parent->centeredInformation;
    const bool mixer = parent && parent->centeredMixer;
    root.panel->setVisible(!state_->edited && !information && !mixer);
    for (std::size_t index = 1; index < state_->levels.size(); ++index) {
        auto& level = state_->levels[index];
        const bool current = index + 1 == state_->levels.size() && !state_->edited;
        level.panel->setVisible(current);
        if (!current) continue;
        if (mixer) {
            const auto preferred = level.mixer->widget()->sizeHint();
            const auto chrome = level.panel->layout()->sizeHint() - level.mixer->sizeHint();
            const int panelWidth = std::min(preferred.width() + chrome.width(),
                std::max(1, width() - 2 * PanelMargin));
            const int scrollHeight = panelWidth < preferred.width() + chrome.width()
                ? level.mixer->horizontalScrollBar()->sizeHint().height() : 0;
            const int panelHeight = std::min(preferred.height() + chrome.height() + scrollHeight,
                std::max(1, height() - 2 * InformationMargin));
            level.panel->setGeometry((width() - panelWidth) / 2, (height() - panelHeight) / 2, panelWidth, panelHeight);
            continue;
        }
        if (information) {
            const int panelWidth = std::min(InformationMaximumWidth, std::max(1, width() - 2 * InformationMargin));
            int contentHeight = 0;
            for (int row = 0; row < level.rows->count(); ++row) {
                if (!level.rows->item(row)->isHidden())
                    contentHeight += std::max(0, level.rows->sizeHintForRow(row));
            }
            const int chrome = level.panel->layout()->sizeHint().height() - level.rows->sizeHint().height();
            const int panelHeight = std::min({InformationMaximumHeight, std::max(1, height() - 2 * InformationMargin),
                contentHeight + chrome});
            level.panel->setGeometry((width() - panelWidth) / 2, (height() - panelHeight) / 2, panelWidth, panelHeight);
            continue;
        }
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

void FullscreenOsd::updateSectionBackground(std::size_t depth) {
    if (depth >= state_->levels.size() || (depth == 0 && !state_->edited)) return;
    const auto& level = state_->levels[depth];
    const int selected = level.rows->currentRow();
    const int alpha = FullOpacity * (FullTransparency - state_->transparency) / FullTransparency;
    QString section;
    std::size_t groups = 0;
    int row = 0;
    for (const auto& entry : *level.entries) {
        row += entry.separatorBefore;
        if (row > selected) break;
        const auto& entrySection = entry.sectionHeading ? entry.label : entry.section;
        if (!entrySection.isEmpty() && !level.rows->item(row)->isHidden() && section != entrySection) {
            section = entrySection;
            ++groups;
        }
        ++row;
    }
    const auto styleFor = [alpha](QRgb color) {
        return QString::fromLatin1(SolidPanelBackground)
            .arg(qRed(color)).arg(qGreen(color)).arg(qBlue(color)).arg(alpha);
    };
    const auto color = groups ? SectionColors[(groups - 1) % SectionColors.size()] : SubmenuColor;
    const auto panelStyle = styleFor(color);
    if (depth > 0 && level.panel->styleSheet() != panelStyle) level.panel->setStyleSheet(panelStyle);
    if (depth + 1 == state_->levels.size() && state_->edited) {
        const auto editorStyle = groups ? panelStyle : QString::fromLatin1(EditorBackground).arg(alpha);
        if (state_->editor->styleSheet() != editorStyle) state_->editor->setStyleSheet(editorStyle);
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
            const bool information = level.parent && level.parent->centeredInformation;
            const bool enabled = information || !entry.enabled || entry.enabled();
            item->setHidden(!visible);
            item->setData(ActivationRole, enabled);
            item->setForeground(enabled ? QBrush{} : QBrush(QColor::fromRgb(UnavailableEntryColor)));
            if (visible) {
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
            else if (entry.status) {
                const auto status = entry.status();
                item->setText(status.isEmpty() ? entry.label : QString::fromLatin1(SettingLabel).arg(entry.label, status));
            }
        }
        const int selected = next == -1 ? before : next;
        if (selected != level.rows->currentRow()) level.rows->setCurrentRow(selected);
        updateSectionBackground(depth);
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
    refreshMixer();
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
        if (entry.enabled && !entry.enabled()) return;
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
    updateSectionBackground(state_->levels.size() - 1);
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
    const QPointer<FullscreenOsd> stillAlive(this);
    flushPendingFaders();
    if (!stillAlive) return;
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
        if (item->data(SettingRole).toBool() && item->data(ActivationRole).toBool() && !item->isHidden()) {
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
    const auto* parent = state_->levels.back().parent;
    if (parent && parent->centeredMixer) {
        navigateMixer(direction);
        return;
    }
    if (parent && parent->centeredInformation) {
        if (direction == Navigation::Confirm || direction == Navigation::Cancel || direction == Navigation::Left)
            retainLevels(index);
        else if (direction == Navigation::Up || direction == Navigation::Down)
            rows->verticalScrollBar()->triggerAction(direction == Navigation::Up
                ? QAbstractSlider::SliderSingleStepSub : QAbstractSlider::SliderSingleStepAdd);
        return;
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

bool FullscreenOsd::isEditing() const noexcept {
    return state_->edited || (!state_->levels.empty() && state_->levels.back().mixer);
}

bool FullscreenOsd::hasPointerCapture() const {
    const auto* grabbed = QWidget::mouseGrabber();
    if (state_->slider->isSliderDown() || (grabbed && isAncestorOf(grabbed))) return true;
    if (state_->levels.empty()) return false;
    const auto& level = state_->levels.back();
    if (level.mixer) return (level.selected && level.selected->isSliderDown()) ||
        level.mixer->horizontalScrollBar()->isSliderDown() || level.mixer->verticalScrollBar()->isSliderDown();
    return level.parent && level.parent->centeredInformation && level.rows->verticalScrollBar()->isSliderDown();
}

bool FullscreenOsd::usesPointerAt(const QPoint& globalPosition) const {
    if (hasPointerCapture()) return true;
    const auto contains = [&](const QWidget* panel) {
        return panel->isVisible() && panel->rect().contains(panel->mapFromGlobal(globalPosition));
    };
    return contains(state_->editor) || std::ranges::any_of(state_->levels, [&](const auto& level) {
        return contains(level.panel);
    });
}

void FullscreenOsd::setExternalKeyboardNavigation(bool enabled) noexcept {
    state_->externalKeyboardNavigation = enabled;
}

bool FullscreenOsd::eventFilter(QObject* object, QEvent* event) {
    if (object == &state_->host) {
        if (event->type() == QEvent::Move || event->type() == QEvent::Resize) fitHost();
        return false;
    }
    const auto* target = qobject_cast<QWidget*>(object);
    if (!isVisible() || !target || (target != this && !isAncestorOf(target))) return false;
    const auto type = event->type();
    const bool pointerActivity = type == QEvent::MouseMove || type == QEvent::MouseButtonPress ||
        type == QEvent::MouseButtonRelease || type == QEvent::MouseButtonDblClick || type == QEvent::Wheel;
    if (!state_->runningCommand && (type == QEvent::KeyPress ||
        (pointerActivity && usesPointerAt(static_cast<QSinglePointEvent*>(event)->globalPosition().toPoint()))))
        state_->idle->start();
    if (type == QEvent::MouseButtonPress && static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton) {
        auto& level = state_->levels.back();
        for (const auto& section : level.sections) {
            const auto fader = std::ranges::find_if(section.faders, [target](const auto& item) {
                return target == item.card || item.card->isAncestorOf(target);
            });
            if (fader == section.faders.end()) continue;
            if (level.selected != fader->slider) {
                level.selected = fader->slider;
                refreshMixer();
            }
            break;
        }
    }
    if (!state_->runningCommand && type == QEvent::Wheel && state_->edited &&
        (target == state_->editor || state_->editor->isAncestorOf(target))) {
        const auto delta = static_cast<QWheelEvent*>(event)->angleDelta().y();
        if (delta != 0) adjustValue(state_->draft + (delta < 0 ? -1 : 1));
        return true;
    }
    if (type == QEvent::KeyPress && !state_->externalKeyboardNavigation) {
        const auto navigateDefault = [this](Navigation direction) {
            navigate(direction);
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
}
}
