#pragma once
#include "MessageInfo.h"
#include "VolumeInfo.h"
#include "MediaKind.h"
#include "RepresentationInfo.h"
#include <map>
#include <memory>
#include <optional>
namespace RetroMediaLab::MediaEngine {
class MediaDocument;
enum class FileSystemEntryKind {Directory,File,Link,Unknown};
struct FileTimestamp {std::int32_t year=0,month=0,day=0,hour=0,minute=0,second=0,nanosecond=0,offsetMinutes=0;bool operator==(const FileTimestamp&)const=default;};
struct DiagnosticInfo {MessageInfo message;std::vector<std::string> context;std::vector<DiagnosticInfo> causes;};
struct FileEntryAnalysis {std::string categoryId,iconId,typeResourceKey,extension,executionKindId,contentFormatId,textEncodingId,previewKindId,typeTranslationDomain;};
struct FileSystemEntryInfo {
    std::string name;FileSystemEntryKind kind=FileSystemEntryKind::Unknown;std::uint64_t size=0;
    std::optional<FileTimestamp> modified,created,accessed;
    std::string comment;std::uint32_t rawAttributes=0;std::int64_t storageReference=0;bool metadataValid=true;
    std::vector<FileSystemEntryInfo> children;
    std::shared_ptr<const std::vector<std::uint8_t>> content;
    std::optional<std::string> nativeTypeId;std::optional<std::uint64_t> occupiedSize;
    std::vector<std::string> attributes;std::optional<bool> dataValid;bool syntheticName=false;
    std::optional<std::string> linkTarget,commentResourceKey,commentTranslationDomain;std::vector<std::string> commentArguments;
    std::vector<DiagnosticInfo> diagnostics;std::map<std::string,std::string> metadata;std::optional<FileEntryAnalysis> analysis;
};
struct FileSystemVolumeInfo {
    std::string name,fileSystemId,fileSystemDisplayName;
    std::optional<std::string> fileSystemDisplayResourceKey,fileSystemDisplayTranslationDomain;
    std::vector<std::string> sourceDiagnostics;std::vector<MessageInfo> sourceMessages;
    std::optional<MessageInfo> nameRecipe;std::vector<std::string> fileSystemDisplayArguments;
    std::uint64_t capacity=0,freeBytes=0;std::optional<FileTimestamp> created,modified;
    std::vector<FileSystemEntryInfo> entries;std::vector<DiagnosticInfo> warnings;bool freeSpaceKnown=true;
    std::vector<std::string> attributes;std::optional<bool> bootable;std::optional<std::int32_t> diskNumber,diskCount;std::optional<std::string> diskNumberOrigin;
};
struct ExploredVolumeInfo {VolumeInfo descriptor;std::optional<std::string> readerId;std::shared_ptr<const FileSystemVolumeInfo> fileSystem;std::vector<DiagnosticInfo> diagnostics;};
struct ExploredMediaImage {
    // App can retain or pass this handle without including the internal document/MFS contracts.
    std::shared_ptr<const MediaDocument> document;
    std::string sourcePath,formatId;MediaKind mediaKind=MediaKind::Unknown;RepresentationKind representationKind=RepresentationKind::Sectors;
    std::map<std::string,std::string> metadata;std::vector<ExploredVolumeInfo> volumes;
    std::vector<std::string> diagnostics;std::vector<MessageInfo> messages;std::vector<DiagnosticInfo> explorationDiagnostics;
    std::optional<std::string> firstFileName;
};
}
