#pragma once
namespace RetroMediaLab::HexGridConstants {
inline constexpr int WideColumns=16,NarrowColumns=8,HexBase=16,HexDigits=2,OffsetDigits=8,CellPadding=2,GridLine=1;
inline constexpr char Padding='0',Unprintable='.';
inline constexpr int FirstPrintable=32,LastPrintable=126;
inline constexpr auto AsciiHeader="ASCII",CharacterSample="M";
inline constexpr auto OwnSectionMetricsProperty="rmlContentSizedSections";
inline constexpr auto Sample="FF",OffsetSample="00000000";
}
