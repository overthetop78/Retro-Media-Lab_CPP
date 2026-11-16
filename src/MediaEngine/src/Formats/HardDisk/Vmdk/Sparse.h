#pragma once
#include "Container.h"
namespace RetroMediaLab::MediaEngine::Vmdk {
std::shared_ptr<const MediaDocument> readSparse(const RecognitionContext&,bool includeRanges,std::stop_token);
}
