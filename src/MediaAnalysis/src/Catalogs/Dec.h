#pragma once
#include "../Constants/Extensions.h"
#include "../Constants/Signatures.h"
#include "../Constants/RecognitionConstants.h"
namespace RetroMediaLab::MediaAnalysis::Catalogs::Dec {
inline constexpr std::array<MediaContentSignature, 0> Rule0Signatures{};
inline constexpr std::array<std::size_t, 0> Rule0Lengths{};
inline constexpr std::array<MediaContentSignature, 0> Rule1Signatures{};
inline constexpr std::array<std::size_t, 0> Rule1Lengths{};
inline constexpr std::array<MediaContentSignature, 0> Rule2Signatures{};
inline constexpr std::array<std::size_t, 0> Rule2Lengths{};
inline constexpr std::array<MediaContentSignature, 0> Rule3Signatures{};
inline constexpr std::array<std::size_t, 0> Rule3Lengths{};
inline constexpr std::array<MediaContentSignature, 0> Rule4Signatures{};
inline constexpr std::array<std::size_t, 0> Rule4Lengths{};
inline constexpr std::array<MediaContentSignature, 0> Rule5Signatures{};
inline constexpr std::array<std::size_t, 0> Rule5Lengths{};
inline constexpr std::array<MediaContentSignature, 0> Rule6Signatures{};
inline constexpr std::array<std::size_t, 0> Rule6Lengths{};
inline constexpr std::array<MediaContentSignature, 0> Rule7Signatures{};
inline constexpr std::array<std::size_t, 0> Rule7Lengths{};
inline constexpr std::array<MediaContentSignature, 0> Rule8Signatures{};
inline constexpr std::array<std::size_t, 0> Rule8Lengths{};
inline constexpr std::array<MediaContentSignature, 0> Rule9Signatures{};
inline constexpr std::array<std::size_t, 0> Rule9Lengths{};
inline constexpr std::array<MediaContentSignature, 0> Rule10Signatures{};
inline constexpr std::array<std::size_t, 0> Rule10Lengths{};
inline constexpr std::array<MediaContentSignature, 0> Rule11Signatures{};
inline constexpr std::array<std::size_t, 0> Rule11Lengths{};
inline constexpr std::array<MediaContentRecognitionRule, 12> Rows{{
    {MediaFileSystemFamily::Dec,MediaContentRecognitionPriority::Standard,Extensions::Txt,Rule0Signatures,MediaContentCategory::Text,MediaTextEncoding::Ascii,MediaExecutionKind::None,MediaPreviewKind::Text,Rule0Lengths},
    {MediaFileSystemFamily::Dec,MediaContentRecognitionPriority::Standard,Extensions::Mac,Rule1Signatures,MediaContentCategory::SourceCode,MediaTextEncoding::Ascii,MediaExecutionKind::None,MediaPreviewKind::Text,Rule1Lengths},
    {MediaFileSystemFamily::Dec,MediaContentRecognitionPriority::Standard,Extensions::For,Rule2Signatures,MediaContentCategory::SourceCode,MediaTextEncoding::Ascii,MediaExecutionKind::None,MediaPreviewKind::Text,Rule2Lengths},
    {MediaFileSystemFamily::Dec,MediaContentRecognitionPriority::Standard,Extensions::Bas,Rule3Signatures,MediaContentCategory::BasicProgram,MediaTextEncoding::Unknown,MediaExecutionKind::None,MediaPreviewKind::BasicListing,Rule3Lengths},
    {MediaFileSystemFamily::Dec,MediaContentRecognitionPriority::Standard,Extensions::Cmd,Rule4Signatures,MediaContentCategory::Command,MediaTextEncoding::Ascii,MediaExecutionKind::CommandScript,MediaPreviewKind::Text,Rule4Lengths},
    {MediaFileSystemFamily::Dec,MediaContentRecognitionPriority::Standard,Extensions::Com,Rule5Signatures,MediaContentCategory::Command,MediaTextEncoding::Ascii,MediaExecutionKind::CommandScript,MediaPreviewKind::Text,Rule5Lengths},
    {MediaFileSystemFamily::Dec,MediaContentRecognitionPriority::Standard,Extensions::Bup,Rule6Signatures,MediaContentCategory::Archive,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::None,Rule6Lengths},
    {MediaFileSystemFamily::Dec,MediaContentRecognitionPriority::Standard,Extensions::Sav,Rule7Signatures,MediaContentCategory::Executable,MediaTextEncoding::NotApplicable,MediaExecutionKind::NativeExecutable,MediaPreviewKind::Hexadecimal,Rule7Lengths},
    {MediaFileSystemFamily::Dec,MediaContentRecognitionPriority::Standard,Extensions::Lda,Rule8Signatures,MediaContentCategory::ObjectCode,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Hexadecimal,Rule8Lengths},
    {MediaFileSystemFamily::Dec,MediaContentRecognitionPriority::Standard,Extensions::Rel,Rule9Signatures,MediaContentCategory::ObjectCode,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Hexadecimal,Rule9Lengths},
    {MediaFileSystemFamily::Dec,MediaContentRecognitionPriority::Standard,Extensions::Obj,Rule10Signatures,MediaContentCategory::ObjectCode,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Hexadecimal,Rule10Lengths},
    {MediaFileSystemFamily::Dec,MediaContentRecognitionPriority::Standard,Extensions::Sys,Rule11Signatures,MediaContentCategory::System,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Hexadecimal,Rule11Lengths}
}};
}
