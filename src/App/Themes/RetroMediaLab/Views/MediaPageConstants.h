#pragma once
#include <array>
namespace RetroMediaLab::MediaPageConstants {
inline constexpr int Padding = 8, Gap = 6, PollInterval = 40;
inline constexpr int NameColumn = 0, ModifiedColumn = 1, TypeColumn = 2, SizeColumn = 3, ColumnCount = 4;
inline constexpr int TreeStretch = 1, ContentsStretch = 4, DetailsStretch = 2;
inline constexpr int DetailsPaneIndex = 2;
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
