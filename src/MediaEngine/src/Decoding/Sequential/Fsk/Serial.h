#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <vector>
#include <span>
#include <cstdint>
#include <stop_token>
namespace RetroMediaLab::MediaEngine::Fsk {
class DecodeResult final {
public:
    RETROMEDIALAB_MEDIAENGINE_API DecodeResult(std::vector<std::uint8_t> bytes,int valid,int tested,std::vector<int> positions={});
    std::span<const std::uint8_t> bytes()const noexcept{return bytes_;}
    int validFrames()const noexcept{return valid_;}int testedFrames()const noexcept{return tested_;}
    std::span<const int> positions()const noexcept{return positions_;}
private:std::vector<std::uint8_t> bytes_;int valid_,tested_;std::vector<int> positions_;
};
RETROMEDIALAB_MEDIAENGINE_API DecodeResult decode(std::span<const std::int32_t> samples,int rate,int baud,int zeroFrequency,int oneFrequency,int dataBits,int stopBits,int minimumSamplesPerBit,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API DecodeResult decodeFrames(std::span<const std::uint8_t> bits,int dataBits,int stopBits,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API void appendCarrier(std::vector<std::int16_t>&,std::int16_t& level,int rate,int baud,int zeroFrequency,int oneFrequency,double seconds,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API void appendFrame(std::vector<std::int16_t>&,std::int16_t& level,int rate,int baud,int zeroFrequency,int oneFrequency,int dataBits,int stopBits,std::uint8_t,std::stop_token={});
}
