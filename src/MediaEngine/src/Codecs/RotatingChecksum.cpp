#include "RotatingChecksum.h"
#include "RotatingChecksumConstants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::RotatingChecksum {
std::uint8_t compute(MediaFileSystems::Bytes values,std::stop_token token){
 checkCancellation(token);std::uint8_t checksum=0;
 for(auto value:values){checkCancellation(token);checksum^=value;checksum=std::uint8_t((checksum>>Constants::RightRotation)|(checksum<<Constants::LeftRotation));}
 return checksum;
}
}
