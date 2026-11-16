#pragma once
#include "../../../Images/RecognitionContext.h"
#include "../../../Sources/AtomicImageWriter.h"
namespace RetroMediaLab::MediaEngine::Uef {
std::vector<std::uint8_t> logicalBytes(const RecognitionContext&,bool& gzip,bool headerOnly,std::stop_token);
void writeGzip(const std::function<void(const ImageOutput&)>& producer,const ImageOutput&,std::stop_token);
}
