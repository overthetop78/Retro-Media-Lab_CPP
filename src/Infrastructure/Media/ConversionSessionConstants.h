#pragma once
namespace RetroMediaLab::Infrastructure::ConversionSessionConstants {
inline constexpr auto SourceRequired="Conversion.SourceRequired",NameRequired="Conversion.NameRequired",SelectOutput="Conversion.CheckOutput";
inline constexpr auto Unavailable="Conversion.EngineInternalUnavailable",Summary="Conversion.Summary",Failed="Conversion.Failed";
inline constexpr auto InternalStart="Conversion.EngineInternalStart",ExternalStart="Conversion.EngineExternal",Complete="Conversion.EngineInternalComplete";
inline constexpr auto Empty="",Separator="\n",Numbered="%1 (%2)%3",Codec="--codec",Encode="encode",Application="Retro Media Lab";
inline constexpr char ExtensionSeparator='.';
inline constexpr int MaximumOutputLines=1024*1024;
inline constexpr double Percent=100.0;
}
