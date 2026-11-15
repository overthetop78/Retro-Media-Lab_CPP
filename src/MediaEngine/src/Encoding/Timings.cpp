#include "Timings.h"
#include "TimingConstants.h"
#include "Constants.h"
#include "../Formats/FormatIds.h"
#include "../Primitives/Identifiers.h"
#include <array>

namespace RetroMediaLab::MediaEngine::Encoding {
std::uint32_t bitCellTicks(std::string_view id) {
    using namespace TimingConstants;
    using Primitives::asciiIdentifierEqualsIgnoreCase;
    using Primitives::asciiIdentifierStartsWithIgnoreCase;
    if (asciiIdentifierEqualsIgnoreCase(id, Formats::AmigaDosHighDensity) || asciiIdentifierEqualsIgnoreCase(id, Formats::Ibm2880))
        return ExtraDensityMfmCell;
    if (asciiIdentifierStartsWithIgnoreCase(id, Formats::AmigaPrefix)) return HighDensityMfmCell;
    if (asciiIdentifierEqualsIgnoreCase(id, Formats::Atari90) || asciiIdentifierEqualsIgnoreCase(id, Formats::AtariXfd90)
        || asciiIdentifierStartsWithIgnoreCase(id, Formats::AcornDfsPrefix)) return SingleDensityFmCell;
    if (asciiIdentifierEqualsIgnoreCase(id, Formats::AcornAdfs1600) || asciiIdentifierEqualsIgnoreCase(id, Formats::AtariSt1440))
        return HighDensityMfmCell;
    constexpr std::array highDensity{Formats::Ibm1200, Formats::Ibm1440, Formats::Ibm1680, Formats::IbmDmf};
    for (auto format : highDensity) if (asciiIdentifierEqualsIgnoreCase(id, format)) return HighDensityMfmCell;
    if (asciiIdentifierEqualsIgnoreCase(id, Formats::AcornDosPlus800)) return DoubleDensityMfmCell;
    constexpr std::array doubleDensity{Formats::AtariPrefix, Formats::AtariStPrefix, Formats::IbmPrefix,
        Formats::MsxPrefix, Formats::AcornAdfsPrefix, Formats::AmstradPrefix, Formats::EpsonQx10Prefix, Formats::UcsdPrefix};
    for (auto prefix : doubleDensity) if (asciiIdentifierStartsWithIgnoreCase(id, prefix)) return DoubleDensityMfmCell;
    return Constants::DefaultCellTicks;
}
std::uint32_t indexTimeTicks(std::string_view id) {
    return Primitives::asciiIdentifierEqualsIgnoreCase(id, Formats::Ibm1200)
        ? TimingConstants::Rpm360Index : TimingConstants::Rpm300Index;
}
}
