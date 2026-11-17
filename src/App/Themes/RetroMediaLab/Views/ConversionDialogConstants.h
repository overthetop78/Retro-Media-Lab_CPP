#pragma once
#include <array>
namespace RetroMediaLab::ConversionDialogConstants {
inline constexpr int Width=640,Height=340,Padding=10,Gap=6,MinimumRate=8000,MaximumRate=192000;
inline constexpr std::array SampleRates{22050,44100,48000,96000,192000};
inline constexpr std::array Bits{8,16,24,32};
inline constexpr std::array Channels{1,2};
inline constexpr auto Input="Emulation.Tape.Pcm.Input",Output="Emulation.Tape.Pcm.Output",Rate="Emulation.Tape.Pcm.SampleRate",ChannelLabel="Emulation.Tape.Pcm.Channels",BitLabel="Emulation.Tape.Pcm.Bits",Signed="Emulation.Tape.Pcm.Signed",Endian="Emulation.Tape.Pcm.LittleEndian";
inline constexpr auto Continue="Common.Continue",Cancel="Common.Cancel",Conflict="Conflict.Title",Help="Conflict.Help",File="Conflict.File",Action="Conflict.Action",Overwrite="Conflict.Overwrite",Skip="Conflict.Skip",Number="Conflict.Number";
inline constexpr auto AllOverwrite="Conflict.AllOverwrite",AllSkip="Conflict.AllSkip",AllNumber="Conflict.AllNumber";
inline constexpr int FileColumn=0,ActionColumn=1,ColumnCount=2;
}
