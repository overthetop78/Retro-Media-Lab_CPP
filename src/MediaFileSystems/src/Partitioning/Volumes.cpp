#include <RetroMediaLab/MediaFileSystems/Volumes.h>
#include "Helpers.h"
#include <QString>
#include <algorithm>
#include <utility>
namespace RetroMediaLab::MediaFileSystems {
VolumeDescriptor::VolumeDescriptor(std::int64_t start, std::int64_t length, std::string origin,
    std::optional<std::string> partitionTable, std::optional<std::int32_t> partitionNumber,
    std::optional<std::int32_t> sessionNumber, std::optional<std::int32_t> trackNumber,
    std::optional<std::string> fileSystemId, std::optional<std::string> partitionType,
    std::optional<std::string> partitionId, std::optional<std::string> name,std::optional<MessageRecipe> nameRecipe)
    : start_(start), length_(length), origin_(std::move(origin)), partitionTable_(std::move(partitionTable)),
      partitionNumber_(partitionNumber), sessionNumber_(sessionNumber), trackNumber_(trackNumber),
      fileSystemId_(std::move(fileSystemId)), partitionType_(std::move(partitionType)),
      partitionId_(std::move(partitionId)), name_(std::move(name)),nameRecipe_(std::move(nameRecipe)) {
    if (start < 0) throw MediaError(DiagnosticCode::VolumeStart, {std::to_string(start)});
    if (length <= 0) throw MediaError(DiagnosticCode::VolumeLength, {std::to_string(length)});
    const auto value = QString::fromUtf8(origin_.data(), qsizetype(origin_.size()));
    if (value.isEmpty() || std::all_of(value.begin(), value.end(), [](QChar character) { return character.isSpace(); }))
        throw MediaError(DiagnosticCode::VolumeOrigin);
    if (partitionNumber && *partitionNumber <= 0) throw MediaError(DiagnosticCode::VolumePartitionNumber, {std::to_string(*partitionNumber)});
    if (sessionNumber && *sessionNumber <= 0) throw MediaError(DiagnosticCode::VolumeSessionNumber, {std::to_string(*sessionNumber)});
    if (trackNumber && *trackNumber <= 0) throw MediaError(DiagnosticCode::VolumeTrackNumber, {std::to_string(*trackNumber)});
}
VolumeDetectionResult detectWholeMedia(const BlockMedia& media, std::stop_token token) {
    checkCancellation(token);
    if (media.capacity()==0) return {{},{{DiagnosticCode::NoVolumeLength,{}}}};
    return {{VolumeDescriptor{0, Binary::volumeCoordinate(media.capacity()), std::string(VolumeConstants::DirectVolume)}},{}};
}
VolumeDetectionResult detectBlockVolumes(const BlockMedia& media, std::stop_token token) {
    if(auto result=detectGpt(media,token))return std::move(*result);
    if(auto result=detectMbr(media,token))return std::move(*result);
    return detectWholeMedia(media,token);
}
}
