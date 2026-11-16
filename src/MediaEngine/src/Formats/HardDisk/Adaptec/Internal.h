#pragma once
#include "Codec.h"
#include "Constants.h"
#include <map>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Adaptec {
inline bool validSize(std::size_t size){return std::find(Constants::CandidateSizes.begin(),Constants::CandidateSizes.end(),size)!=Constants::CandidateSizes.end();}
struct Location {std::int32_t offset=-1;bool nextData=false,nextClock=false,sync=false;};
using Locations=std::map<std::int64_t,Location>;
void writeErrorCheck(std::span<std::uint8_t>);
void appendByte(Flux::Bits&,std::uint8_t,bool&);
void appendField(Flux::Bits&,std::span<const std::uint8_t>,bool&);
std::vector<Field> decodeCore(const Flux::NanosecondRevolution&,std::int32_t,std::int32_t,std::int32_t,Locations*,std::stop_token);
}
