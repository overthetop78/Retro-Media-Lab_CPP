#include "FmAddressMarks.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::FmAddressMarks {
const Pattern& forMark(std::uint8_t mark){if(mark<Constants::First||mark>Constants::Last)throw MediaError(DiagnosticCode::FmUnsupportedAddressMark,{std::to_string(mark)});return Constants::Patterns[mark-Constants::First];}
}
