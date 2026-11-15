#pragma once
#include "../Constants/Extensions.h"
#include "../Constants/Signatures.h"
#include "../Constants/RecognitionConstants.h"
namespace RetroMediaLab::MediaAnalysis::Catalogs::AtariCommon {
inline constexpr std::array<MediaContentSignature, 0> Rule0Signatures{};
inline constexpr std::array<std::size_t, 0> Rule0Lengths{};
inline constexpr std::array<MediaContentRecognitionRule, 1> Rows{{
    {MediaFileSystemFamily::Atari,MediaContentRecognitionPriority::Standard,Extensions::Bas,Rule0Signatures,MediaContentCategory::BasicProgram,MediaTextEncoding::Unknown,MediaExecutionKind::None,MediaPreviewKind::BasicListing,Rule0Lengths}
}};
}
