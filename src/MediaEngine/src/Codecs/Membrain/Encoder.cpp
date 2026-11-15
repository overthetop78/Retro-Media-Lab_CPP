#include "Mfm.h"
#include "../Crc16.h"
#include "../../Encoding/Bits.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>

namespace RetroMediaLab::MediaEngine::Membrain {
namespace {
void validate(std::string_view field, std::int32_t value, std::int32_t maximum) {
    if (value < 0 || value > maximum)
        throw MediaError(DiagnosticCode::MembrainAddressRange,
            {std::string(field), std::to_string(value), std::to_string(maximum)});
}
}
Flux::Bits Encoder::encodeBits(const Encoding::Request& request, std::stop_token token) const {
    using namespace Constants;
    validate(Encoding::Constants::CylinderField, request.cylinder, MaximumCylinder);
    validate(Encoding::Constants::HeadField, request.head, MaximumHead);
    Flux::Bits bits;
    for (const auto& sector : request.sectors) {
        checkCancellation(token);
        if (sector.data.size() != SectorSize)
            throw MediaError(DiagnosticCode::MembrainSectorSize, {std::to_string(sector.data.size())});
        validate(Encoding::Constants::NumberField, sector.number, MaximumSector);
        const auto address = pack(request.cylinder, request.head, sector.number);
        const std::array header{SyncByte, HeaderMark, address.front(), address.back()};
        const auto protectedHeader = Crc16::append(header, Crc16::Constants::Ibm,
            Crc16::Constants::Zero, token);
        Encoding::raw(bits, HeaderPattern);
        Encoding::mfm(bits, MediaFileSystems::Bytes(protectedHeader).subspan(DataPrefixBytes));
        Encoding::gap(bits, HeaderGapBits);

        std::vector<std::uint8_t> body{SyncByte, FirstDataMark};
        body.insert(body.end(), sector.data.begin(), sector.data.end());
        const auto protectedData = Crc16::append(body, Crc16::Constants::Ibm,
            Crc16::Constants::Zero, token);
        Encoding::raw(bits, DataPattern);
        Encoding::mfm(bits, MediaFileSystems::Bytes(protectedData).subspan(DataPrefixBytes));
        Encoding::gap(bits, DataGapBits);
    }
    return bits;
}
}

