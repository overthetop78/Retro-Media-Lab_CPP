#pragma once

#include <functional>

class QWidget;
class QString;

namespace RetroMediaLab {
struct WorkspaceState;
namespace Translations { class TranslationService; }
namespace Emulation { struct Configuration; }
namespace VideoPresentation { class ProfileStore; }

using EditSavedConfiguration = std::function<void(QWidget*, const QString&, const Emulation::Configuration&)>;
QWidget* createEmulationConfigurationsPanel(QWidget*, Translations::TranslationService&, WorkspaceState&,
    const VideoPresentation::ProfileStore&, EditSavedConfiguration);
}
