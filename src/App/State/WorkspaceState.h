#pragma once
#include "AppApi.h"
#include <QByteArray>
namespace RetroMediaLab {
// Session-owned state survives destruction of theme windows. Business models will live outside views.
struct WorkspaceState {
    QByteArray mainWindowGeometry;
    bool mainWindowMaximized = false;
    bool mainWindowOpen = true;
    RETROMEDIALAB_APP_API bool hasGeometry() const noexcept;
};
}
