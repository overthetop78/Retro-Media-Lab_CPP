#pragma once
#include "MediaKind.h"
#include "MessageInfo.h"
#include <functional>
#include <optional>
namespace RetroMediaLab::MediaEngine {
enum class MediaExplorationStage {ReadingMedia,ReadingFileSystem,RecognizingFiles};
struct MediaExplorationProgress {MediaExplorationStage stage;std::string detail;double value;std::optional<MediaKind> mediaKind;std::optional<MessageInfo> message;};
using MediaExplorationProgressCallback=std::function<void(const MediaExplorationProgress&)>;
}
