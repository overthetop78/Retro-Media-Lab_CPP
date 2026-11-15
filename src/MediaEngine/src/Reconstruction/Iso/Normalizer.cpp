#include "Builder.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::IsoReconstruction {
std::vector<std::uint8_t> padTo(std::span<const std::uint8_t> data,std::int32_t size,std::stop_token token){
    checkCancellation(token);if(size<0||data.size()>std::uint64_t(size))return {};
    std::vector<std::uint8_t> result(std::size_t(size),0);std::copy(data.begin(),data.end(),result.begin());checkCancellation(token);return result;
}
}
