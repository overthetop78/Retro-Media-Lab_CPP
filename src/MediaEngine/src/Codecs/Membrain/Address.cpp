#include "Mfm.h"

namespace RetroMediaLab::MediaEngine::Membrain {
std::array<std::uint8_t, Constants::AddressByteCount> pack(std::int32_t cylinder, std::int32_t head, std::int32_t sector) noexcept {
    using namespace Constants;
    return {std::uint8_t(std::uint32_t(cylinder) >> CylinderLowBits),
        std::uint8_t(((std::uint32_t(cylinder) & CylinderLowValueMask) << CylinderLowShift)
            | (std::uint32_t(head) << HeadShift) | (std::uint32_t(sector) & SectorMask))};
}
Address unpack(std::uint8_t cylinderHigh, std::uint8_t packed) noexcept {
    using namespace Constants;
    return {std::uint8_t(((cylinderHigh & CylinderHighMask) << CylinderLowBits)
        | ((packed & CylinderLowMask) >> CylinderLowShift)),
        std::uint8_t((packed >> HeadShift) & HeadMask), std::uint8_t(packed & SectorMask)};
}
}

