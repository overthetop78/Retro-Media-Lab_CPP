#pragma once
#include "../Primitives/Binary.h"
#include "PartitionConstants.h"
#include <RetroMediaLab/MediaFileSystems/Volumes.h>
#include <limits>
namespace RetroMediaLab::MediaFileSystems::Binary {
using namespace PartitionConstants;
inline std::int64_t volumeCoordinate(std::uint64_t value) {
    if (value > std::uint64_t(std::numeric_limits<std::int64_t>::max())) throw MediaError(DiagnosticCode::VolumeRangeOverflow);
    return std::int64_t(value);
}
inline bool hasGptSignature(Bytes data) { return data.size() >= GptSignature.size() && std::equal(GptSignature.begin(), GptSignature.end(), data.begin()); }
inline std::string partitionType(std::uint8_t type) {
    std::string value(HexPrefix);
    value.push_back(HexDigits[type >> HexShift]); value.push_back(HexDigits[type & HexMask]); return value;
}
inline void overlaps(VolumeDetectionResult& result, DiagnosticCode code) {
    std::stable_sort(result.volumes.begin(), result.volumes.end(), [](const auto& a, const auto& b) { return a.start() < b.start(); });
    for (std::size_t i = 1; i < result.volumes.size(); ++i) {
        const auto& previous = result.volumes[i-1]; const auto& current = result.volumes[i];
        if (current.start() - previous.start() < previous.length())
            result.diagnostics.push_back({code, {std::to_string(*previous.partitionNumber()), std::to_string(*current.partitionNumber())}});
    }
}
}
