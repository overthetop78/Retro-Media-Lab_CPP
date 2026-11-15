#include "Mfm.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>

namespace RetroMediaLab::MediaEngine::Aed6200p {
std::array<std::uint8_t, Constants::HeaderIdentityByteCount>
buildHeader(std::int32_t cylinder, std::int32_t sector, std::int32_t size, std::stop_token token) {
    checkCancellation(token);
    return {Constants::HeaderMark, std::uint8_t(cylinder), std::uint8_t(size),
        std::uint8_t(sector), std::uint8_t(std::uint32_t(size) >> Constants::BitsPerByte)};
}
}

