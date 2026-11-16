#pragma once
#include "Container.h"
#include "../../../Sources/AtomicImageWriter.h"
namespace RetroMediaLab::MediaEngine::Vmdk {
void writeSparse(const BlockImage&,const std::filesystem::path& fileName,const SeekableImageOutput&,std::stop_token);
}
