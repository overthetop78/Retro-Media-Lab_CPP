#pragma once

class QWidget;

namespace RetroMediaLab {
namespace Translations { class TranslationService; }

inline constexpr auto EmulationNetworkTitleKey = "Emulation.Network.Title";
QWidget* createEmulationNetworkPanel(QWidget*, Translations::TranslationService&);
}
