#include "Blocks.h"
#include "../../Encoding/Bits.h"
#include "../../Primitives/Bits.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>

namespace RetroMediaLab::MediaEngine::Arburg {
Flux::Bits Encoder::encodeBits(const Encoding::Request& request, std::stop_token token) const {
    Flux::Bits bits;
    for (const auto& sector : request.sectors) {
        checkCancellation(token);
        const auto system = attribute(sector.attributes, std::string(Constants::SystemAttribute), 0) != 0;
        auto block = buildBlock(sector.data, system, token);
        if (system) {
            Encoding::raw(bits, Constants::SystemMark);
            const auto body = encodeSystem(block, token);
            bits.insert(bits.end(), body.begin(), body.end());
        } else {
            Encoding::raw(bits, Constants::DataMark);
            for (auto& value : block) value = Primitives::reverseBits(value);
            Encoding::doubleFm(bits, block);
        }
        Encoding::gap(bits, Constants::GapBits, true);
    }
    return bits;
}
}

