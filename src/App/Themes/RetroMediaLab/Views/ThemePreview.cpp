#include "ThemePreview.h"
#include "../ThemeConstants.h"
#include "../RetroMediaLabLayout.h"
#include <QPainter>
#include <QStyle>
#include <QTimer>
namespace RetroMediaLab {
ThemePreview::ThemePreview(ThemeService& themes, Translations::TranslationService& translations,
    const WorkspaceState& state, QWidget* source, QWidget* parent)
    : QWidget(parent), themes_(themes), translations_(translations), state_(state), source_(source) {
    setObjectName(ThemeConstants::ThemePreview);
    setMinimumHeight(ThemeConstants::ThemeSettings::PreviewMinimumHeight);
    setMaximumHeight(ThemeConstants::ThemeSettings::PreviewMaximumHeight);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}
void ThemePreview::setSettings(const QJsonObject& settings) {
    settings_ = settings;
    refresh();
}
void ThemePreview::refresh() {
    if (scheduled_) return;
    scheduled_ = true;
    QTimer::singleShot(0, this, [this] {
        scheduled_ = false;
        if (!source_) return;
        auto preview = themes_.createPreview(settings_);
        std::unique_ptr<QStyle> style(preview->createStyle());
        WorkspaceState snapshot;
        snapshot.openPageIds = state_.openPageIds;
        snapshot.activePageId = state_.activePageId;
        snapshot.console = state_.console;
        snapshot.setDefaultImageDirectory(state_.defaultImageDirectory());
        // Own a detached body only: no application window, DWM call or preferences dialog.
        auto view = std::make_unique<RetroMediaLabLayout>(*preview, translations_, snapshot, [] {}, nullptr);
        view->setAutoFillBackground(true);
        auto palette = preview->palette();
        // Resolve every local role explicitly, including a native palette with mask zero:
        // otherwise QWidget would inherit the active application's colors in the preview.
        for (int role = 0; role < QPalette::NColorRoles; ++role) {
            const auto colorRole = static_cast<QPalette::ColorRole>(role);
            if (colorRole == QPalette::NoRole) continue;
            for (const auto group : {QPalette::Active, QPalette::Inactive, QPalette::Disabled})
                palette.setBrush(group, colorRole, palette.brush(group, colorRole));
        }
        view->setPalette(palette);
        auto font = view->font();
        if (!preview->data()->fontFamily.isEmpty()) font.setFamily(preview->data()->fontFamily);
        font.setPixelSize(preview->data()->fontSize);
        view->setFont(font);
        view->setStyle(style.get());
        for (auto* child : view->findChildren<QWidget*>()) child->setStyle(style.get());
        view->resize(source_->size());
        view->ensurePolished();
        image_ = view->grab();
        update();
        // view dies before snapshot/style/service; only the owned image survives.
    });
}
void ThemePreview::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    refresh();
}
void ThemePreview::paintEvent(QPaintEvent*) {
    const auto theme = themes_.data();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(theme->color(ThemeConstants::ControlBrush));
    painter.setPen(theme->color(ThemeConstants::BorderBrush));
    painter.drawRoundedRect(QRectF(rect()).adjusted(ThemeConstants::ThemeSettings::BorderInset,
        ThemeConstants::ThemeSettings::BorderInset, -ThemeConstants::ThemeSettings::BorderInset,
        -ThemeConstants::ThemeSettings::BorderInset), theme->cardRadius, theme->cardRadius);
    if (image_.isNull()) return;
    auto scaled = image_.size();
    scaled.scale(size() - QSize(ThemeConstants::ThemeSettings::PreviewInset, ThemeConstants::ThemeSettings::PreviewInset), Qt::KeepAspectRatio);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.drawPixmap(QRect(QPoint((width() - scaled.width()) / ThemeConstants::ThemeSettings::CenterDivisor,
        (height() - scaled.height()) / ThemeConstants::ThemeSettings::CenterDivisor), scaled), image_);
}
}
