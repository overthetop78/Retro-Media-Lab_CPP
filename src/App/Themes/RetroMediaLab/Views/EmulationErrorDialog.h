#pragma once

#include <QString>
#include <memory>

class QWidget;

namespace RetroMediaLab {
struct ThemeData;
namespace Emulation { class EmulationMessageException; }
namespace Translations { class TranslationService; }

// Returns text for a status message. Dialog messages are shown modally.
QString presentEmulationError(QWidget& owner, const std::shared_ptr<const ThemeData>& theme,
    const Translations::TranslationService& translations, const Emulation::EmulationMessageException& error,
    const QString& machineName);
}
