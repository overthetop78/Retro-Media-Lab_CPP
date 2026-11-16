#pragma once
#include <string_view>
#include <cstdint>
namespace RetroMediaLab::MediaEngine::SimhTape::Constants {
inline constexpr std::string_view FormatId="tape.simh-tap",WriterId="tape-simh",Extension=".tap",ObjectKindKey="simhObjectKind",RecordErrorKey="simhRecordError",MarkerValueKey="simhMarkerValue",DataRecordKind="dataRecord",TapeMarkKind="tapeMark",EraseGapKind="eraseGap",EndOfMediumKind="endOfMedium",True="True",False="False";
inline constexpr int WordSize=4,RecordAlignment=2,HexRadix=16,HexWidth=8;
inline constexpr char ZeroPad='0';
inline constexpr std::uint32_t TapeMark=0x00000000,EraseGap=0xfffffffe,EndOfMedium=0xffffffff,ErrorFlag=0x80000000,ReservedLengthBits=0x7f000000,RecordLengthMask=0x00ffffff;
}
