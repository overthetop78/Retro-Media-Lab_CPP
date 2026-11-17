#pragma once
namespace RetroMediaLab::Infrastructure::MigrationConstants {
inline constexpr auto SourceUnrecognized="Explorer.PhysicalSectorsNoFileSystem";
inline constexpr auto SourceDestinationSame="Migration.SourceDestinationSame";
inline constexpr auto Failed="Migration.Failed",SourceFailed="Migration.SourceFailed";
inline constexpr auto Completed="Migration.Completed",AppDomain="App";
inline constexpr auto MigratedSuffix="-migrated";
}
