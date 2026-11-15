#pragma once
#include "FileSystemModels.h"
namespace RetroMediaLab::MediaFileSystems {
struct MigrationEntry {
    std::string sourcePath, targetName;
    EntryKind kind = EntryKind::Unknown;
    std::optional<std::vector<std::uint8_t>> content;
    std::optional<Timestamp> modified;
    std::string comment;
    std::uint32_t rawAttributes = 0;
    bool metadataValid = true;
    std::vector<MigrationEntry> children;
};
struct MigrationPlan {
    std::string sourceFileSystemId, targetFileSystemId, volumeName;
    std::vector<MigrationEntry> entries;
};
}
