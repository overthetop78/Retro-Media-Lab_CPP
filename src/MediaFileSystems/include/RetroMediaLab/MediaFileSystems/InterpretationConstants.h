#pragma once
#include <cstddef>
namespace RetroMediaLab::MediaFileSystems::InterpretationConstants {
inline constexpr std::size_t MinimumWarningThreshold=3,MinimumValidatedCatalogEntries=3;
inline constexpr char FieldSeparator='\0',EntrySeparator='\x1f',PathSeparator='/',FormatSeparator='.';
}
