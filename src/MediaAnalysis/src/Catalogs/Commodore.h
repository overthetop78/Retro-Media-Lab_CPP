#pragma once
#include "../Constants/Extensions.h"
#include "../Constants/Signatures.h"
#include "../Constants/RecognitionConstants.h"
namespace RetroMediaLab::MediaAnalysis::Catalogs::Commodore {
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
inline constexpr std::array<MediaContentRecognitionRule, 10> Rows{{
    {MediaFileSystemFamily::Commodore,MediaContentRecognitionPriority::Standard,Extensions::Txt,Rule0Signatures,MediaContentCategory::Text,MediaTextEncoding::Petscii,MediaExecutionKind::None,MediaPreviewKind::Text,Rule0Lengths},
    {MediaFileSystemFamily::Commodore,MediaContentRecognitionPriority::Standard,Extensions::Seq,Rule1Signatures,MediaContentCategory::Text,MediaTextEncoding::Petscii,MediaExecutionKind::None,MediaPreviewKind::Text,Rule1Lengths},
    {MediaFileSystemFamily::Commodore,MediaContentRecognitionPriority::Standard,Extensions::Prg,Rule2Signatures,MediaContentCategory::Program,MediaTextEncoding::NotApplicable,MediaExecutionKind::InterpretedProgram,MediaPreviewKind::None,Rule2Lengths},
    {MediaFileSystemFamily::Commodore,MediaContentRecognitionPriority::Standard,Extensions::Koa,Rule3Signatures,MediaContentCategory::Image,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Image,Rule3Lengths},
    {MediaFileSystemFamily::Commodore,MediaContentRecognitionPriority::Standard,Extensions::Art,Rule4Signatures,MediaContentCategory::Image,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Image,Rule4Lengths},
    {MediaFileSystemFamily::Commodore,MediaContentRecognitionPriority::Standard,Extensions::Iff,Rule5Signatures,MediaContentCategory::Image,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Image,Rule5Lengths},
    {MediaFileSystemFamily::Commodore,MediaContentRecognitionPriority::Standard,Extensions::Sid,Rule6Signatures,MediaContentCategory::Audio,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Audio,Rule6Lengths},
    {MediaFileSystemFamily::Commodore,MediaContentRecognitionPriority::Standard,Extensions::Mus,Rule7Signatures,MediaContentCategory::Audio,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Audio,Rule7Lengths},
    {MediaFileSystemFamily::Commodore,MediaContentRecognitionPriority::Standard,Extensions::Arc,Rule8Signatures,MediaContentCategory::Archive,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::None,Rule8Lengths},
    {MediaFileSystemFamily::Commodore,MediaContentRecognitionPriority::Standard,Extensions::Sda,Rule9Signatures,MediaContentCategory::Archive,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::None,Rule9Lengths}
}};
}
