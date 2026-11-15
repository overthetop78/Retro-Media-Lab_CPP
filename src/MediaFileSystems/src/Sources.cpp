#include <RetroMediaLab/MediaFileSystems/Sources.h>
#include <utility>
namespace RetroMediaLab::MediaFileSystems {
BlockMedia::BlockMedia(std::shared_ptr<const RandomAccessData> source, std::uint32_t blockSize, bool hardDisk, std::optional<std::uint64_t> declaredBlockCount)
    : source_(std::move(source)), blockSize_(blockSize), hardDisk_(hardDisk), declaredBlockCount_(declaredBlockCount) {
    if (!source_) throw MediaError(DiagnosticCode::InvalidSource);
    if (blockSize_ == 0 || blockSize_ > VolumeConstants::MaximumBlockSize) throw MediaError(DiagnosticCode::InvalidBlockSize);
}
void BlockMedia::readExactly(std::uint64_t offset, MutableBytes destination, std::stop_token token) const {
    checkCancellation(token);
    if (offset > capacity() || destination.size() > capacity() - offset) throw MediaError(DiagnosticCode::ReadOutOfBounds);
    source_->readExactly(offset, destination, token);
}
}
