#pragma once
#include "Themes/ThemeService.h"
#include "../ThemeConstants.h"
#include "State/WorkspaceState.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QWidget>
#include <array>
class QLabel;
class QListWidget;
class QPushButton;
namespace RetroMediaLab {
class ProfileSettingsPage final : public QWidget {
public:
    ProfileSettingsPage(ThemeService&, Translations::TranslationService&, WorkspaceState&, QWidget*);
protected:
    bool eventFilter(QObject*,QEvent*)override;
private:
    void refreshTexts();
    void refresh();
    void rename(std::size_t);
    void remove(std::size_t);
    Translations::TranslationService& translations_;
    WorkspaceState& state_;
    std::array<QLabel*, ThemeConstants::ProfilesPage::CategoryCount> titles_;
    std::array<QListWidget*, ThemeConstants::ProfilesPage::CategoryCount> lists_;
    std::array<QPushButton*, ThemeConstants::ProfilesPage::CategoryCount> rename_,remove_;
    Translations::TranslationService::Subscription languageSubscription_;
    Infrastructure::OperationProfiles::Subscription profileSubscription_;
};
}
