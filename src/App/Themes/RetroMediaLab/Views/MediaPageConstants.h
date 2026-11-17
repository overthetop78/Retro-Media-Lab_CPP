#pragma once
#include <array>
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
inline constexpr int TreeStretch = 1, ContentsStretch = 4, DetailsStretch = 2;
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
inline constexpr auto Automatic = "Explorer.Automatic";
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
