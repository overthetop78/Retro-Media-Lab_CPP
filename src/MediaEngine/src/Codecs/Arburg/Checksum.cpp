#include "Blocks.h"
#include <algorithm>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>

namespace RetroMediaLab::MediaEngine::Arburg {
std::uint16_t checksum(MediaFileSystems::Bytes bytes, std::stop_token token) {
    checkCancellation(token);
    std::uint16_t result = 0;
    for (auto value : bytes) {
        checkCancellation(token);
        result = std::uint16_t(result + value);
    }
    return result;
}

bool checksumValid(MediaFileSystems::Bytes block, std::int64_t usefulSize, std::stop_token token) {
    checkCancellation(token);
    if (usefulSize < 0)
        throw MediaError(DiagnosticCode::ArburgUsefulSize, {std::to_string(usefulSize)});
    if (std::uint64_t(usefulSize) > block.size()
        || block.size() - std::size_t(usefulSize) < Constants::ChecksumBytes) return false;
    const auto value = checksum(block.first(std::size_t(usefulSize)), token);
    return block[std::size_t(usefulSize)] == std::uint8_t(value)
        && block[std::size_t(usefulSize) + 1] == std::uint8_t(value >> Constants::BitsPerByte);
}

std::vector<std::uint8_t> createBlock(MediaFileSystems::Bytes data, std::int64_t totalSize,
    std::stop_token token) {
    checkCancellation(token);
    if (totalSize < Constants::ChecksumBytes || totalSize > Constants::MaximumBytes
        || data.size() > std::uint64_t(totalSize - Constants::ChecksumBytes))
        throw MediaError(DiagnosticCode::ArburgBlockCapacity,
            {std::to_string(data.size()), std::to_string(totalSize)});
    std::vector<std::uint8_t> block(std::size_t(totalSize), Constants::PaddingByte);
    std::copy(data.begin(), data.end(), block.begin());
    const auto value = checksum(data, token);
    block[data.size()] = std::uint8_t(value);
    block[data.size() + 1] = std::uint8_t(value >> Constants::BitsPerByte);
    return block;
}

std::vector<std::uint8_t> buildBlock(MediaFileSystems::Bytes source, bool system,
    std::stop_token token) {
    checkCancellation(token);
    const auto useful = system ? Constants::SystemUsefulSize : Constants::DataUsefulSize;
    const auto total = system ? Constants::SystemSize : Constants::DataSize;
    if (source.size() != useful && source.size() != total) {
        throw MediaError(system ? DiagnosticCode::ArburgSystemPayloadSize : DiagnosticCode::ArburgDataPayloadSize,
            {std::to_string(useful), std::to_string(total), std::to_string(source.size())});
    }
    return createBlock(source.first(useful), total, token);
}
}

