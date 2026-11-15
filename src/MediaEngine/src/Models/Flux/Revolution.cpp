#include "Flux.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Flux {
Revolution createRevolution(BitView bits,std::uint32_t cell,std::uint32_t index,bool trailing,std::stop_token token){
 checkCancellation(token);if(!cell)throw MediaError(DiagnosticCode::FluxZeroCell);if(!index)throw MediaError(DiagnosticCode::FluxZeroIndex);
 Revolution result{index,{}};std::uint64_t cells=0;
 const auto emit=[&]{if(cells>Constants::MaximumInterval/cell)throw MediaError(DiagnosticCode::FluxIntervalOverflow,{std::to_string(cells),std::to_string(cell)});result.intervals.push_back(std::uint32_t(cells*cell));};
 for(const auto bit:bits){checkCancellation(token);if(bit>1)throw MediaError(DiagnosticCode::FluxInvalidBit);++cells;if(!bit)continue;emit();cells=0;}
 if(trailing&&cells)emit();return result;
}
}
