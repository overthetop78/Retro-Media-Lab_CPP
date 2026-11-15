#pragma once
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
namespace RetroMediaLab::MediaAnalysis {
enum class MediaContentCategory {
    File,
    Text,
    Document,
    SourceCode,
    BasicProgram,
    BootProgram,
    Program,
    Executable,
    Command,
    System,
    ObjectCode,
    Data,
    Configuration,
    Library,
    Image,
    Audio,
    Rom,
    Media,
    Archive,
    DiskImage,
    Link,
    Font
};
enum class MediaContentFormat {
    Unknown,
    Empty,
    PlainText,
    BasicProgram,
    NativeExecutable,
    CommandScript,
    ObjectCode,
    SystemBinary,
    StructuredData,
    Configuration,
    Library,
    Image,
    Audio,
    Rom,
    Media,
    Archive,
    DiskImage,
    Font
};
enum class MediaContentRecognitionPriority {
    Primary,
    Standard,
    Fallback
};
enum class MediaContentSearchDirection {
    Start,
    End,
    SearchStart,
    SearchEnd
};
enum class MediaEntryKind {
    Directory,
    File,
    Link,
    Unknown
};
enum class MediaExecutionKind {
    None,
    NativeExecutable,
    InterpretedProgram,
    CommandScript
};
enum class MediaFileSystemFamily {
    Unknown,
    Amiga,
    IbmPc,
    Atari,
    AtariTos,
    Atari8Bit,
    AppleDos,
    ProDos,
    Macintosh,
    Lisa,
    Commodore,
    Cpm,
    BbcMicro,
    Dec,
    Msx,
    Ucsd
};
enum class MediaPreviewKind {
    None,
    Text,
    BasicListing,
    Image,
    Audio,
    Hexadecimal,
    ExternalApplication
};
enum class MediaTextEncoding {
    NotApplicable,
    Unknown,
    Ascii,
    DosOem,
    Atascii,
    Petscii,
    Latin1,
    MacRoman,
    AppleAscii,
    Msx
};
using Bytes = std::span<const std::uint8_t>;
// Absent content differs from a successfully extracted empty file.
using Content = std::optional<Bytes>;
struct MediaContentSignature {
    std::optional<std::size_t> position;
    MediaContentSearchDirection direction;
    std::span<const Bytes> byteGroups;
};
struct MediaContentRecognitionRule {
    std::optional<MediaFileSystemFamily> family;
    MediaContentRecognitionPriority priority;
    std::string_view extension;
    std::span<const MediaContentSignature> signatures;
    MediaContentCategory category;
    MediaTextEncoding textEncoding;
    MediaExecutionKind executionKind;
    MediaPreviewKind previewKind;
    std::span<const std::size_t> contentLengths{};
};
struct MediaContentTypeDefinition {
    std::optional<MediaFileSystemFamily> family;
    std::string extension;
    MediaContentCategory category;
    std::string typeResourceKey;
    MediaExecutionKind executionKind;
    std::string iconId;
    MediaContentFormat contentFormat;
    MediaTextEncoding textEncoding;
    MediaPreviewKind previewKind;
    bool operator==(const MediaContentTypeDefinition&) const = default;
};
}
