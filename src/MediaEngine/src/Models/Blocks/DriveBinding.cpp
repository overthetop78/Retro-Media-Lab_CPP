#include "Internal.h"
#include "../../Sources/DriveRandomAccessData.h"
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaEngine {
namespace M=MediaFileSystems;
namespace Blocks {
DrivePosition position(const State& state,std::int64_t address){
    if(!state.geometry)return {0,0,-1,address/state.blockSize};
    const auto& geometry=*state.geometry;const auto sector=address/geometry.sectorSize();const auto cylinder=sector/geometry.sectorsPerTrack()/geometry.heads();if(cylinder>std::numeric_limits<std::int32_t>::max())throw M::MediaError(M::DiagnosticCode::BlockOverflow);
    return {std::int32_t(cylinder),std::int32_t(sector/geometry.sectorsPerTrack()%geometry.heads()),std::int32_t(sector%geometry.sectorsPerTrack()),-1};
}
std::int64_t contiguousLength(const State& state,std::int64_t address){if(!state.geometry)return state.capacity-address;const auto unit=multiply(state.geometry->sectorsPerTrack(),state.geometry->sectorSize());return unit-address%unit;}
std::shared_ptr<const M::RandomAccessData> bindSource(std::shared_ptr<const M::RandomAccessData> source,std::shared_ptr<DriveMechanics> drive,State layout,std::int64_t address,std::int64_t sourceOffset,bool writing){
    const auto at=[layout,address,sourceOffset](std::int64_t offset){return position(layout,add(address,offset-sourceOffset));};
    const auto contiguous=[layout,address,sourceOffset](std::int64_t offset){return contiguousLength(layout,add(address,offset-sourceOffset));};
    return std::make_shared<DriveRandomAccessData>(std::move(source),std::move(drive),at,contiguous,writing);
}
}
std::shared_ptr<const BlockImage> bindBlocks(std::shared_ptr<const BlockImage> image,std::shared_ptr<DriveMechanics> drive,bool writing){
    if(!image)throw M::MediaError(M::DiagnosticCode::BlockNullImage);if(!drive||(image->drive()==drive&&image->driveWriting()==writing))return image;
    const auto& original=*image->state_;Blocks::State layout{original.capacity,original.blockSize,{},original.geometry,{},false};auto state=std::make_shared<Blocks::State>(Blocks::State{original.capacity,original.blockSize,{},original.geometry,drive,writing});state->ranges.reserve(original.ranges.size());
    for(const auto& range:original.ranges){if(!range.source())state->ranges.push_back(range);else state->ranges.emplace_back(range.address(),range.length(),range.kind(),Blocks::bindSource(range.source(),drive,layout,range.address(),range.sourceOffset(),writing),range.sourceOffset());}
    return std::shared_ptr<const BlockImage>(new BlockImage(std::move(state)));
}
}
