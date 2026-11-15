#include "SizeCodes.h"
#include "SectorConstants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine {
std::optional<std::uint8_t> trySectorSizeCode(std::int64_t bytes) noexcept {
    for(std::uint8_t code=0;code<=SectorConstants::MaximumIsoSizeCode;++code)if((SectorConstants::MinimumIsoSectorSize<<code)==bytes)return code;
    return {};
}
std::uint8_t sectorSizeCode(std::int64_t bytes){if(auto code=trySectorSizeCode(bytes))return *code;throw MediaError(DiagnosticCode::SectorUnsupportedSize,{std::to_string(bytes)});}
std::uint32_t sectorSizeBytes(std::uint8_t code){
    if(code>SectorConstants::MaximumIsoSizeCode)throw MediaError(DiagnosticCode::SectorUnsupportedSizeCode,{std::to_string(code),std::to_string(SectorConstants::MaximumIsoSizeCode)});
    return SectorConstants::MinimumIsoSectorSize<<code;
}
}
