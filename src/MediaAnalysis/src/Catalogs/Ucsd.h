#pragma once
#include "../Constants/Extensions.h"
#include "../Constants/Signatures.h"
#include "../Constants/RecognitionConstants.h"
namespace RetroMediaLab::MediaAnalysis::Catalogs::Ucsd {
inline constexpr std::array<MediaContentSignature, 0> Rule0Signatures{};
inline constexpr std::array<std::size_t, 0> Rule0Lengths{};
inline constexpr std::array<MediaContentSignature, 0> Rule1Signatures{};
inline constexpr std::array<std::size_t, 0> Rule1Lengths{};
inline constexpr std::array<MediaContentSignature, 0> Rule2Signatures{};
inline constexpr std::array<std::size_t, 0> Rule2Lengths{};
inline constexpr std::array<MediaContentSignature, 0> Rule3Signatures{};
inline constexpr std::array<std::size_t, 0> Rule3Lengths{};
inline constexpr std::array<MediaContentRecognitionRule, 4> Rows{{
    {MediaFileSystemFamily::Ucsd,MediaContentRecognitionPriority::Standard,Extensions::Text,Rule0Signatures,MediaContentCategory::Text,MediaTextEncoding::Ascii,MediaExecutionKind::None,MediaPreviewKind::Text,Rule0Lengths},
    {MediaFileSystemFamily::Ucsd,MediaContentRecognitionPriority::Standard,Extensions::Foto,Rule1Signatures,MediaContentCategory::Image,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Image,Rule1Lengths},
    {MediaFileSystemFamily::Ucsd,MediaContentRecognitionPriority::Standard,Extensions::Graf,Rule2Signatures,MediaContentCategory::Image,MediaTextEncoding::NotApplicable,MediaExecutionKind::None,MediaPreviewKind::Image,Rule2Lengths},
    {MediaFileSystemFamily::Ucsd,MediaContentRecognitionPriority::Standard,Extensions::Code,Rule3Signatures,MediaContentCategory::Executable,MediaTextEncoding::NotApplicable,MediaExecutionKind::NativeExecutable,MediaPreviewKind::Hexadecimal,Rule3Lengths}
}};
}
