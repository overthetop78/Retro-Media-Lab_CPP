#pragma once

#include "EmulationConfigurationsPanel.h"

class QWidget;

namespace RetroMediaLab {
struct WorkspaceState;
namespace Translations { class TranslationService; }

class EmulationPreferencesDialog {
public:
    static void open(QWidget* parent, Translations::TranslationService&, WorkspaceState&, const VideoPresentation::ProfileStore&, EditSavedConfiguration);
};
}
