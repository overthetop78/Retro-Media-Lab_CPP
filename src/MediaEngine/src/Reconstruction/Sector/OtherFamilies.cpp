#include "Policies.h"
#include "Constants.h"
#include "../../Formats/FormatIds.h"
#include "../../Encoding/RareProfiles.h"
#include "../../Models/Sectors/SizeCodes.h"
#include "../../Codecs/DecRx02/Constants.h"
#include "../../Codecs/Iso/FmConstants.h"
#include "../../Codecs/Iso/MfmConstants.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>

namespace RetroMediaLab::MediaEngine::Reconstruction::Sector {
using Primitives::asciiIdentifierEqualsIgnoreCase;
using Primitives::asciiIdentifierStartsWithIgnoreCase;
namespace {
Encoding::RareProfile requireRare(const Image& image) {
    if (auto profile = Encoding::rareProfile(image.geometry().formatId)) return *profile;
    throw MediaError(DiagnosticCode::PolicyRareUnknown, {image.geometry().formatId});
}
std::shared_ptr<const Block> firstBlock(const Image& image) {
    std::shared_ptr<const Block> result;
    image.visitAvailableBlocks([&](auto index) { result = availableBlock(image,index); return false; });
    if (!result) throw MediaError(DiagnosticCode::PolicyJfdDensity);
    return result;
}
}
bool Rx02Policy::canHandle(const Image& image) const { return asciiIdentifierEqualsIgnoreCase(image.geometry().formatId, Formats::DecRx02); }
std::string_view Rx02Policy::encoderId(const Image&) const { return DecRx02::Constants::Id; }
std::vector<Encoding::Sector> Rx02Policy::createTrackSectors(const Image&, std::span<const Item> items, std::stop_token token) const {
    std::vector<Encoding::Sector> result;
    for (const auto& item : items) {
        checkCancellation(token);
        if (!item.block) throw MediaError(DiagnosticCode::PolicyNullBlock);
        if (item.block->data.size() < Constants::Rx02BlockBytes) continue;
        const auto first = (std::int64_t(checkedNumber(item.address.number)) - Constants::FirstSector) * Constants::Rx02SectorsPerBlock + Constants::FirstSector;
        for (std::int32_t part = 0; part < Constants::Rx02SectorsPerBlock; ++part) {
            Encoding::Sector sector;
            sector.number = checkedNumber(first + part);
            sector.sizeCode = sectorSizeCode(Constants::Rx02SectorBytes);
            const auto start = item.block->data.begin() + part * Constants::Rx02SectorBytes;
            sector.data.assign(start, start + Constants::Rx02SectorBytes);
            result.push_back(std::move(sector));
        }
    }
    return result;
}
bool AtariPolicy::canHandle(const Image& image) const {
    return asciiIdentifierStartsWithIgnoreCase(image.geometry().formatId, Formats::AtariPrefix)
        || asciiIdentifierStartsWithIgnoreCase(image.geometry().formatId, Formats::AtariStPrefix);
}
std::string_view AtariPolicy::encoderId(const Image& image) const {
    const auto& id = image.geometry().formatId;
    return asciiIdentifierEqualsIgnoreCase(id, Formats::Atari90) || asciiIdentifierEqualsIgnoreCase(id, Formats::AtariXfd90)
        || asciiIdentifierEqualsIgnoreCase(id, Formats::AtariAtx) ? IsoFm::Constants::Id : IsoMfm::Constants::Id;
}
Address AtariPolicy::visualAddress(const Image& image, Address address) const {
    const auto& g = image.geometry();
    if (!asciiIdentifierStartsWithIgnoreCase(g.formatId, Formats::AtariPrefix) || g.sectorsPerTrack != Constants::AtariLinearSectors
        || g.cylinders <= Constants::EightyCylinders) return address;
    const auto sectors = asciiIdentifierEqualsIgnoreCase(g.formatId, Formats::Atari130)
        || asciiIdentifierEqualsIgnoreCase(g.formatId, Formats::AtariXfd130) ? Constants::AtariEnhancedSectors : Constants::AtariStandardSectors;
    return {address.cylinder / sectors, 0, address.cylinder % sectors + Constants::FirstSector};
}
bool RarePolicy::canHandle(const Image& image) const { return Encoding::rareProfile(image.geometry().formatId).has_value(); }
std::string_view RarePolicy::encoderId(const Image& image) const { return requireRare(image).encoderId; }
std::uint32_t RarePolicy::bitCellTicks(const Image& image, std::int32_t) const { return requireRare(image).cellTicks; }
std::uint32_t RarePolicy::indexTimeTicks(const Image& image, std::int32_t) const { return requireRare(image).indexTicks; }
bool JfdPolicy::canHandle(const Image& image) const {
    if (image.geometry().formatId != Formats::Jfd) return false;
    bool any = false, compatible = true;
    std::optional<std::uint8_t> density;
    image.visitAvailableBlocks([&](auto index) {
        const auto block = availableBlock(image,index);
        if (any && density != block->formatCode) compatible = false;
        if (!any) density = block->formatCode;
        any = true;
        if (!block->tag || block->tag->size() != Constants::JfdTagBytes
            || (*block->tag)[Constants::JfdTagDensityIndex] != Constants::JfdOrdinaryTrack) compatible = false;
        return compatible;
    });
    return any && compatible;
}
std::string_view JfdPolicy::encoderId(const Image& image) const {
    return firstBlock(image)->formatCode == Constants::JfdFmDensity ? IsoFm::Constants::Id : IsoMfm::Constants::Id;
}
std::uint32_t JfdPolicy::bitCellTicks(const Image& image, std::int32_t) const {
    const auto density = firstBlock(image)->formatCode;
    if (!density || !*density) throw MediaError(DiagnosticCode::PolicyJfdDensity);
    return Constants::JfdDensityDivisor / *density;
}
}

