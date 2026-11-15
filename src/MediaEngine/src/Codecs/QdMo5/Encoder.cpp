#include "Mfm.h"
#include "../../Encoding/Bits.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::QdMo5 {
namespace {
void validate(std::string_view field, std::int32_t value, std::int32_t maximum) {
    if (value < 0 || value > maximum)
        throw MediaError(DiagnosticCode::QdAddressRange,
            {std::string(field), std::to_string(value), std::to_string(maximum)});
}
}
Flux::Bits Encoder::encodeBits(const Encoding::Request& request, std::stop_token token) const {
    using namespace Constants;
    Flux::Bits bits;
    for (const auto& sector : request.sectors) {
        checkCancellation(token);
        if (sector.data.size() != SectorSize)
            throw MediaError(DiagnosticCode::QdSectorSize, {std::to_string(sector.data.size())});
        validate(SectorField, sector.number, MaximumSector);
        const auto prefix = attribute(sector.attributes, std::string(PrefixAttribute), DefaultPrefix);
        validate(PrefixAttribute, prefix, MaximumPrefix);
        std::vector<std::uint8_t> header{std::uint8_t(sector.number >> BitsPerByte), std::uint8_t(sector.number)};
        header.resize(HeaderTailBytes, PaddingByte);
        Encoding::raw(bits, HeaderMark);
        Encoding::mfm(bits, header);
        Encoding::gap(bits, HeaderGapBits);
        std::vector<std::uint8_t> body{std::uint8_t(prefix)};
        body.insert(body.end(), sector.data.begin(), sector.data.end());
        body.push_back(checksum(std::uint8_t(prefix), sector.data, token));
        Encoding::raw(bits, Preamble);
        Encoding::mfm(bits, body);
        Encoding::gap(bits, DataGapBits);
    }
    return bits;
}
}

