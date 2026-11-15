#pragma once
#include "Export.h"
#include "Models.h"
namespace RetroMediaLab::MediaAnalysis {
RETROMEDIALAB_MEDIAANALYSIS_API std::string normalizeExtension(std::string_view);
RETROMEDIALAB_MEDIAANALYSIS_API bool hasContentConditions(const MediaContentRecognitionRule&);
RETROMEDIALAB_MEDIAANALYSIS_API bool matches(const MediaContentRecognitionRule&, Content);
RETROMEDIALAB_MEDIAANALYSIS_API std::span<const MediaContentRecognitionRule> recognitionRules();
RETROMEDIALAB_MEDIAANALYSIS_API const MediaContentRecognitionRule* findRule(MediaFileSystemFamily, std::string_view, Content = std::nullopt, MediaContentRecognitionPriority = MediaContentRecognitionPriority::Standard);
}
