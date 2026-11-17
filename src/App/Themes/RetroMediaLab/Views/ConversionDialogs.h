#pragma once
#include "Media/ConversionConflicts.h"
#include <RetroMediaLab/MediaEngine/MediaReading.h>
#include <RetroMediaLab/Translations/TranslationService.h>
#include <optional>
class QWidget;
namespace RetroMediaLab {
std::optional<MediaEngine::TapePcmProfile> requestPcmProfile(Translations::TranslationService&,QWidget*,bool input,std::optional<MediaEngine::TapePcmProfile> initial={});
std::optional<Infrastructure::ConversionConflictDecisions> requestConversionConflicts(Translations::TranslationService&,QWidget*,std::span<const MediaEngine::ConversionOutput>);
}
