#include "Policy.h"
#include "Constants.h"
#include "../../Encoding/Timings.h"
#include "../../Models/Sectors/SizeCodes.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>

namespace RetroMediaLab::MediaEngine::Reconstruction::Sector {
std::shared_ptr<const Block> availableBlock(const Image& image, std::uint64_t index, std::stop_token token) {
    // AvailableBlocks in C# exposes recorded data without GetBlock's nominal-size check.
    auto block = image.block(index, token);
    if (!block) throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::SectorMissingBlock, {std::to_string(index)});
    return block;
}
std::int32_t checkedNumber(std::int64_t value) {
    if (value < Constants::MinimumAddress || value > Constants::MaximumAddress)
        throw MediaError(DiagnosticCode::PolicyAddressRange, {std::to_string(value)});
    return std::int32_t(value);
}
std::vector<Encoding::Sector> Policy::createTrackSectors(const Image&, std::span<const Item> items, std::stop_token token) const {
    std::vector<Encoding::Sector> result;
    result.reserve(items.size());
    for (const auto& item : items) {
        checkCancellation(token);
        if (!item.block) throw MediaError(DiagnosticCode::PolicyNullBlock);
        const auto& block = *item.block;
        const auto number = block.recordedAddress ? block.recordedAddress->number : item.address.number;
        if (number < Constants::MinimumAddress || number > Constants::MaximumAddress)
            throw MediaError(DiagnosticCode::PolicyAddressRange, {std::to_string(number)});
        Encoding::Sector sector;
        sector.number = std::int32_t(number);
        sector.data = block.data;
        sector.deleted = block.deletedDataMark.value_or(false);
        sector.sizeCode = trySectorSizeCode(std::int64_t(block.data.size()));
        if (block.tag && !block.tag->empty()) {
            sector.attributes.emplace();
            for (std::size_t index = 0; index < block.tag->size(); ++index)
                sector.attributes->emplace(std::string(Constants::TagPrefix) + std::to_string(index), (*block.tag)[index]);
        }
        if (block.recordedAddress) {
            for (auto value : {block.recordedAddress->cylinder, block.recordedAddress->head})
                if (value < Constants::MinimumAddress || value > Constants::MaximumAddress)
                    throw MediaError(DiagnosticCode::PolicyAddressRange, {std::to_string(value)});
            sector.recordedCylinder = std::int32_t(block.recordedAddress->cylinder);
            sector.recordedHead = std::int32_t(block.recordedAddress->head);
        }
        sector.recordedSizeCode = block.recordedSizeCode;
        sector.headerIntegrityValid = block.headerIntegrityValid;
        sector.dataIntegrityValid = block.dataIntegrityValid.has_value() ? block.dataIntegrityValid : block.integrityValid;
        sector.recordedErrorCheckCode = block.recordedErrorCheckCode;
        result.push_back(std::move(sector));
    }
    return result;
}
std::uint32_t Policy::bitCellTicks(const Image& image, std::int32_t) const { return Encoding::bitCellTicks(image.geometry().formatId); }
std::uint32_t Policy::indexTimeTicks(const Image& image, std::int32_t) const { return Encoding::indexTimeTicks(image.geometry().formatId); }
}

