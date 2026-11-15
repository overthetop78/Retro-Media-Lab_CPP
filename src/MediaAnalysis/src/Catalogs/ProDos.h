#pragma once
#include "../Constants/Extensions.h"
#include "../Constants/Signatures.h"
#include "../Constants/RecognitionConstants.h"
namespace RetroMediaLab::MediaAnalysis::Catalogs::ProDos {
inline constexpr std::array<MediaContentSignature, 0> Rule0Signatures{};
inline constexpr std::array<std::size_t, 0> Rule0Lengths{};
inline constexpr std::array<MediaContentSignature, 0> Rule1Signatures{};
inline constexpr std::array<std::size_t, 0> Rule1Lengths{};
inline constexpr std::array<MediaContentSignature, 0> Rule2Signatures{};
inline constexpr std::array<std::size_t, 0> Rule2Lengths{};
inline constexpr std::array<MediaContentSignature, 0> Rule3Signatures{};
inline constexpr std::array<std::size_t, 0> Rule3Lengths{};
inline constexpr std::array<MediaContentRecognitionRule, 4> Rows{{
    {MediaFileSystemFamily::ProDos,MediaContentRecognitionPriority::Standard,Extensions::Txt,Rule0Signatures,MediaContentCategory::Text,MediaTextEncoding::AppleAscii,MediaExecutionKind::None,MediaPreviewKind::Text,Rule0Lengths},
    {MediaFileSystemFamily::ProDos,MediaContentRecognitionPriority::Standard,Extensions::Pic,Rule1Signatures,MediaContentCategory::Image,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Image,Rule1Lengths},
    {MediaFileSystemFamily::ProDos,MediaContentRecognitionPriority::Standard,Extensions::Shk,Rule2Signatures,MediaContentCategory::Archive,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::None,Rule2Lengths},
    {MediaFileSystemFamily::ProDos,MediaContentRecognitionPriority::Standard,Extensions::Sys,Rule3Signatures,MediaContentCategory::System,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Hexadecimal,Rule3Lengths}
}};
}
