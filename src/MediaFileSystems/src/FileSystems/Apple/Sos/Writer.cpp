#include <RetroMediaLab/MediaFileSystems/AppleSos.h>
#include <RetroMediaLab/MediaFileSystems/AppleRecognitionConstants.h>
#include "../../../Primitives/Identifiers.h"
#include <algorithm>
namespace RetroMediaLab::MediaFileSystems::AppleSos {
SectorWritePlan write(const MigrationPlan& plan, const SectorGeometry& geometry, std::uint32_t count, std::span<const SectorAddress> addresses, std::string format, std::optional<Timestamp> time, std::stop_token token) {
    if (Identifiers::isBlank(format)) throw MediaError(DiagnosticCode::SectorInvalidFormatId, {format});
    auto result = ProDos::write(plan, geometry, count, addresses, time, token);
    std::sort(result.blocks.begin(), result.blocks.end(), [](const auto& a, const auto& b) { return a.logicalBlock < b.logicalBlock; });
    auto& boot = result.blocks.front().data;
    using namespace Apple::RecognitionConstants;
    std::copy(SosMarker.begin(), SosMarker.end(), boot.begin() + SosMarkerOffset);
    result.geometry.formatId = std::move(format);
    return result;
}
}
