#pragma once
#include <cstdint>
#include <string>
class QFile;
namespace RetroMediaLab::MediaEngine {
void preallocateImage(QFile&,std::uint64_t,const std::string&);
}
