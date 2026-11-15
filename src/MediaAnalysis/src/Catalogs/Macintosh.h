#pragma once
#include "../Constants/Extensions.h"
#include "../Constants/Signatures.h"
#include "../Constants/RecognitionConstants.h"
namespace RetroMediaLab::MediaAnalysis::Catalogs::Macintosh {
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
inline constexpr std::array<MediaContentRecognitionRule, 11> Rows{{
    {MediaFileSystemFamily::Macintosh,MediaContentRecognitionPriority::Standard,Extensions::Txt,Rule0Signatures,MediaContentCategory::Text,MediaTextEncoding::MacRoman,MediaExecutionKind::None,MediaPreviewKind::Text,Rule0Lengths},
    {MediaFileSystemFamily::Macintosh,MediaContentRecognitionPriority::Standard,Extensions::Pict,Rule1Signatures,MediaContentCategory::Image,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Image,Rule1Lengths},
    {MediaFileSystemFamily::Macintosh,MediaContentRecognitionPriority::Standard,Extensions::Pct,Rule2Signatures,MediaContentCategory::Image,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Image,Rule2Lengths},
    {MediaFileSystemFamily::Macintosh,MediaContentRecognitionPriority::Standard,Extensions::Snd,Rule3Signatures,MediaContentCategory::Audio,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Audio,Rule3Lengths},
    {MediaFileSystemFamily::Macintosh,MediaContentRecognitionPriority::Standard,Extensions::Aiff,Rule4Signatures,MediaContentCategory::Audio,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Audio,Rule4Lengths},
    {MediaFileSystemFamily::Macintosh,MediaContentRecognitionPriority::Standard,Extensions::Aif,Rule5Signatures,MediaContentCategory::Audio,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Audio,Rule5Lengths},
    {MediaFileSystemFamily::Macintosh,MediaContentRecognitionPriority::Standard,Extensions::Sit,Rule6Signatures,MediaContentCategory::Archive,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::None,Rule6Lengths},
    {MediaFileSystemFamily::Macintosh,MediaContentRecognitionPriority::Standard,Extensions::Cpt,Rule7Signatures,MediaContentCategory::Archive,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::None,Rule7Lengths},
    {MediaFileSystemFamily::Macintosh,MediaContentRecognitionPriority::Standard,Extensions::Hqx,Rule8Signatures,MediaContentCategory::Archive,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::None,Rule8Lengths},
    {MediaFileSystemFamily::Macintosh,MediaContentRecognitionPriority::Standard,Extensions::Bin,Rule9Signatures,MediaContentCategory::Archive,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::None,Rule9Lengths},
    {MediaFileSystemFamily::Macintosh,MediaContentRecognitionPriority::Standard,Extensions::App,Rule10Signatures,MediaContentCategory::Executable,MediaTextEncoding::NotApplicable,MediaExecutionKind::NativeExecutable,MediaPreviewKind::Hexadecimal,Rule10Lengths}
}};
}
