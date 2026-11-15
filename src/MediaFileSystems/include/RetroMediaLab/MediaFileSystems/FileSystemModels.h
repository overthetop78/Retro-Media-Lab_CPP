#pragma once
#include "Diagnostics.h"
#include "MessageRecipe.h"
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <stop_token>
#include <vector>
namespace RetroMediaLab::MediaFileSystems {
enum class EntryKind { Directory, File, Link, Unknown };
struct Timestamp {
    std::int32_t year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0, nanosecond = 0, offsetMinutes = 0;
    bool operator==(const Timestamp&) const = default;
};
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool validTimestamp(const Timestamp&) noexcept;
struct EntryAnalysis {
    std::string categoryId, iconId, typeResourceKey, extension, executionKindId, contentFormatId, textEncodingId, previewKindId, typeTranslationDomain;
};
struct FileSystemEntry {
    std::string name;
    EntryKind kind = EntryKind::Unknown;
    std::uint64_t size = 0;
    std::optional<Timestamp> modified, created, accessed;
    std::string comment;
    std::uint32_t rawAttributes = 0;
    std::int64_t storageReference = 0;
    bool metadataValid = true;
    std::vector<FileSystemEntry> children;
    std::optional<std::vector<std::uint8_t>> content;
    std::optional<std::string> nativeTypeId;
    std::optional<std::uint64_t> occupiedSize;
    std::vector<std::string> attributes;
    std::optional<bool> dataValid;
    bool syntheticName = false;
    std::optional<std::string> linkTarget;
    std::optional<std::string> commentResourceKey, commentTranslationDomain;
    std::vector<std::string> commentArguments;
    std::vector<Diagnostic> diagnostics;
    std::map<std::string,std::string> metadata;
    std::optional<EntryAnalysis> analysis;
};
struct FileSystemVolume {
    std::string name, fileSystemId, fileSystemDisplayName;
    std::optional<std::string> fileSystemDisplayResourceKey, fileSystemDisplayTranslationDomain;
    std::vector<std::string> sourceDiagnostics;
    std::vector<MessageRecipe> sourceMessages;
    std::optional<MessageRecipe> nameRecipe;
    std::vector<std::string> fileSystemDisplayArguments;
    std::uint64_t capacity = 0, freeBytes = 0;
    std::optional<Timestamp> created, modified;
    std::vector<FileSystemEntry> entries;
    std::vector<Diagnostic> warnings;
    bool freeSpaceKnown = true;
    std::vector<std::string> attributes;
    std::optional<bool> bootable;
    std::optional<std::int32_t> diskNumber, diskCount;
    std::optional<std::string> diskNumberOrigin;
};
struct AnalyzedFileSystemEntry {
    std::shared_ptr<const FileSystemEntry> entry;
    EntryAnalysis analysis;
    std::vector<AnalyzedFileSystemEntry> children;
};
RETROMEDIALAB_MEDIAFILESYSTEMS_API void analyzeEntries(std::string_view formatId, FileSystemVolume&, std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::vector<AnalyzedFileSystemEntry> analyzeEntryViews(std::string_view formatId, std::shared_ptr<const FileSystemVolume>, std::stop_token = {});
}
