#pragma once
#include <array>
#include <utility>
namespace RetroMediaLab::MigrationDialogConstants {
inline constexpr auto ObjectName="migrationDialog",Open="Migration.Open",Title="Migration.Title",Source="Migration.Source",Destination="Migration.Destination",Output="Migration.Output",NoSource="Migration.NoSource",SourceSummary="Migration.SourceSummary";
inline constexpr auto Waiting="Migration.Waiting",Compatible="Migration.Compatible",Incompatible="Migration.Incompatible",CompatibleWithLosses="Migration.CompatibleWithLosses",AcceptLosses="Migration.AcceptLosses";
inline constexpr auto Severity="Migration.Severity",Path="Migration.Path",Description="Migration.Description",Blocking="Migration.Blocking",MetadataLoss="Migration.MetadataLoss",Execute="Migration.Execute",Overwrite="Migration.OverwriteConfirm";
inline constexpr auto Browse="Common.Browse",Close="Preferences.Close",Stop="Common.Stop",Argument="{0}";
inline constexpr int SourceSummaryRow=2,DestinationLabelRow=3,DestinationInputRow=4,OutputLabelRow=5,OutputInputRow=6,ColumnCount=3,FullSpan=2;
inline constexpr int Padding=8,Gap=6,Width=840,Height=540,MinimumWidth=480,MinimumHeight=320,ScreenInset=48,DescriptionColumn=2;
inline constexpr std::array<std::pair<const char*,const char*>,11> TargetLabels{{
 {"amiga.amigados","Migration.Target.Amiga880"},{"atarist.720","Migration.Target.Atari720"},{"ibm.720","Migration.Target.Ibm720"},{"msx.2dd","Migration.Target.Msx720"},
 {"apple2.appledos.140","Migration.Target.AppleDos140"},{"apple2.prodos.140","Migration.Target.ProDos140"},{"apple2.prodos.800","Migration.Target.ProDos800"},{"apple3.sos","Migration.Target.Sos800"},
 {"commodore.1541","Migration.Target.Commodore1541"},{"commodore.1571","Migration.Target.Commodore1571"},{"commodore.1581","Migration.Target.Commodore1581"}}};
inline constexpr std::array<const char*,10> LossLabels{"Migration.Loss.MissingContent","Migration.Loss.UnsupportedEntryKind","Migration.Loss.InvalidName","Migration.Loss.NameTooLong","Migration.Loss.NameCollision","Migration.Loss.FileTooLarge","Migration.Loss.InvalidMetadata","Migration.Loss.ModifiedDate","Migration.Loss.Comment","Migration.Loss.Attributes"};
}
