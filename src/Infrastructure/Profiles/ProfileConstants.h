#pragma once
#include <array>
namespace RetroMediaLab::Infrastructure::ProfileConstants {
inline constexpr auto Section="profiles",Id="id",Operation="operation",Name="name",Values="values",Enabled="enabledOptions";
inline constexpr auto DefaultPrefix="default-",DefaultName="Default",Expert="expert",Format="format",Extension="extension",Folder="folder",Result="result",Raw="raw",Known="known";
inline constexpr auto Tags="tags",FormatPrefix="format:",ExtensionsPrefix="extensions:",OptionPrefix="--";
inline constexpr char ExtensionSeparator=',';
inline constexpr auto NameRequired="Profile.NameRequired",Duplicate="Profile.DuplicateName",SystemRename="Profile.SystemRename",SystemDelete="Profile.SystemDelete";
inline constexpr auto Rename="Profiles.Rename",Delete="Profile.SystemDelete",DeleteConfirm="Profile.DeleteConfirm",Title="Profile.Title",NameLabel="Profile.Name",DefaultLabel="Profile.Default";
inline constexpr auto DeleteAction="Common.Delete",Cancel="Common.Cancel",Yes="Common.Yes",No="Common.No",Argument="{0}";
inline constexpr std::array Operations{"Read","Write","Convert"};
inline constexpr std::size_t OperationCount=Operations.size();
}
