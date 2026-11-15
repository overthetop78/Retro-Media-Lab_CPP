#pragma once
#include <string_view>
#include <cstddef>
namespace RetroMediaLab::MediaFileSystems::AtariMetadata::ClkConstants {
inline constexpr std::size_t HeaderSector=361,FirstDirectory=362,LastDirectory=393,EntryLength=32,DataLength=126,IconLength=572,SectorSize=128,NameLength=16,StartOffset=16,TypeOffset=19,LengthOffset=20,FirstOccupied=360;
inline constexpr char ActiveUpper='X',ActiveLower='x',Null='\0',Space=' ',AsciiReplacement='?';
inline constexpr unsigned AsciiMaximum=0x7f;
inline constexpr std::string_view Signature="PRINT SHOP:CLK!",Id="atari-clk-graphics-library",Display="Atari CLK graphics library",NativeType="atari-clk-graphic",Graphic="graphics",Library="graphics-library",CommentKey="MediaFileSystems.AtariClkGraphic",Domain="MediaFileSystems",ContainerKey="container",StartKey="startSector",TypeKey="typeId",LengthKey="declaredLength";
}
