#pragma once
#include "BlockImage.h"
#include "Constants.h"
#include "../../Sources/DriveAccess.h"
namespace RetroMediaLab::MediaEngine::Blocks {
struct State {std::int64_t capacity;std::int32_t blockSize;std::vector<DataRange> ranges;std::optional<HardDiskGeometry> geometry;std::shared_ptr<DriveMechanics> drive;bool writing=false;};
std::int64_t add(std::int64_t,std::int64_t);
std::int64_t multiply(std::int64_t,std::int64_t);
std::int64_t wrapAdd(std::int64_t,std::int64_t)noexcept;
DrivePosition position(const State&,std::int64_t);
std::int64_t contiguousLength(const State&,std::int64_t);
void read(const State&,std::int64_t,MediaFileSystems::MutableBytes,std::stop_token);
std::shared_ptr<const MediaFileSystems::RandomAccessData> bindSource(std::shared_ptr<const MediaFileSystems::RandomAccessData>,std::shared_ptr<DriveMechanics>,State,std::int64_t rangeAddress,std::int64_t sourceOffset,bool);
using Access=DriveAccessScope;
}
