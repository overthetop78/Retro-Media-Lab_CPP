#pragma once
#include "../Constants/Extensions.h"
#include "../Constants/Signatures.h"
#include "../Constants/RecognitionConstants.h"
namespace RetroMediaLab::MediaAnalysis::Catalogs::Cpm {
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
inline constexpr std::array<MediaContentSignature, 0> Rule12Signatures{};
inline constexpr std::array<std::size_t, 0> Rule12Lengths{};
inline constexpr std::array<MediaContentSignature, 0> Rule13Signatures{};
inline constexpr std::array<std::size_t, 0> Rule13Lengths{};
inline constexpr std::array<MediaContentRecognitionRule, 14> Rows{{
    {MediaFileSystemFamily::Cpm,MediaContentRecognitionPriority::Standard,Extensions::Txt,Rule0Signatures,MediaContentCategory::Text,MediaTextEncoding::Ascii,MediaExecutionKind::None,MediaPreviewKind::Text,Rule0Lengths},
    {MediaFileSystemFamily::Cpm,MediaContentRecognitionPriority::Standard,Extensions::Doc,Rule1Signatures,MediaContentCategory::Document,MediaTextEncoding::Ascii,MediaExecutionKind::None,MediaPreviewKind::Text,Rule1Lengths},
    {MediaFileSystemFamily::Cpm,MediaContentRecognitionPriority::Standard,Extensions::Asm,Rule2Signatures,MediaContentCategory::SourceCode,MediaTextEncoding::Ascii,MediaExecutionKind::None,MediaPreviewKind::Text,Rule2Lengths},
    {MediaFileSystemFamily::Cpm,MediaContentRecognitionPriority::Standard,Extensions::Mac,Rule3Signatures,MediaContentCategory::SourceCode,MediaTextEncoding::Ascii,MediaExecutionKind::None,MediaPreviewKind::Text,Rule3Lengths},
    {MediaFileSystemFamily::Cpm,MediaContentRecognitionPriority::Standard,Extensions::For,Rule4Signatures,MediaContentCategory::SourceCode,MediaTextEncoding::Ascii,MediaExecutionKind::None,MediaPreviewKind::Text,Rule4Lengths},
    {MediaFileSystemFamily::Cpm,MediaContentRecognitionPriority::Standard,Extensions::C,Rule5Signatures,MediaContentCategory::SourceCode,MediaTextEncoding::Ascii,MediaExecutionKind::None,MediaPreviewKind::Text,Rule5Lengths},
    {MediaFileSystemFamily::Cpm,MediaContentRecognitionPriority::Standard,Extensions::H,Rule6Signatures,MediaContentCategory::SourceCode,MediaTextEncoding::Ascii,MediaExecutionKind::None,MediaPreviewKind::Text,Rule6Lengths},
    {MediaFileSystemFamily::Cpm,MediaContentRecognitionPriority::Standard,Extensions::Bas,Rule7Signatures,MediaContentCategory::BasicProgram,MediaTextEncoding::Unknown,MediaExecutionKind::None,MediaPreviewKind::BasicListing,Rule7Lengths},
    {MediaFileSystemFamily::Cpm,MediaContentRecognitionPriority::Standard,Extensions::Lib,Rule8Signatures,MediaContentCategory::Library,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Hexadecimal,Rule8Lengths},
    {MediaFileSystemFamily::Cpm,MediaContentRecognitionPriority::Standard,Extensions::Arc,Rule9Signatures,MediaContentCategory::Archive,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::None,Rule9Lengths},
    {MediaFileSystemFamily::Cpm,MediaContentRecognitionPriority::Standard,Extensions::Lbr,Rule10Signatures,MediaContentCategory::Archive,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::None,Rule10Lengths},
    {MediaFileSystemFamily::Cpm,MediaContentRecognitionPriority::Standard,Extensions::Com,Rule11Signatures,MediaContentCategory::Executable,MediaTextEncoding::NotApplicable,MediaExecutionKind::NativeExecutable,MediaPreviewKind::Hexadecimal,Rule11Lengths},
    {MediaFileSystemFamily::Cpm,MediaContentRecognitionPriority::Standard,Extensions::Cmd,Rule12Signatures,MediaContentCategory::Executable,MediaTextEncoding::NotApplicable,MediaExecutionKind::NativeExecutable,MediaPreviewKind::Hexadecimal,Rule12Lengths},
    {MediaFileSystemFamily::Cpm,MediaContentRecognitionPriority::Standard,Extensions::Sub,Rule13Signatures,MediaContentCategory::Command,MediaTextEncoding::Ascii,MediaExecutionKind::CommandScript,MediaPreviewKind::Text,Rule13Lengths}
}};
}
