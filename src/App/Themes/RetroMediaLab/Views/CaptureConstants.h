#pragma once
#include <array>
namespace RetroMediaLab::CaptureConstants {
inline constexpr int ProgressRowHeight=22,ProgressLabelWidth=64,ProgressCountWidth=70,ProgressCellGap=1;
inline constexpr double ProgressMinimumWidth=1;
inline constexpr auto ProgressSide="Visual.Side",ProgressDivider=" / ",NumericArgument="{%1:N0}";
inline constexpr auto ProgressSuccessBrush="Theme.SectorDataBrush",ProgressRetryBrush="Theme.WarningBrush",ProgressFailedBrush="Theme.ErrorBrush";
inline constexpr auto ReadRunning="Status.Running", ReadSuccess="Status.Success";
inline constexpr int CaptureCardPadding=12, CaptureCardRadius=8, CaptureCardGap=10, CaptureActionGap=8;
inline constexpr unsigned CaptureTextRgb=0x222222;
inline constexpr std::array<unsigned,2> RunningCaptureColors{0xE7F0FF,0x4D76E8},
    CompletedCaptureColors{0xE5F5EA,0x42A566}, ArchivedCaptureColors{0xF1F3F5,0xC7CDD4};
inline constexpr auto ScpBanner="Read.ScpBannerSummary",ScpUnavailable="Read.ScpSummaryUnavailable",ChecksumValid="Visual.ChecksumValid",ChecksumInvalid="Visual.ChecksumInvalid";
inline constexpr int Padding=8,Gap=6,OptionWidth=120,AdvancedHeight=260,SequenceWidth=75;
inline constexpr int NameColumn=2;
inline constexpr int ComboMinimumLength=10;
inline constexpr auto ReservedProfile="Profile.SystemRename";
inline constexpr auto Extension="Read.Extension";
inline constexpr auto Advanced="Common.Advanced",Expert="Common.ExpertArguments",Browse="Common.Browse",Execute="Common.Execute",Stop="Common.Stop";
inline constexpr auto Profile="Profile.Title",DefaultProfile="Profile.Default",Save="Common.Save",Reset="Common.Reset",SaveProfile="Profile.SaveTitle",ProfileName="Profile.Name",ReplaceProfile="Profile.Replace";
inline constexpr auto Yes="Common.Yes",No="Common.No",Source="Write.Source",Folder="Read.Folder",Name="Read.FileName";
inline constexpr auto Raw="Read.RawScp",Known="Read.KnownFormat",Format="Write.Detected",Device="Hardware.Floppy",AutoNumber="Read.AutoNumber",Digits="Read.Digits",Letters="Read.Letters",Next="Read.Next";
inline constexpr auto OpenVisual="Read.OpenVisualize",OpenExplorer="Read.OpenExplore",WriteConfirm="Write.Confirm",WriteTitle="Write.ConfirmTitle",ReadTitle="Read.Title",WritePageTitle="Write.Title";
inline constexpr auto VerifyOff="Write.VerifyOff",VerifyOn="Write.VerifyOn",FileExists="Read.FileExistsTitle",Overwrite="Read.ConflictOverwrite",NextNumber="Read.ConflictNextNumber",EditName="Read.ConflictEditName";
inline constexpr auto StopConfirm="Operation.StopConfirm",StopTitle="Operation.StopTitle",CleanupFailed="Read.CancelledFileDeleteFailed";
inline constexpr auto Argument="{%1}",Separator=" · ";
inline constexpr auto NewLine='\n';
inline constexpr auto PageRead="readPage",PageWrite="writePage",SourceInput="captureSource",DirectoryInput="captureDirectory",NameInput="captureFileName",FormatInput="captureFormat",DeviceInput="captureDevice";
inline constexpr auto Automatic="Explorer.Automatic",MissingProvider="Engines.MissingProvider";
}
