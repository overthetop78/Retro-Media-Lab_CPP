#include "Serial.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Fsk {
DecodeResult::DecodeResult(std::vector<std::uint8_t> bytes,int valid,int tested,std::vector<int> positions):bytes_(std::move(bytes)),valid_(valid),tested_(tested),positions_(std::move(positions)) {
    if(valid<0||tested<0||valid>tested)throw MediaError(DiagnosticCode::FskInvalid);
}
}
