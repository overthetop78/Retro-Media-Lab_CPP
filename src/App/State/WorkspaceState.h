#pragma once
#include <Constants/AppConstants.h>
#include "AppApi.h"
#include <QByteArray>
#include <QStringList>
#include "Shell/ConsoleState.h"
#include <functional>
#include <memory>
#include <vector>
namespace RetroMediaLab {
// Session-owned state survives destruction of theme windows. Business models will live outside views.
struct WorkspaceState {
    QStringList openPageIds{AppConstants::Read, AppConstants::Write, AppConstants::Convert, AppConstants::Visualize, AppConstants::Explorer, AppConstants::Tools, AppConstants::Emulation};
    QString activePageId = AppConstants::Read;
    ConsoleState console;
    bool preferencesOpen = false;
    QString preferencesSection = AppConstants::General;
    QByteArray preferencesGeometry;
    QByteArray mainWindowGeometry;
    bool mainWindowMaximized = false;
    bool mainWindowOpen = true;
    RETROMEDIALAB_APP_API bool hasGeometry() const noexcept;
    using DirectorySubscription = std::shared_ptr<const std::function<void()>>;
    const QString& defaultImageDirectory() const noexcept { return defaultImageDirectory_; }
    RETROMEDIALAB_APP_API void setDefaultImageDirectory(QString);
    RETROMEDIALAB_APP_API DirectorySubscription subscribeToDefaultImageDirectory(std::function<void()>);
private:
    QString defaultImageDirectory_;
    std::vector<std::weak_ptr<const std::function<void()>>> directoryObservers_;
};
}
