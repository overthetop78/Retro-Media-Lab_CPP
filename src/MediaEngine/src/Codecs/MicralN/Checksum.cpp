#include "Fm.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::MicralN {
std::uint8_t checksum(MediaFileSystems::Bytes data,std::stop_token token){checkCancellation(token);auto sum=Constants::InitialChecksum;for(auto byte:data){checkCancellation(token);sum=updateChecksum(sum,byte);}return sum;}
}
