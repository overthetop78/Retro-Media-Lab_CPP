#pragma once
namespace RetroMediaLab::ConversionPageConstants {
inline constexpr int Padding=8,Gap=6,AdvancedHeight=180,OptionWidth=125,FormatRole=256,ExtensionRole=257,ProgressMaximum=100;
inline constexpr int NameColumn=2,TagsColumn=3,SourceStretch=2;
inline constexpr auto Profile="Profile.Title",DefaultProfile="Profile.Default",Save="Common.Save",Reset="Common.Reset",SaveProfile="Profile.SaveTitle",ProfileName="Profile.Name",ReplaceProfile="Profile.Replace",ReservedProfile="Profile.SystemRename";
inline constexpr auto Source="Conversion.Source",Name="Conversion.OutputNames",AddTags="Conversion.AddTags",Selected="Conversion.Selected",Formats="Conversion.CommonFormats";
inline constexpr auto Browse="Common.Browse",Execute="Common.Execute",Stop="Common.Stop",Cancel="Common.Cancel",Yes="Common.Yes",No="Common.No";
inline constexpr auto Advanced="Common.Advanced",Expert="Common.ExpertArguments",Title="Conversion.Title",ShowRare="Conversion.ShowRare",OutputTracks="Advanced.OutputTracks";
inline constexpr auto LossTitle="Conversion.SequentialLossesTitle",LossConfirm="Conversion.SequentialLossesConfirm",StopTitle="Operation.StopTitle",StopConfirm="Operation.StopConfirm";
inline constexpr auto Conflict="Conflict.Title",Overwrite="Conflict.AllOverwrite",Skip="Conflict.AllSkip",Number="Conflict.AllNumber";
inline constexpr auto Sector="Conversion.Fidelity.SectorData",Tracks="Conversion.Fidelity.ReconstructedTracks",Flux="Conversion.Fidelity.PreservedFlux";
inline constexpr auto Separator=" · ",NewLine="\n",Argument="{0}",SourceInput="conversionSource",NameInput="conversionName",FormatTree="conversionFormats";
}
