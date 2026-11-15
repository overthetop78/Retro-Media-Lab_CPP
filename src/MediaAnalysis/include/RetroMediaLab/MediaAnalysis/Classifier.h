#pragma once
#include "Export.h"
#include "Models.h"
#include "Localization.h"
#include <map>
namespace RetroMediaLab::MediaAnalysis {
struct ClassificationInput {
    std::string_view extension;
    MediaEntryKind kind = MediaEntryKind::File;
    std::optional<std::string_view> nativeTypeId;
    std::string_view comment;
    std::optional<bool> dataValid;
    Content content;
    std::map<std::string, std::string> metadata;
    MediaFileSystemFamily family = MediaFileSystemFamily::Unknown;
};
RETROMEDIALAB_MEDIAANALYSIS_API MediaContentTypeDefinition classify(const ClassificationInput&);
}
