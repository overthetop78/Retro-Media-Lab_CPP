#include "../ThemeConstants.h"
#include "WorkspaceTabs.h"
#include "ExplorerPage.h"
#include "VisualizationPage.h"
#include "CapturePage.h"
#include "Shared/Navigation/WorkspacePages.h"
#include "../ThemeIcons.h"
namespace RetroMediaLab {
WorkspaceTabs::WorkspaceTabs(ThemeService& themes, Translations::TranslationService& translations,
    WorkspaceState& state, QWidget* parent) : QTabWidget(parent) {
    setObjectName(ThemeConstants::WorkspaceTabs);
    setDocumentMode(false);
    setUsesScrollButtons(true);
    setIconSize(QSize(ThemeConstants::Tabs::IconExtent, ThemeConstants::Tabs::IconExtent));
    for (const auto& id : state.openPageIds) {
        for (const auto& page : workspacePages()) {
            if (id != page.id) continue;
            QWidget* view;
            if (id == AppConstants::Explorer) view = new ExplorerPage(themes, translations, state, this);
            else if (id == AppConstants::Visualize) view = new VisualizationPage(themes, translations, state, this);
            else if (id == AppConstants::Read || id == AppConstants::Write) view = new CapturePage(id == AppConstants::Read ? Infrastructure::CaptureMode::Read : Infrastructure::CaptureMode::Write,themes,translations,state,[this](const QString& target){for(int index=0;index<count();++index)if(widget(index)->objectName()==target){setCurrentIndex(index);break;}},this);
            else view = new QWidget(this);
            view->setObjectName(id);
            view->setAutoFillBackground(true);
            addTab(view, themeIcon(id, themes.data()->color(ThemeConstants::MutedTextBrush)),
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
            setTabIcon(i, themeIcon(widget(i)->objectName(), themes.data()->color(ThemeConstants::MutedTextBrush)));
    });
}
}

