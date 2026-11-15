#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <cstdint>
#include <vector>
#include <memory>
#include <span>
#include <optional>
#include <stop_token>
namespace RetroMediaLab::MediaEngine::Flux {
using Bits=std::vector<std::uint8_t>;
using BitView=std::span<const std::uint8_t>;
using Intervals=std::span<const std::uint32_t>;
enum class TimingMode {NonFm,Fm,Nrzi};
class Bitstream {
 Bits bits_;double cell_;
public:
 RETROMEDIALAB_MEDIAENGINE_API Bitstream(Bits,double cellTicks);
 BitView bits()const noexcept{return bits_;}
 double cellTicks()const noexcept{return cell_;}
 RETROMEDIALAB_MEDIAENGINE_API Bitstream withCircularTail(std::int64_t count)const;
};
struct Revolution {std::uint32_t indexTimeTicks=0;std::vector<std::uint32_t> intervals;};
using SharedRevolution=std::shared_ptr<const Revolution>;
RETROMEDIALAB_MEDIAENGINE_API double estimateCell(Intervals,TimingMode,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Bitstream reconstruct(Intervals,double initialCell,std::int32_t maximumCells,bool adapt = true,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Bitstream reconstructPll(Intervals,double centre,std::int32_t maximumCells,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Bitstream decodeAdaptiveFm(Intervals,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Bitstream decodeAdaptiveMfm(Intervals,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Bitstream decodePllFm(Intervals,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Bitstream decodePllMfm(Intervals,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Bitstream decodePll(Intervals,double cellTicks,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Bitstream decodeNrzi(Intervals,std::optional<double> cellTicks = {},std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Bitstream decodeAdaptiveDoubledNrzi(Intervals,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Revolution createRevolution(BitView,std::uint32_t cellTicks,std::uint32_t indexTicks,bool includeTrailing = true,std::stop_token = {});
}
