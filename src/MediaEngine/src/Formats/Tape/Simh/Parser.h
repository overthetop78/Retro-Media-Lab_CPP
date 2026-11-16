#pragma once
#include "Reader.h"
namespace RetroMediaLab::MediaEngine::SimhTape {
struct ParseResult {std::vector<SequentialSegment> segments;std::vector<MediaFileSystems::MessageRecipe> messages;};
ParseResult parse(const RecognitionContext&,bool createSegments,std::stop_token);
}
