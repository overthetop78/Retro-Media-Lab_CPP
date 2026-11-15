#include "Policies.h"
#include "../../Formats/Apple/Geometry.h"
#include "Constants.h"
#include "../../Formats/FormatIds.h"
#include "../../Primitives/Identifiers.h"
#include "../../Codecs/AppleII/Constants.h"
#include "../../Codecs/AppleRwts18/Constants.h"
#include "../../Codecs/AppleIwm/Constants.h"
#include "../../Codecs/Iso/MfmConstants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <array>

namespace RetroMediaLab::MediaEngine::Reconstruction::Sector {
using Primitives::asciiIdentifierEqualsIgnoreCase;
using Primitives::asciiIdentifierStartsWithIgnoreCase;
bool ApplePolicy::canHandle(const Image& image) const {
    constexpr std::array prefixes{Formats::AppleIIPrefix, Formats::AppleIIIPrefix, Formats::AppleMacPrefix, Formats::AppleLisaPrefix, Formats::MacPrefix};
    for (auto prefix : prefixes) if (asciiIdentifierStartsWithIgnoreCase(image.geometry().formatId, prefix)) return true;
    return false;
}
std::string_view ApplePolicy::encoderId(const Image& image) const {
    const auto& g = image.geometry();
    if (asciiIdentifierEqualsIgnoreCase(g.formatId, Formats::AppleIIRwts18)) return AppleRwts18::Constants::Id;
    if ((asciiIdentifierEqualsIgnoreCase(g.formatId, Formats::AppleIIProDos) || asciiIdentifierEqualsIgnoreCase(g.formatId, Formats::AppleIIProDos800))
        && g.blockSize == Constants::ProDosBlockBytes && g.cylinders >= Constants::EightyCylinders) return AppleIwm::Constants::MacId;
    if (asciiIdentifierStartsWithIgnoreCase(g.formatId, Formats::AppleIIPrefix) || asciiIdentifierStartsWithIgnoreCase(g.formatId, Formats::AppleIIIPrefix)) return AppleII::Constants::Id;
    if (asciiIdentifierStartsWithIgnoreCase(g.formatId, Formats::AppleLisaPrefix) && g.cylinders == Constants::LisaCylinders && g.heads == Constants::DoubleHeads) return AppleIwm::Constants::LisaId;
    if (asciiIdentifierEqualsIgnoreCase(g.formatId, Formats::Mac1440)) return IsoMfm::Constants::Id;
    return AppleIwm::Constants::MacId;
}
std::vector<Encoding::Sector> ApplePolicy::createTrackSectors(const Image& image, std::span<const Item> items, std::stop_token token) const {
    const auto& g = image.geometry();
    if (!(asciiIdentifierEqualsIgnoreCase(g.formatId, Formats::AppleIIProDos) || asciiIdentifierEqualsIgnoreCase(g.formatId, Formats::AppleIIISos))
        || g.cylinders >= Constants::EightyCylinders) return Policy::createTrackSectors(image, items, token);
    std::vector<Encoding::Sector> result;
    for (const auto& item : items) {
        checkCancellation(token);
        if (!item.block) throw MediaError(DiagnosticCode::PolicyNullBlock);
        const auto& block = *item.block;
        if (block.data.size() < Constants::ProDosBlockBytes) continue;
        const auto base = std::int64_t(checkedNumber(block.address.number)) * Constants::SectorsPerProDosBlock;
        for (std::int32_t part = 0; part < Constants::SectorsPerProDosBlock; ++part) {
            Encoding::Sector sector;
            sector.number = checkedNumber(base + part);
            const auto start = block.data.begin() + part * Constants::AppleSectorBytes;
            sector.data.assign(start, start + Constants::AppleSectorBytes);
            result.push_back(std::move(sector));
        }
    }
    return result;
}
Address ApplePolicy::visualAddress(const Image& image, Address address) const {
    const auto& g = image.geometry();
    if (asciiIdentifierStartsWithIgnoreCase(g.formatId, Formats::AppleLisaPrefix) && g.heads == Constants::SingleHead && g.cylinders > Constants::LisaLinearThreshold)
        return {address.cylinder / Constants::DoubleHeads, address.cylinder % Constants::DoubleHeads, address.number};
    return address;
}
Encoding::Attributes ApplePolicy::trackAttributes(const Image& image, std::int32_t count) const {
    const auto& g = image.geometry();
    if (asciiIdentifierStartsWithIgnoreCase(g.formatId, Formats::AppleIIPrefix))
        return std::map<std::string,std::int32_t>{{std::string(Constants::SectorCountAttribute), count},
            {std::string(Constants::FormatAttribute), g.cylinders >= Constants::EightyCylinders ? Constants::AppleII80Format : Constants::AppleIIFormat}};
    if (asciiIdentifierStartsWithIgnoreCase(g.formatId, Formats::AppleMacPrefix) || asciiIdentifierStartsWithIgnoreCase(g.formatId, Formats::MacPrefix))
        return std::map<std::string,std::int32_t>{{std::string(Constants::FormatAttribute), g.heads == Constants::SingleHead ? Constants::MacSingleFormat : Constants::MacDoubleFormat}};
    if (asciiIdentifierStartsWithIgnoreCase(g.formatId, Formats::AppleLisaPrefix))
        return std::map<std::string,std::int32_t>{{std::string(Constants::FormatAttribute), Constants::LisaFormat}};
    return {};
}
std::uint32_t ApplePolicy::bitCellTicks(const Image& image, std::int32_t cylinder) const {
    const auto id = encoderId(image);
    if (id == AppleII::Constants::Id || id == AppleRwts18::Constants::Id) return Constants::AppleIICellTicks;
    if (id == AppleIwm::Constants::MacId || id == AppleIwm::Constants::LisaId) return Constants::IwmCellTicks;
    return Policy::bitCellTicks(image, cylinder);
}
std::uint32_t ApplePolicy::indexTimeTicks(const Image& image, std::int32_t cylinder) const {
    const auto id = encoderId(image);
    if (id == AppleIwm::Constants::LisaId) {
        if (cylinder < 0 || cylinder >= Constants::LisaCylinders)
            throw MediaError(DiagnosticCode::PolicyLisaCylinder, {std::to_string(cylinder), std::to_string(Constants::LisaCylinders - 1)});
        return std::uint32_t(Apple::Geometry::LisaFileWare::sectors(cylinder)) * Constants::LisaTicksPerSector;
    }
    if (id == AppleIwm::Constants::MacId) {
        for (std::size_t zone = 0; zone < Apple::Geometry::MacintoshGcr::ZoneEnds.size() - 1; ++zone)
            if (cylinder < Apple::Geometry::MacintoshGcr::ZoneEnds[zone]) return Constants::MacIndexTicks[zone];
        return Constants::MacIndexTicks.back();
    }
    return Policy::indexTimeTicks(image, cylinder);
}
}

