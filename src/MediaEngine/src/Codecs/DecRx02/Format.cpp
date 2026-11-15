#include "Rx02.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::DecRx02 {
namespace {
constexpr DataMark makeMark(std::uint8_t mark,DataEncoding encoding,bool deleted){const bool m2=encoding==DataEncoding::M2Fm;return {mark,FmAddressMarks::Constants::Patterns[mark-FmAddressMarks::Constants::First],encoding,deleted,m2?Constants::M2FmSectorSize:Constants::FmSectorSize,m2?Constants::M2FmSectorSizeCode:Constants::FmSectorSizeCode};}
constexpr std::array marks{makeMark(Constants::FmDeletedDataMark,DataEncoding::Fm,true),makeMark(Constants::M2FmDataMark,DataEncoding::M2Fm,false),makeMark(Constants::AlternateFaMark,DataEncoding::Fm,false),makeMark(Constants::FmDataMark,DataEncoding::Fm,false),makeMark(Constants::AlternateFcMark,DataEncoding::Fm,false),makeMark(Constants::M2FmDeletedDataMark,DataEncoding::M2Fm,true)};
}
std::span<const DataMark> dataMarks()noexcept{return marks;}
SectorFormat encodingForSize(std::int32_t size){if(size==Constants::FmSectorSize)return {DataEncoding::Fm,Constants::FmSectorSizeCode};if(size==Constants::M2FmSectorSize)return {DataEncoding::M2Fm,Constants::M2FmSectorSizeCode};throw MediaError(DiagnosticCode::Rx02SectorSize,{std::to_string(size)});}
const DataMark& dataMarkFor(DataEncoding encoding,bool deleted)noexcept{const auto index=encoding==DataEncoding::M2Fm?(deleted?Constants::M2FmDeletedIndex:Constants::M2FmNormalIndex):(deleted?Constants::FmDeletedIndex:Constants::FmNormalIndex);return marks[index];}
}
