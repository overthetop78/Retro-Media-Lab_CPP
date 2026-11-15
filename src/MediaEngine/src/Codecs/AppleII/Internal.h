#pragma once
#include "Codec.h"
#include "Constants.h"
#include "../Apple/BitLatch.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::AppleII::Internal {
template<std::size_t Size> constexpr auto inverse(const std::array<std::uint8_t,Size>& table){
    std::array<std::int16_t,Constants::InverseTableSize> result{};result.fill(-1);
    for(std::size_t index=0;index<table.size();++index)result[table[index]]=std::int16_t(index);
    return result;
}
inline constexpr auto InverseSixAndTwoTable=inverse(Constants::SixAndTwoTable),InverseFiveAndThreeTable=inverse(Constants::FiveAndThreeTable);

inline void validateSize(MediaFileSystems::Bytes source){if(source.size()!=Constants::SectorSize)throw MediaError(DiagnosticCode::AppleIISectorSize,{std::to_string(source.size())});}
template<class Table> std::optional<std::vector<std::uint8_t>> values(Flux::BitView bits,std::int64_t& cursor,std::int32_t count,const Table& inverse,std::stop_token token){
    auto encoded=Apple::readBytes(bits,cursor,count,token);if(!encoded)return std::nullopt;
    for(auto& symbol:*encoded){checkCancellation(token);const auto value=inverse[symbol];if(value<0)return std::nullopt;symbol=std::uint8_t(value);}
    return encoded;
}
}
