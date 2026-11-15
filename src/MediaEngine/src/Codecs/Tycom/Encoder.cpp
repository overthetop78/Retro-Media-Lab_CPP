#include "Fm.h"
#include "../Crc16.h"
#include "../FmAddressMarks.h"
#include "../../Encoding/Bits.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Tycom {
namespace {
void validate(std::string_view field, std::int32_t value) {
    if (value < 0 || value > Constants::MaximumAddress)
        throw MediaError(DiagnosticCode::TycomAddressRange,
            {std::string(field), std::to_string(value), std::to_string(Constants::MaximumAddress)});
}
}
Flux::Bits Encoder::encodeBits(const Encoding::Request& request, std::stop_token token) const {
    using namespace Constants;
    Flux::Bits bits;
    for (const auto& sector : request.sectors) {
        checkCancellation(token);
        if (sector.data.size() != SectorSize)
            throw MediaError(DiagnosticCode::TycomSectorSize, {std::to_string(sector.data.size())});
        validate(CylinderField, request.cylinder);
        validate(SectorField, sector.number);
        const std::array identity{HeaderMark, std::uint8_t(request.cylinder), std::uint8_t(sector.number)};
        const auto header = Crc16::append(identity, Crc16::Constants::Ccitt,
            Crc16::Constants::AllBitsSet, token);
        Encoding::raw(bits, FmAddressMarks::forMark(HeaderMark));
        Encoding::doubleFm(bits, MediaFileSystems::Bytes(header).subspan(1));
        Encoding::gap(bits, GapBits, true);
        const auto mark = sector.deleted ? FirstDataMark : LastDataMark;
        std::vector<std::uint8_t> body{mark};
        body.insert(body.end(), sector.data.begin(), sector.data.end());
        const auto data = Crc16::append(body, Crc16::Constants::Ccitt,
            Crc16::Constants::AllBitsSet, token);
        Encoding::raw(bits, FmAddressMarks::forMark(mark));
        Encoding::doubleFm(bits, MediaFileSystems::Bytes(data).subspan(1));
        Encoding::gap(bits, GapBits, true);
    }
    return bits;
}
}

