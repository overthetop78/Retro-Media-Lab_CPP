#include "Mfm.h"
#include "../Crc16.h"
#include "../../Encoding/Bits.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>

namespace RetroMediaLab::MediaEngine::Aed6200p {
namespace {
void validate(std::string_view field, std::int64_t value, std::int32_t maximum) {
    if (value < 0 || value > maximum)
        throw MediaError(DiagnosticCode::AedAddressRange,
            {std::string(field), std::to_string(value), std::to_string(maximum)});
}
}

Flux::Bits Encoder::encodeBits(const Encoding::Request& request, std::stop_token token) const {
    using namespace Constants;
    Flux::Bits bits;
    for (const auto& sector : request.sectors) {
        checkCancellation(token);
        validate(CylinderField, request.cylinder, MaximumAddress);
        validate(SectorField, sector.number, MaximumAddress);
        validate(SizeField, sector.data.size(), MaximumSize);

        const auto identity = buildHeader(request.cylinder, sector.number,
            std::int32_t(sector.data.size()), token);
        const auto header = Crc16::append(identity, Crc16::Constants::Ccitt,
            Crc16::Constants::AllBitsSet, token);
        Encoding::raw(bits, HeaderPattern);
        Encoding::mfm(bits, MediaFileSystems::Bytes(header).subspan(DataMarkByteCount));
        Encoding::gap(bits, FirstGapBitCount);

        const auto& mark = DataMarks[sector.deleted ? DeletedDataMarkIndex : NormalDataMarkIndex];
        std::vector<std::uint8_t> body{mark.mark};
        body.insert(body.end(), sector.data.begin(), sector.data.end());
        const auto data = Crc16::append(body, Crc16::Constants::Ccitt,
            Crc16::Constants::AllBitsSet, token);
        Encoding::raw(bits, mark.pattern);
        Encoding::mfm(bits, MediaFileSystems::Bytes(data).subspan(DataMarkByteCount));
        Encoding::gap(bits, SecondGapBitCount);
    }
    return bits;
}
}

