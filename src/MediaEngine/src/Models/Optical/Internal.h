#pragma once
#include "Track.h"
#include <limits>
namespace RetroMediaLab::MediaEngine::OpticalInternal {
namespace M=MediaFileSystems;
inline std::int64_t add(std::int64_t a,std::int64_t b){if((b>0&&a>std::numeric_limits<std::int64_t>::max()-b)||(b<0&&a<std::numeric_limits<std::int64_t>::min()-b))throw M::MediaError(M::DiagnosticCode::OpticalOverflow);return a+b;}
inline std::int64_t multiply(std::int64_t a,std::int64_t b){if(a<0||b<0||(b&&a>std::numeric_limits<std::int64_t>::max()/b))throw M::MediaError(M::DiagnosticCode::OpticalOverflow);return a*b;}
inline bool fits(const M::RandomAccessData& source,std::int64_t offset,std::int64_t count){return offset>=0&&count>=0&&std::uint64_t(offset)<=source.length()&&std::uint64_t(count)<=source.length()-std::uint64_t(offset);}
inline void relative(const OpticalTrackOptions& track,std::int64_t sector){if(sector<0||sector>=track.sectorCount)throw M::MediaError(M::DiagnosticCode::OpticalRelativeSector);}
inline std::vector<std::uint8_t> read(const std::shared_ptr<const M::RandomAccessData>& source,std::int64_t offset,std::int32_t count,std::stop_token token){std::vector<std::uint8_t> bytes(std::size_t(count),std::uint8_t{});source->readExactly(std::uint64_t(offset),bytes,token);return bytes;}
}
