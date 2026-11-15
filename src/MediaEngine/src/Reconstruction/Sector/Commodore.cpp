#include "Policies.h"
#include "Constants.h"
#include "../../Formats/FormatIds.h"
#include "../../Formats/Commodore900/Constants.h"
#include "../../Primitives/Identifiers.h"
#include "../../Codecs/Commodore/Commodore1541Constants.h"
#include "../../Codecs/Commodore/Commodore900Constants.h"
#include "../../Codecs/Iso/MfmConstants.h"
#include "../../Encoding/TimingConstants.h"
#include <RetroMediaLab/MediaFileSystems/CommodoreGeometryConstants.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <map>

namespace RetroMediaLab::MediaEngine::Reconstruction::Sector {
namespace Cbm = MediaFileSystems::Commodore::Constants;
using Primitives::asciiIdentifierEqualsIgnoreCase;
using Primitives::asciiIdentifierStartsWithIgnoreCase;
bool CommodorePolicy::canHandle(const Image& image) const {
    return asciiIdentifierStartsWithIgnoreCase(image.geometry().formatId, Formats::CommodorePrefix)
        || asciiIdentifierStartsWithIgnoreCase(image.geometry().formatId, Formats::Commodore900Prefix);
}
std::string_view CommodorePolicy::encoderId(const Image& image) const {
    const auto& id = image.geometry().formatId;
    if (asciiIdentifierStartsWithIgnoreCase(id, Cbm::Format1581)) return IsoMfm::Constants::Id;
    if (asciiIdentifierStartsWithIgnoreCase(id, Formats::Commodore900Prefix)) return Commodore900Gcr::Constants::Id;
    return Commodore1541Gcr::Constants::Id;
}
std::vector<Encoding::Sector> CommodorePolicy::createTrackSectors(const Image& image, std::span<const Item> items, std::stop_token token) const {
    if (!asciiIdentifierStartsWithIgnoreCase(image.geometry().formatId, Cbm::Format1581))
        return Policy::createTrackSectors(image, items, token);
    std::map<std::uint64_t,std::vector<std::shared_ptr<const Block>>> groups;
    for (const auto& item : items) {
        checkCancellation(token);
        if (!item.block) throw MediaError(DiagnosticCode::PolicyNullBlock);
        groups[item.block->logicalBlock / Cbm::BlocksPerPhysicalSector1581].push_back(item.block);
    }
    std::vector<Encoding::Sector> result;
    for (auto& [group, blocks] : groups) {
        checkCancellation(token);
        std::stable_sort(blocks.begin(), blocks.end(), [](const auto& a, const auto& b) { return a->logicalBlock < b->logicalBlock; });
        Encoding::Sector sector;
        sector.number = std::int32_t(group % Cbm::PhysicalSectors1581) + Cbm::FirstPhysicalSector1581;
        sector.sizeCode = Cbm::PhysicalSizeCode1581;
        for (const auto& block : blocks) {
            const auto take = std::min(block->data.size(), std::size_t(Cbm::PhysicalSectorBytes1581) - sector.data.size());
            sector.data.insert(sector.data.end(), block->data.begin(), block->data.begin() + take);
            if (sector.data.size() == Cbm::PhysicalSectorBytes1581) break;
        }
        if (sector.data.size() == Cbm::PhysicalSectorBytes1581) result.push_back(std::move(sector));
    }
    return result;
}
Address CommodorePolicy::visualAddress(const Image& image, Address address) const {
    if (!asciiIdentifierStartsWithIgnoreCase(image.geometry().formatId, Cbm::Format1581)) return address;
    const auto logical = std::int64_t(checkedNumber(address.cylinder)) * image.geometry().sectorsPerTrack + checkedNumber(address.number);
    const auto physical = logical / Cbm::BlocksPerPhysicalSector1581;
    constexpr auto perCylinder = Cbm::PhysicalHeads1581 * Cbm::PhysicalSectors1581;
    return {physical / perCylinder, physical % perCylinder / Cbm::PhysicalSectors1581,
        physical % Cbm::PhysicalSectors1581 + Cbm::FirstPhysicalSector1581};
}
Encoding::Attributes CommodorePolicy::trackAttributes(const Image& image, std::int32_t) const {
    const auto& g = image.geometry();
    if (asciiIdentifierEqualsIgnoreCase(g.formatId, Cbm::Format1541) || asciiIdentifierEqualsIgnoreCase(g.formatId, Cbm::Format1571))
        return std::map<std::string,std::int32_t>{{std::string(Constants::TracksPerSideAttribute), checkedNumber(g.cylinders)}};
    return {};
}
std::uint32_t CommodorePolicy::bitCellTicks(const Image& image, std::int32_t cylinder) const {
    const auto& id = image.geometry().formatId;
    if (asciiIdentifierStartsWithIgnoreCase(id, Formats::Commodore900Prefix)) {
        for (std::size_t zone = 0; zone + 1 < Commodore900::Constants::ZoneEnds.size(); ++zone)
            if (cylinder < std::int64_t(Commodore900::Constants::ZoneEnds[zone])) return Constants::Commodore900CellTicks[zone];
        return Constants::Commodore900CellTicks.back();
    }
    if (asciiIdentifierEqualsIgnoreCase(id, Cbm::Format1541) || asciiIdentifierEqualsIgnoreCase(id, Cbm::Format1571)) {
        const auto track = std::int64_t(cylinder) + Cbm::FirstTrack;
        for (std::size_t zone = 0; zone + 1 < Cbm::ZoneEnds.size(); ++zone)
            if (track <= Cbm::ZoneEnds[zone]) return Constants::Commodore1541CellTicks[zone];
        return Constants::Commodore1541CellTicks.back();
    }
    if (asciiIdentifierEqualsIgnoreCase(id, Cbm::Format1581)) return Encoding::TimingConstants::DoubleDensityMfmCell;
    return Policy::bitCellTicks(image, cylinder);
}
}

