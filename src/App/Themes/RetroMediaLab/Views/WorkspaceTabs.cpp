#include "WorkspaceTabs.h"
#include "Shared/Navigation/WorkspacePages.h"
#include "../ThemeIcons.h"
namespace RetroMediaLab {
WorkspaceTabs::WorkspaceTabs(ThemeService& themes, Translations::TranslationService& translations,
    WorkspaceState& state, QWidget* parent) : QTabWidget(parent) {
    setObjectName("workspaceTabs");
    setDocumentMode(false);
    setUsesScrollButtons(true);
    setIconSize(QSize(17, 17));
    for (const auto& id : state.openPageIds) {
        for (const auto& page : workspacePages()) {
            if (id != page.id) continue;
            auto* view = new QWidget(this);
            view->setObjectName(id);
            view->setAutoFillBackground(true);
            addTab(view, themeIcon(id, themes.data()->color("MutedTextBrush")),
                QString::fromStdString(translations.text(page.translationKey)));
            break;
        }
    }
    for (int i = 0; i < count(); ++i)
        if (widget(i)->objectName() == state.activePageId) setCurrentIndex(i);
    connect(this, &QTabWidget::currentChanged, this, [this, &state](int index) {
        if (index >= 0) state.activePageId = widget(index)->objectName();
    });
    subscription_ = translations.subscribe([this, &translations] {
        for (int i = 0; i < count(); ++i) {
            for (const auto& page : workspacePages()) {
                if (widget(i)->objectName() == page.id)
                    setTabText(i, QString::fromStdString(translations.text(page.translationKey)));
            }
        }
    });
    themeSubscription_ = themes.subscribe([this, &themes] {
        for (int i = 0; i < count(); ++i)
            setTabIcon(i, themeIcon(widget(i)->objectName(), themes.data()->color("MutedTextBrush")));
    });
}
}

