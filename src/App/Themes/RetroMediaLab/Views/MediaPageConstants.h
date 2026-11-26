#pragma once
#include <array>
#include <utility>
namespace RetroMediaLab::MediaPageConstants {
inline constexpr auto Category="Explorer.Category",Preview="Explorer.Preview",Execution="Explorer.Execution";
inline constexpr auto CategoryPrefix="Explorer.Category.",ContentFormatPrefix="Explorer.ContentFormat.",PreviewPrefix="Explorer.Preview.",ExecutionPrefix="Explorer.Execution.";
inline constexpr auto ContentUnknown="Explorer.ContentFormat.Unknown",PreviewNone="Explorer.Preview.None",NotApplicable="NotApplicable",UnknownId="Unknown";
inline constexpr std::array TextEncodingIds{"Ascii","DosOem","Atascii","Petscii","Latin1","MacRoman","AppleAscii","Msx"};
inline constexpr std::array TextEncodingNames{"ASCII","DOS OEM","ATASCII","PETSCII","Latin-1","Mac Roman","Apple ASCII","MSX"};
inline constexpr std::array ExecutionIds{"NativeExecutable","InterpretedProgram","CommandScript"};
inline constexpr std::array RecordMetadataKeys{"recordCount","fullRecordCount","partialRecordCount","baudRates"};
inline constexpr std::array RecordMetadataLabels{"Explorer.Records","Explorer.FullRecords","Explorer.PartialRecords","Explorer.BaudRates"};
inline constexpr auto EndRecordKey="endRecordPresent",EndRecordLabel="Explorer.EndRecord",TrueValue="true",Yes="Common.Yes",No="Common.No";
inline constexpr int Padding = 8, Gap = 6, PollInterval = 40;
inline constexpr int NameColumn = 0, ModifiedColumn = 1, TypeColumn = 2, SizeColumn = 3, ColumnCount = 4;
inline constexpr int CellHorizontalInset = 8;
inline constexpr double ModifiedColumnShare = .22, TypeColumnShare = .21, SizeColumnShare = .12;
inline constexpr char ExtensionSeparator = '.';
// Seed the splitter with pixel-like values above the child minimum size hints.
inline constexpr int TreeInitialWidth = 200, ContentsInitialWidth = 700, DetailsInitialWidth = 300;
inline constexpr int PaneStretch = 1;
inline constexpr int IdentityIconSize = 40, DetailIconSize = 22;
inline constexpr auto ResourceIconPath = "Icons/FileTypes/%1.png";
inline constexpr auto FolderIcon = "folder", FileIcon = "file", FloppyIcon = "media-floppy", MediaIdentityPrefix = "media-", OpticalIcon = "cd", TapeIcon = "media";
inline constexpr std::array FileIconAliases{
    std::pair{"basic-program", "source-code"}, std::pair{"program", "executable"},
    std::pair{"audio", "audio-note"}, std::pair{"rom", "rom-memory"},
    std::pair{"disk-image", "disc-image"}, std::pair{"link", "shortcut"}};
inline constexpr auto AutomaticDetection = "Classification.Automatic";
inline constexpr auto OpenIcon = "open-image", RefreshIcon = "refresh", SaveIcon = "save", SettingsIcon = "preferences", ReadIcon = "read", ExplorerIcon = "explorer", VisualizationIcon = "visualize";
inline constexpr auto DetectedFormats = "Explorer.DetectedFormats", FormatsSeparator = " · ", EmptyValue = "—";
inline constexpr std::array SummaryKeys{"Explorer.Volume", "Explorer.System", "Explorer.Protection", "Explorer.FileSystem", "Explorer.Capacity", "Explorer.Free", "Explorer.Entries"};
inline constexpr int DetailsPaneIndex = 2;
inline constexpr auto ReadDisk="Explorer.ReadDisk",ReadConfirm="Explorer.ReadDiskConfirm",ReadConfirmTitle="Explorer.ReadDiskConfirmTitle",Stop="Common.Stop",StopConfirm="Operation.StopConfirm",StopTitle="Operation.StopTitle";
inline constexpr auto NoFileSystem="Explorer.PhysicalSectorsNoFileSystem";
inline constexpr auto Type="Explorer.Type",TextEncoding="Explorer.TextEncoding",ContentFormat="Explorer.ContentFormat",Warnings="Explorer.Warnings",PartitionTable="Visual.PartitionTableLabel",PartitionNumber="Visual.PartitionNumberLabel",PartitionType="Explorer.PartitionType",PartitionId="Explorer.PartitionId",Track="Explorer.Track",NoImage="Visual.NoFile";
inline constexpr auto Open = "Explorer.OpenImage";
inline constexpr auto Loading = "Explorer.Loading";
inline constexpr auto Unnamed = "Explorer.Unnamed";
inline constexpr auto SelectEntry = "Explorer.SelectEntry";
inline constexpr auto Directory = "Explorer.Directory";
inline constexpr auto File = "Explorer.File";
inline constexpr auto Link = "Explorer.Link";
inline constexpr auto Unknown = "Explorer.Unknown";
inline constexpr auto ExecutableCategory = "Executable", SuccessBrush = "Theme.SuccessBrush";
inline constexpr auto NoPartitionTable = "Visual.PartitionTable.None";
inline constexpr auto Session = "Explorer.Session";
inline constexpr auto Format = "Explorer.Format";
inline constexpr auto LoadFailed = "Explorer.LoadFailed";
inline constexpr auto Volume = "Explorer.Volume";
inline constexpr auto FileSystem = "Explorer.FileSystem";
inline constexpr auto Capacity = "Explorer.Capacity";
inline constexpr auto Free = "Explorer.Free";
inline constexpr auto Entries = "Explorer.Entries";
inline constexpr auto Comment = "Explorer.Comment";
inline constexpr auto Cancel = "Hardware.Cancel";
inline constexpr auto Argument = "{0}";
inline constexpr auto ArgumentPattern = "{%1}";
inline constexpr auto DetailSeparator = ": ";
inline constexpr auto LineSeparator = '\n';
inline constexpr std::array HeaderKeys{"Explorer.Name", "Explorer.Modified", "Explorer.Type", "Explorer.Size"};
}
