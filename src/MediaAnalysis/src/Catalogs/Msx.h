#pragma once
#include "../Constants/Extensions.h"
#include "../Constants/Signatures.h"
#include "../Constants/RecognitionConstants.h"
namespace RetroMediaLab::MediaAnalysis::Catalogs::Msx {
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
    {MediaFileSystemFamily::Msx,MediaContentRecognitionPriority::Standard,Extensions::Txt,Rule0Signatures,MediaContentCategory::Text,MediaTextEncoding::Msx,MediaExecutionKind::None,MediaPreviewKind::Text,Rule0Lengths},
    {MediaFileSystemFamily::Msx,MediaContentRecognitionPriority::Standard,Extensions::Doc,Rule1Signatures,MediaContentCategory::Document,MediaTextEncoding::Msx,MediaExecutionKind::None,MediaPreviewKind::Text,Rule1Lengths},
    {MediaFileSystemFamily::Msx,MediaContentRecognitionPriority::Standard,Extensions::Asc,Rule2Signatures,MediaContentCategory::Text,MediaTextEncoding::Ascii,MediaExecutionKind::None,MediaPreviewKind::Text,Rule2Lengths},
    {MediaFileSystemFamily::Msx,MediaContentRecognitionPriority::Standard,Extensions::Bas,Rule3Signatures,MediaContentCategory::BasicProgram,MediaTextEncoding::Unknown,MediaExecutionKind::None,MediaPreviewKind::BasicListing,Rule3Lengths},
    {MediaFileSystemFamily::Msx,MediaContentRecognitionPriority::Standard,Extensions::Sc2,Rule4Signatures,MediaContentCategory::Image,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Image,Rule4Lengths},
    {MediaFileSystemFamily::Msx,MediaContentRecognitionPriority::Standard,Extensions::Sc5,Rule5Signatures,MediaContentCategory::Image,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Image,Rule5Lengths},
    {MediaFileSystemFamily::Msx,MediaContentRecognitionPriority::Standard,Extensions::Sc7,Rule6Signatures,MediaContentCategory::Image,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Image,Rule6Lengths},
    {MediaFileSystemFamily::Msx,MediaContentRecognitionPriority::Standard,Extensions::Sc8,Rule7Signatures,MediaContentCategory::Image,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Image,Rule7Lengths},
    {MediaFileSystemFamily::Msx,MediaContentRecognitionPriority::Standard,Extensions::Mgs,Rule8Signatures,MediaContentCategory::Audio,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Audio,Rule8Lengths},
    {MediaFileSystemFamily::Msx,MediaContentRecognitionPriority::Standard,Extensions::Bgm,Rule9Signatures,MediaContentCategory::Audio,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Audio,Rule9Lengths},
    {MediaFileSystemFamily::Msx,MediaContentRecognitionPriority::Standard,Extensions::Lzh,Rule10Signatures,MediaContentCategory::Archive,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::None,Rule10Lengths},
    {MediaFileSystemFamily::Msx,MediaContentRecognitionPriority::Standard,Extensions::Pma,Rule11Signatures,MediaContentCategory::Archive,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::None,Rule11Lengths},
    {MediaFileSystemFamily::Msx,MediaContentRecognitionPriority::Standard,Extensions::Com,Rule12Signatures,MediaContentCategory::Executable,MediaTextEncoding::NotApplicable,MediaExecutionKind::NativeExecutable,MediaPreviewKind::Hexadecimal,Rule12Lengths},
    {MediaFileSystemFamily::Msx,MediaContentRecognitionPriority::Standard,Extensions::Rom,Rule13Signatures,MediaContentCategory::Rom,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Hexadecimal,Rule13Lengths}
}};
}
